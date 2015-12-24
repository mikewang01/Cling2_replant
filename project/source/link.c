/***************************************************************************/
/**
 * File: auth.c
 * 
 * Description: USER related data processing functions
 *
 * Created on December 22, 2015
 *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

//#define _SKIP_LINK_

//#define AUTO_AUTHENTICATE 1

const I8U _bytesInFlash2[2] = {0xFF,0xDC};

const I8U LINK_VERSION_STRING[]= ",1.0,";

const I8U _s1[] = "fb73A";
const I8U _s2[] = "ea62A";
const I8U _s3[] = "d951A";
const I8U _s4[] = "c840A";

BOOLEAN _generatingBlob = BOOLEAN_FALSE; // indicates that we are generating a blob (used for timeout check)
BOOLEAN _checkingBlob = BOOLEAN_FALSE;	// indicates that we are checking a blob (used for timeout check)
BOOLEAN _waitingBeforeFileCheck = BOOLEAN_FALSE; // flag indicating we are waiting for a short period before checking file.
I8U *token_string; // (length: 100, MAX_ENCRYPT_DECRYPT_STRING_SIZE)
I8U *crypto_buffer;  // 16 BYTES BUFFER FOR crypto.(length: 128)
I8U *_thekey; // 16 bytes
I8U _pad;
I32U _startTime, _timeCheck;
I32U _userID;
I16U _decodedStringSize;

#define MUTEX_SPIN_COUNTER_MAX 1000
I16U _mutexSpinCounter = 0;

I8U *_buf; // local pointer into the string

void _HexToAscii(I8U *inBytes, I8U *outString, I8U length) {
	I8U i, byteToConvert;
	for (i = 0; i < length; i++) {
		byteToConvert = (inBytes[i]>>4)&0x0f;
		byteToConvert += (byteToConvert>9)? 'A' - 0x0a : '0';
		outString[(i)<<1] = byteToConvert;

		byteToConvert = (inBytes[i])&0x0f;
		byteToConvert += (byteToConvert>9)? 'A' - 0x0a : '0';
		outString[((i)<<1)+1] = byteToConvert;
	}
}

void _AsciiToHex(I8U *inString, I8U *outBytes, I8U length) {
	I8U i, characterToConvert, tempByte;
    for (i=0; i<(length>>1); i++) {
        characterToConvert = inString[i<<1];
        if (characterToConvert <= '9') {
            tempByte = characterToConvert - '0';
        } else if (characterToConvert <= 'F') {
            tempByte = (characterToConvert - 'A') + 10;
        } else {
            tempByte = (characterToConvert - 'a') + 10;
        }
        outBytes[i] = tempByte << 4;

        characterToConvert = inString[(i<<1)+1];
        if (characterToConvert <= '9') {
            tempByte = characterToConvert - '0';
        } else if (characterToConvert <= 'F') {
            tempByte = (characterToConvert - 'A') + 10;
        } else {
            tempByte = (characterToConvert - 'a') + 10;
        }
        outBytes[i] |= tempByte;
    }
}

void _mapk(void) {
	I8U i;
	I8U *pkey = (I8U *)_thekey;
	
	for (i=0;i<4;i++) {
		*pkey++ = _s4[i];
		*pkey++ = _s3[i];
		*pkey++ = _s2[i];
		*pkey++ = _s1[i];
	}
//	memcpy(_thekey, _keystore, 16);
}

void _cleark(void) {
	memset(_thekey, 0, sizeof(_thekey));
}

// Converts a string to upper.  Stops at length or null character.
void _toupper(I8U* charString, I8U length) {
	I8U i;
	for (i=0;i<length;i++) {
		if (NULL == charString[i]) break;
		// ascii order is "0Aa", that is, the lower case has a larger value than lower case
		if (charString[i] >= 'a') {
			charString[i] -= 'a' - 'A';
		}
	}
}

// generates a random number.  It is based on the current time, the device ID, and other data
void _fill_up_random_characters(I8U *charBuffer, I8U number_of_characters) 
{
	I32U currentTickTime = CLK_get_system_time();
	I8U i = 0;
	
	for (i = 0; i < number_of_characters; i++) {
		charBuffer[i] = '0'+(currentTickTime%10);
		currentTickTime /= 10;
	}
}

// last Flash address is 0xFF80 so there are 0xFF80 - 0x4400 bytes.  750 chunks.
//#define CHUNKS_IN_FLASH1 750
//#define CHUNKS_IN_FLASH2 1024
//#define WORDS_IN_CHUNK_BUFFER 32
//#define DEBUG_MEMORY_MAP 1
#define BYTES_IN_FLASH1 48128
//#define BYTES_IN_FLASH2 65500
//#define WORDS_IN_FLASH1 24000 /* these are 2B values */
//#define WORDS_IN_FLASH2 65500 /* these are 2B values */


void LINK_state_machine(void) 
{
	LINK_CTX *acc = &cling.link;
	I32U curr_sys_time = CLK_get_system_time(); // Get current system time
	CLING_FILE f;

	// safety to make sure we never hold the mutex;  we clear the mutex on entering the state machine
	SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);

	switch (acc->auth_state) {
		case LINK_S_IDLE:{
			if (CTL_A1 == (CTL_A1 & cling.system.mcu_reg[REGISTER_MCU_CTL])) {
				_timeCheck = curr_sys_time;
				acc->auth_state = LINK_S_START_AUTHENTICATION;
				acc->trigger_cmd = CTL_A1;
				N_SPRINTF("[LINK] A1 ...");
				// Borrow pedo grobal buffer as it is a one-time use
				crypto_buffer = PEDO_get_global_buffer()+512;
				_thekey = crypto_buffer+128;
				token_string = _thekey+64;
				// terminal state for this phase is WRITE_ENCRYPT_FILE or GENERATE_ERROR_FILE
			} else if (CTL_A2 == (CTL_A2 & cling.system.mcu_reg[REGISTER_MCU_CTL])) {
				_timeCheck = curr_sys_time;
				acc->auth_state = LINK_S_WAIT_FOR_CHECK_FLAG;
				cling.link.trigger_cmd = CTL_A2;
				N_SPRINTF("[LINK] A2 ...");
				// Borrow pedo grobal buffer as it is a one-time use
				crypto_buffer = PEDO_get_global_buffer()+512;
				_thekey = crypto_buffer+128;
				token_string = _thekey+64;

				// terminal state for this phase is AUTHORIZED or GENERATE_ERROR_FILE
			} else if (CTL_FA == (CTL_FA & cling.system.mcu_reg[REGISTER_MCU_CTL])) {
				_timeCheck = curr_sys_time;
				// force the userID to 0x7F_FF_FF_FF
				_userID = (I32U)0x7FFFFFFF;
				acc->auth_state = /*LINK_STATE_STORE; */LINK_S_WRITE_LINK_INFO;
				cling.link.trigger_cmd = CTL_FA;
				// terminal state for this phase is AUTHORIZED or GENERATE_ERROR_FILE
				N_SPRINTF("[LINK] force auth ...");
				// Borrow pedo grobal buffer as it is a one-time use
				crypto_buffer = PEDO_get_global_buffer()+512;
				_thekey = crypto_buffer+128;
				token_string = _thekey+64;

			} else {
				cling.link.trigger_cmd = 0;
			}
		}
		break;

		case LINK_S_START_AUTHENTICATION:{
			{
				cling.link.b_authorizing = TRUE;
				cling.link.link_ts = curr_sys_time;
				_generatingBlob = BOOLEAN_TRUE;
				_timeCheck = curr_sys_time;

				// Store the device ID string into the token string (20 bytes).
				SYSTEM_get_dev_id(&token_string[0]);
				
				// Get version string
				_buf = &token_string[VERSION_STRING_START_POSITION];
				
				// now add the version number (it has the right colons in it)
				sprintf((char *)_buf, (const char *)LINK_VERSION_STRING);
				memcpy(crypto_buffer, token_string, CHARACTERS_IN_FIRST_ENCRYPT_STRING);

				// Encrypt the first section of Device ID, and put it into Token
				_fill_up_random_characters(&crypto_buffer[CHARACTERS_IN_FIRST_ENCRYPT_STRING],6);
				
				crypto_buffer[CHARACTERS_IN_FIRST_ENCRYPT_STRING] = ',';

				_mapk();
#ifndef _CLING_PC_SIMULATION_
				// now kick off the encryption on the first blob.
				BTLE_aes_encrypt((I8U *)_thekey, crypto_buffer, crypto_buffer);
#endif
				acc->auth_state = LINK_S_GENERATE_ENCRYPT_BLOCK1;
			}
		}
		break;

		case LINK_S_GENERATE_ENCRYPT_BLOCK1:{
			// convert the encrypted block to a string and put it into our string buffer.
			_buf = &token_string[FIRST_AES_OUT_TEXT_POSITION];
			_HexToAscii(crypto_buffer, _buf, ENCRYPT_DECRYPT_BLOCK_SIZE);

			// generate the second encryption block
			memcpy(crypto_buffer, &token_string[CHARACTERS_IN_FIRST_ENCRYPT_STRING], CHARACTERS_IN_SECOND_ENCRYPT_STRING);
			_fill_up_random_characters(&crypto_buffer[CHARACTERS_IN_SECOND_ENCRYPT_STRING],6);

			crypto_buffer[CHARACTERS_IN_SECOND_ENCRYPT_STRING] = ',';
#ifndef _CLING_PC_SIMULATION_
			// now kick off the encryption on the second blob.
			BTLE_aes_encrypt((I8U *)_thekey, crypto_buffer, crypto_buffer);
#endif
			acc->auth_state = LINK_S_GENERATE_ENCRYPT_BLOCK2;
		}
		break;

		case LINK_S_GENERATE_ENCRYPT_BLOCK2:{
			// convert the encrypted block to a string and put it into our string buffer.
			token_string[SECOND_AES_OUT_TEXT_POSITION-1] = ',';
			_buf = &token_string[SECOND_AES_OUT_TEXT_POSITION];
			_HexToAscii(crypto_buffer, _buf, ENCRYPT_DECRYPT_BLOCK_SIZE);
			acc->auth_state = LINK_S_WRITE_ENCRYPT_FILE;
			_mutexSpinCounter = 0;
		}
		break;

		case LINK_S_WRITE_ENCRYPT_FILE:{
			if (/*TRUE*/SYSTEM_get_mutex(MUTEX_MCU_LOCK_VALUE)) {
				_cleark();
				f.fc = FILE_fopen((I8U *)"security_device.txt", FILE_IO_WRITE);
				FILE_fwrite(f.fc, token_string, ENCRYPT_FILE_SIZE);
				FILE_fclose(f.fc);
				SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
				acc->auth_state = LINK_S_IDLE;
				_startTime = curr_sys_time;
				_generatingBlob = BOOLEAN_FALSE;
				cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_A1;
				_mutexSpinCounter = 0;
				if (cling.link.trigger_cmd == CTL_A1) {
					N_SPRINTF("[LINK] A1 done");
					acc->comm_state = LINK_COMM_A1_DONE;
					acc->userID = (I32U) 0xFFFFFFFF; // cling.link.userID;
					_generatingBlob = BOOLEAN_FALSE;
					CP_create_auth_stat_msg();
				}
			} else {
				if (_mutexSpinCounter++ > MUTEX_SPIN_COUNTER_MAX) {
					acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
					acc->error_code = ERROR_WRITE_FILE_MUTEX_HOLD;
				}
			}
		}
		break;

		case LINK_S_WAIT_FOR_CHECK_FLAG:{
			cling.link.b_authorizing = TRUE;
			cling.link.link_ts = curr_sys_time;

			if (CTL_A2 == (CTL_A2 & cling.system.mcu_reg[REGISTER_MCU_CTL])) {
					acc->auth_state = LINK_S_WAITING_ON_RETURN_FILE;
					_waitingBeforeFileCheck = BOOLEAN_FALSE;
					_checkingBlob = BOOLEAN_TRUE;
					_mutexSpinCounter = 0;
					_timeCheck = curr_sys_time; // starts counter for decoding everything
				#if 0
				
					// Auth-2 not supported at the moment
					acc->error_code = ERROR_DECRYPT_NOT_SUPPORTED;
					acc->auth_state = GENERATE_ERROR_FILE;
					SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
				#endif
			}
		}
		break;

		case LINK_S_WAITING_ON_RETURN_FILE:{
#ifdef AUTO_AUTHENTICATE
			if ((curr_sys_time - _timeCheck) > DEBUG_MILLISECONDS_BEFORE_AUTO_AUTH) {
				_timeCheck = curr_sys_time;
				// force the userID to 0x7F_FF_FF_FF
				cling.link.pairing.userID = (I32U)0x7FFFFFFF;
				acc->auth_state = WRITE_LINK_INFO;
				break;
			}
#endif
			if (/*TRUE*/SYSTEM_get_mutex(MUTEX_MCU_LOCK_VALUE)) {
				_mutexSpinCounter = 0;
				if (TRUE == FILE_if_exists((I8U *)"security_service.txt")) {
					CLING_FILE f;
					// file exists, so check
					f.fc = FILE_fopen((I8U *)"security_service.txt", FILE_IO_READ);
					if (FROM_SERVICE_FILE_SIZE != f.fc->size) {
						// we have an error and have no need to process anything further.
						FILE_fclose(f.fc);
						acc->error_code = ERROR_FILE_LENGTH;
						acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
						SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
						break;
					}
					// size is right.  read the file into the buffer.
					FILE_fread(f.fc, token_string, FROM_SERVICE_FILE_SIZE);
					FILE_fclose(f.fc);

					// confirm that there are two comma in the expected place
					if ((',' != token_string[ENCRYPT_STRING_SIZE]) || (',' != token_string[(ENCRYPT_STRING_SIZE << 1) + 1])) {
						// we have an error and have no need to process anything further.
						acc->error_code = ERROR_FILE_CONSTRUCTION;
						acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
						SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
						break;
					} else {
						N_SPRINTF("[LINK] detected 2 ','");
					}
					
					// convert the first part of the buffer to binary and put in the buffer
					_AsciiToHex(&token_string[0], crypto_buffer, ENCRYPT_STRING_SIZE);

					_mapk();

					// then call the decryption routine
					BTLE_aes_decrypt((I8U *)_thekey, crypto_buffer, crypto_buffer);

					acc->auth_state = LINK_S_DECRYPT_BLOB1;
					
				} else {
					// We were told to check the file but it doesn't exist.
					acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
					_checkingBlob = BOOLEAN_FALSE;
					acc->error_code = ERROR_FILE_NOT_FOUND;
				}
				SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
			}  // end if (SYSTEM_get_mutex(MUTEX_MCU_LOCK_VALUE))
			else {
				_mutexSpinCounter++;
				if (_mutexSpinCounter > MUTEX_SPIN_COUNTER_MAX) {
					acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
					acc->error_code = ERROR_READ_FILE_MUTEX_HOLD;
				}
			}
		}
		break;

		case LINK_S_DECRYPT_BLOB1:
		{
			N_SPRINTF("[LINK] DECRYPT BLOB1");
			// Copy out the first 16 hex digits
			memcpy(&token_string[0], crypto_buffer, ENCRYPT_DECRYPT_BLOCK_SIZE);

			// decrypt the next blob
			// convert the second part of the buffer to binary and put in the buffer
			_AsciiToHex(&token_string[ENCRYPT_STRING_SIZE+1], crypto_buffer, ENCRYPT_STRING_SIZE);
			_mapk();
			// then call the decryption routine
			BTLE_aes_decrypt((I8U *)_thekey, crypto_buffer, crypto_buffer);
			acc->auth_state = LINK_S_DECRYPT_BLOB2;
			break;
		}
		case LINK_S_DECRYPT_BLOB2:
		{
			N_SPRINTF("[LINK] DECRYPT BLOB2");

			// copy the second part of the information out of the encrypt block to the text block
			// compress the pieces together;  the ID is two pieces
			memcpy(&token_string[CHARACTERS_IN_FIRST_ENCRYPT_STRING], crypto_buffer, ENCRYPT_DECRYPT_BLOCK_SIZE);
			
			// decrypt the next blob
			// convert the first part of the buffer to binary and put in the buffer
			_AsciiToHex(&token_string[(ENCRYPT_STRING_SIZE<<1)+1+1 /* for "," */], crypto_buffer, ENCRYPT_STRING_SIZE);
			_mapk();
			// then call the decryption routine
			BTLE_aes_decrypt((I8U *)_thekey, crypto_buffer, crypto_buffer);
			acc->auth_state = LINK_S_DECRYPT_BLOB3;
			break;
		}
		case LINK_S_DECRYPT_BLOB3:{
			N_SPRINTF("[LINK] DECRYPT BLOB3");
			// copy the third part of the information out of the encrypt block to the text block
			memcpy(&token_string[ENCRYPT_DECRYPT_BLOCK_SIZE<<1], crypto_buffer, ENCRYPT_DECRYPT_BLOCK_SIZE);
			acc->auth_state = LINK_S_CHECK_FILE;
		}
		break;
		
		case LINK_S_CHECK_FILE: {
			I8U i,j;
			I16U idDoesNotMatchQ;
			
			// Convert string all upper case.
			_toupper(token_string,DEVICE_ID_TOTAL_CHARACTERS);
			
			// add a colon and then the device ID;
			token_string[DEVICE_ID_TOTAL_CHARACTERS] = ',';
			j=DEVICE_ID_TOTAL_CHARACTERS+1;
			for (i=32; i<45; i++) {
				// stop when we find the colon
				if (',' == token_string[i]) break;
				token_string[j++] = token_string[i];
			}
			// in case of an error later, we store this information so that we may dump out the
			// completely assembled string before dumping the error code.  Only
			// used if we have DEBUG_LINK_CODE_BLOBS defined.
			N_SPRINTF("[LINK] user id position: %d", j);
			_decodedStringSize = j;

			// validate the return data (need to check that the ID matches and if it does, check the ID).
			// use part of the encryptDecryptString to put the device ID
			SYSTEM_get_dev_id(&token_string[64]);
			idDoesNotMatchQ = strncmp((char *)token_string, (char *)&token_string[64], DEVICE_ID_TOTAL_CHARACTERS);
			if (idDoesNotMatchQ) {
				// force the userID to 0x7F_FF_FF_FF
				cling.link.pairing.userID = (I32U)0x7FFFFFFF;
				acc->error_code = ERROR_ID_MISMATCH;
				acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
			} else {
				// convert the user ID to a binary value;  digits is the difference between DEVICE_ID_TOTAL_CHARACTERS+1 and j
				// go right to left.
				I32U multiplicand = 1;
				I16S _digitValue;
				_userID = 0;
				for (i=j-1; i>=DEVICE_ID_TOTAL_CHARACTERS+1; i--) {
					_digitValue = token_string[i]-'0';
					if ((_digitValue > 10) || (_digitValue < 0)) {
						acc->error_code = ERROR_BAD_DEVICE_ID;
						acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
						break;
					}
					_userID += _digitValue * multiplicand;
					multiplicand *= 10;
				}
				N_SPRINTF("[LINK] user id is: %d", _userID);
			}
			if (LINK_S_GENERATE_ERROR_FILE != acc->auth_state) 
				acc->auth_state = LINK_S_WRITE_LINK_INFO;
		}
		break;

		case LINK_S_GENERATE_ERROR_FILE:
		{
			_generatingBlob = BOOLEAN_FALSE;
			_checkingBlob = BOOLEAN_FALSE;
			if (SYSTEM_get_mutex(MUTEX_MCU_LOCK_VALUE) || (_mutexSpinCounter > MUTEX_SPIN_COUNTER_MAX)) {
				SYSTEM_release_mutex(MUTEX_MCU_LOCK_VALUE);
				acc->auth_state = LINK_S_IDLE;
				cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_A1;
				cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_A2;
				cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_FA;
				_generatingBlob = BOOLEAN_FALSE;
				_checkingBlob = BOOLEAN_FALSE;
				_mutexSpinCounter = 0;
				N_SPRINTF("[LINK] error file:%d", acc->error_code);
				acc->comm_state = LINK_COMM_ERROR;
				acc->userID = (I32U) 0xFFFFFFFF; // cling.link.userID;
				CP_create_auth_stat_msg();
				_cleark();
			} else _mutexSpinCounter++;
		}
		break;
		

		case LINK_S_WRITE_LINK_INFO:
		{
			N_SPRINTF("[LINK] write auth info");
			cling.link.b_authorizing = TRUE;
			cling.link.link_ts = curr_sys_time;
			
			// note that the crc for the flash is generated at boot if not already authenticated
			cling.link.pairing.epoch = cling.time.time_since_1970;
			// Flag the authentication
			cling.link.pairing.userID = _userID;
			cling.link.pairing.authToken = LINK_TOKEN;
			cling.link.pairing.crc = 0x1234;
			acc->auth_state = LINK_S_AUTHORIZED;
			
			// Simulate a single button click to change UI
			cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_A2;
			cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_FA;

			// Finishing all decryption tasks, re-initializae pedometer ...
			// Has to initializ pedometer here as we borrow the buffer for encryption & decryption.
			TRACKING_initialization();
			// Right after authorization, tune up sensitivity.
			PEDO_set_step_detection_sensitivity(TRUE);
			
			// Now unit is authorized, we will re-initialize all activity counts wi
			TRACKING_total_data_load_file();

		}
		break;

		case LINK_S_AUTHORIZED:
		{
			acc->auth_state = LINK_S_WRITING_LINK_INFO;
			if (cling.link.trigger_cmd == CTL_A2) {
				acc->comm_state = LINK_COMM_A2_DONE;
				_checkingBlob = BOOLEAN_FALSE;
			} else if (cling.link.trigger_cmd == CTL_FA)
				acc->comm_state = LINK_COMM_FA_DONE;
			else
				acc->comm_state = LINK_COMM_ERROR;
			acc->userID = cling.link.pairing.userID;
			CP_create_auth_stat_msg();
			_timeCheck = CLK_get_system_time();
			cling.link.b_authorizing = FALSE;
			N_SPRINTF("[LINK] authorized!  userID: %d", acc->userID);
		}
		break;

		case LINK_S_WRITING_LINK_INFO:
		{
			I32U dw_buf[16];
#ifdef _SKIP_LINK_
			cling.link.auth_state = LINK_S_IDLE;
			N_SPRINTF("[LINK] Writing link info! ");
#else
			memcpy(dw_buf, (I8U *)&cling.link.pairing, sizeof(PAIRING_CTX));
	
			FLASH_Write_App(SYSTEM_LINK_SPACE_START, (I8U *)dw_buf, sizeof(PAIRING_CTX));
			cling.link.auth_state = LINK_S_REBOOT_DEVICE;
#endif
			cling.link.link_ts = curr_sys_time;
		}
		break;
	
		case LINK_S_REBOOT_DEVICE:
			if (curr_sys_time > (cling.link.link_ts + 5000)) {
				SYSTEM_restart_from_reset_vector();
				cling.link.auth_state = LINK_S_IDLE;
				N_SPRINTF("[LINK] reboot");
			}
			break;
		
		default:{
			acc->auth_state = LINK_S_GENERATE_ERROR_FILE;
			acc->error_code = ERROR_HIT_DEFAULT_STATE;
		}
		break;
	}
	
}

void LINK_init()
{
	LINK_CTX *acc = &cling.link;
	PAIRING_CTX *a = &cling.link.pairing;
	
	FLASH_Read_App(SYSTEM_LINK_SPACE_START, (I8U *)a, sizeof(PAIRING_CTX));
	
	Y_SPRINTF("[LINK] %x, %x, %x", a->authToken, a->crc, a->userID);
	
	// Initialize all communciation context
	acc->error_code = NO_ERROR;
	acc->auth_state = LINK_S_IDLE;  // initial value
	acc->comm_state = LINK_COMM_IDLE;

#ifdef _SKIP_LINK_
	acc->auth_state = LINK_S_AUTHORIZED;
#endif
}

BOOLEAN LINK_is_authorizing()
{
	return cling.link.b_authorizing;
}

BOOLEAN LINK_is_authorized()
{
#ifdef _SKIP_LINK_
	return TRUE;
#else
	return (LINK_TOKEN == cling.link.pairing.authToken);
#endif
}

void LINK_deauthorize()
{
	LINK_CTX *acc = &cling.link;
	PAIRING_CTX *a = &cling.link.pairing;

	FLASH_erase_App(SYSTEM_LINK_SPACE_START);
	FLASH_erase_App(SYSTEM_INFORMATION_SPACE_START);
	
	acc->auth_state = LINK_S_IDLE;
	memset((I8U *)a, 0, sizeof(PAIRING_CTX));
}


