/***************************************************************************//**
 * File: btle.c
 * 
 * Description: BLUETOOTH low energy protocol and state machine handle
 *
 ******************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "main.h"
#include "Aes.h"

#define BLE_PAYLOAD_LEN 20
#ifndef _CLING_PC_SIMULATION_
extern int8_t ancs_master_handle;

#endif

///////////////////////////////
// SPEED 
//   Low: 
//       Min connection interval -- 300 * 1.25ms = 375ms
//       Man connection interval -- 320 * 1.25ms = 400ms
//       Slave Latency -- 4
//       Connection supervision timeout-- 600 * 10ms = 6000ms
//       Enable Update Request: ON
// 
//   Medium:
//       Min connection interval -- 72 * 1.25ms = 90ms
//       Man connection interval -- 92 * 1.25ms = 115ms
//       Slave Latency -- 0
//       Connection supervision timeout-- 600 * 10ms = 6000ms
//       Enable Update Request: ON
// 
//   High:
//      Min connection interval -- 16 * 1.25ms = 20ms
//      Man connection interval -- 36 * 1.25ms = 45ms
//      Slave Latency -- 0
//      Connection supervision timeout-- 600 * 10ms = 6000ms
//      Enable Update Request: default ON
//
//    off:
//      Min connection interval -- 300 * 1.25ms = 375ms
//      Man connection interval -- 320 * 1.25ms = 400ms
//      Slave Latency -- 4
//      Connection supervision timeout-- 600 * 10ms = 6000ms
//      Enable Update Request: OFF
//

I32U ble_char_add(I16U uuid, I8U idx)
{
#ifndef _CLING_PC_SIMULATION_
	BLE_CTX *r = &cling.ble;
	ble_gatts_char_md_t char_md;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_attr_md_t user_md;
	ble_gatts_attr_t    attr_char_value;
	ble_uuid_t          ble_uuid;
	ble_gatts_char_handles_t *pchar_handles;
	I32U   err_code = NRF_SUCCESS;
	I8U user_desp[2];
	I8U ble_rx_tx_buf[BLE_PAYLOAD_LEN];

	// User description
	memset(&user_md, 0, sizeof(user_md));
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&user_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&user_md.write_perm);
	user_md.vloc       = BLE_GATTS_VLOC_STACK;
	user_md.rd_auth    = 0;
	user_md.wr_auth    = 0;
	user_md.vlen       = 0;
	user_desp[0] = idx;
	user_desp[1] = 0;

	memset(&char_md, 0, sizeof(char_md));

	if ((idx == 1) || (idx == 2) || (idx == 3) || (idx == 4)) {
		char_md.char_props.notify = 1;
	} else if ((idx == 5) || (idx == 6) || (idx == 7) || (idx == 8)) {
		char_md.char_props.write = 1;
	} else {
		return err_code;
	}
	pchar_handles = &r->char_handles[idx-1];
	char_md.p_char_user_desc         = user_desp;
	char_md.char_user_desc_max_size  = 2;
	char_md.char_user_desc_size        = 2;
	char_md.p_char_pf        = NULL;
	char_md.p_user_desc_md   = &user_md;
	char_md.p_cccd_md        = NULL;
	char_md.p_sccd_md        = NULL;
	
  BLE_UUID_BLE_ASSIGN(ble_uuid, uuid);
    
	memset(&attr_md, 0, sizeof(attr_md));

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
	attr_md.vloc       = BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth    = 0;
	attr_md.wr_auth    = 0;
	attr_md.vlen       = 0;

	memset(&attr_char_value, 0, sizeof(attr_char_value));

	attr_char_value.p_uuid       = &ble_uuid;
	attr_char_value.p_attr_md    = &attr_md;

	attr_char_value.init_len     = BLE_PAYLOAD_LEN;
	attr_char_value.init_offs    = 0;
	attr_char_value.max_len      = BLE_PAYLOAD_LEN;
	attr_char_value.p_value      = ble_rx_tx_buf;

	return sd_ble_gatts_characteristic_add(r->service_handle,
																				 &char_md,
																				 &attr_char_value,
																				 pchar_handles);
#else
	return 0;
#endif
}

void _radio_state_cleanup()
{
	// iOS should release MUTEX whatsoever
	SYSTEM_release_mutex(MUTEX_IOS_LOCK_VALUE);

	// if we are in middle of a file write, we then close out this file
	FILE_fclose_force();
}

// Note:
//
// iOS/Android device:  Master as they scan and discover device, 
//                      GATT client as they ask for data from BLE device (Nordic wristband)
//
//
// Nordic wristband:    Slave as they advertice and are discovered eventually. They also accept a connection request
//                      GATT server as they have data and respond to client request
//
// Data flow -
//
// Wristband -> iOS/Android dev:   using NOTIFY, as the sever sends data to the client without the client requesting it 
//
// iOS/Android dev -> Wristband:   using WRITE, as the client needs to send data to the server
I32U BTLE_services_init()
{
	BLE_CTX *r = &cling.ble;
	I32U   err_code;
#ifndef _CLING_PC_SIMULATION_
	ble_uuid_t ble_uuid;
	
	// Clear all BTLE states
	_radio_state_cleanup();
	
	// Get total available TX buffer
	sd_ble_tx_buffer_count_get(&r->tx_buf_available);
	
	// Add service
	BLE_UUID_BLE_ASSIGN(ble_uuid, UUID_ADV_SERVICE);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &r->service_handle);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}
	
	// Server - Notify characteristics (for sending packets)
	ble_char_add(UUID_TX_SP, 1);
	ble_char_add(UUID_TX_START, 2);
	ble_char_add(UUID_TX_MIDDLE, 3);
	ble_char_add(UUID_TX_END, 4);
	
	// Write characteristics (for sending packets)
	ble_char_add(UUID_RX_SP, 5);
	ble_char_add(UUID_RX_START, 6);
	ble_char_add(UUID_RX_MIDDLE, 7);
	ble_char_add(UUID_RX_END, 8);
	
	return NRF_SUCCESS;
#else
	return 0;
#endif
}

void BTLE_disconnect(I8U reason)
{
#ifndef _CLING_PC_SIMULATION_
	BLE_CTX *r = &cling.ble;
	uint32_t err_code;
	
	Y_SPRINTF("[BTLE] disconnect BLE for reason: %d", reason);
	
	err_code = sd_ble_gap_disconnect(r->conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION ); //BLE_HCI_STATUS_CODE_SUCCESS);
	APP_ERROR_CHECK(err_code);
	
	r->btle_State = BTLE_STATE_DISCONNECTING;
#endif
}

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
#ifndef _CLING_PC_SIMULATION_
static void _on_connect(ble_evt_t * p_ble_evt)
{
	BLE_CTX *r = &cling.ble;
	
	r->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	
	r->btle_State = BTLE_STATE_CONNECTED;
	
	r->packet_received_ts = CLK_get_system_time();
	
	// Just connected, BLE connection is in a default fast mode
	r->b_conn_params_updated = FALSE;
	cling.system.conn_params_update_ts = CLK_get_system_time();
	sd_ble_tx_buffer_count_get(&r->tx_buf_available);
	
	Y_SPRINTF("[BTLE] Connected! ");
}
#endif
/**@brief Function for handling the Disconnect event.
 *
 */
#ifndef _CLING_PC_SIMULATION_
static void _on_disconnect()
{
	BLE_CTX *r = &cling.ble;

  r->conn_handle = BLE_CONN_HANDLE_INVALID;
	r->btle_State = BTLE_STATE_DISCONNECTED;
	
  Y_SPRINTF("[BTLE] disconnected! ");
	
	// Clear all BTLE states
	_radio_state_cleanup();
	
	// Reset minute streaming amount;
	BTLE_reset_streaming();
	
	// Stop notifying vibration
	NOTIFIC_stop_notifying();
	
	if (r->disconnect_evt & BLE_DISCONN_EVT_FAST_CONNECT)
		return;
	
	if (!r->disconnect_evt) {
//		NOTIFIC_start_idle_alert();
	}
	
	if (OTA_if_enabled()) {
		
		N_SPRINTF("[BTLE] reboot if BLE disconnected in middle of OTA! ");
	
		SYSTEM_restart_from_reset_vector();
	}
}

void BTLE_reset_streaming()
{
	BLE_CTX *r = &cling.ble;
	// Reset minute streaming amount;
	r->streaming_minute_file_amount = 0;
	r->streaming_minute_file_index = 0;
	r->streaming_minute_scratch_amount = 0;
	r->streaming_minute_scratch_index = 0;

	cling.system.mcu_reg[REGISTER_MCU_CTL] &= ~CTL_IM;	
	
	N_SPRINTF("[BTLE] Reset BTLE streaming");
}
#endif

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_hrs       Heart Rate Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
#if 0
I8U tblTEST[FAT_SECTOR_SIZE];
I32U tbl_base = 0;
static I8U num_load_dev_info = 0;
#endif

#ifndef _CLING_PC_SIMULATION_
static void _on_write(ble_evt_t * p_ble_evt)
{
	BLE_CTX *r = &cling.ble;
	ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	I8U *rec_buf = cling.gcp.pkt_buff;
	I8U i;

	if (p_evt_write->len != 20) return;
	
	// Set index for correct characteristics
	rec_buf[0] = 0xff;
	
	if (r->char_handles[4].value_handle == p_evt_write->handle) {
		rec_buf[1] = 0xe5;
	} else if (r->char_handles[5].value_handle == p_evt_write->handle) {
		rec_buf[1] = 0xe6;
	} else if (r->char_handles[6].value_handle == p_evt_write->handle) {
		rec_buf[1] = 0xe7;
	} else if (r->char_handles[7].value_handle == p_evt_write->handle) {
		rec_buf[1] = 0xe8;
	} else {
		return;
	}

	// Fill up the rest of packet
	for (i = 0; i < 20; i++) {
		rec_buf[i+2] = p_evt_write->data[i];
	}
	
	cling.gcp.b_new_pkt = TRUE;
	
	N_SPRINTF("[BTLE] a new packet");

	r->packet_received_ts = CLK_get_system_time();
	
}

void BTLE_execute_adv(BOOLEAN b_fast)
{
	BLE_CTX *r = &cling.ble;
	
	if (!BATT_is_charging()) {
		if (BATT_is_low_battery() && LINK_is_authorized()) {
				Y_SPRINTF("[BTLE] BLE low power");
				r->btle_State = BTLE_STATE_IDLE;
				return;
		}
	}
	
	if (!cling.system.b_powered_up)
		return;
	
	// if UI is ON or battery is in charging state, we should start advertising again
	if (b_fast)
		r->adv_mode = BLE_FAST_ADV;
	else 
		r->adv_mode = BLE_SLOW_ADV;
	
	HAL_advertising_start();

	r->btle_State = BTLE_STATE_ADVERTISING;
	
	Y_SPRINTF("[BTLE] start advertising.");
}
            
void BTLE_on_ble_evt(ble_evt_t * p_ble_evt)
{
		BLE_CTX *r = &cling.ble;
	
    switch (p_ble_evt->header.evt_id)
    {
				case BLE_EVT_TX_COMPLETE:
					r->tx_buf_available += (I8U) p_ble_evt->evt.common_evt.params.tx_complete.count;
					break;

        case BLE_GAP_EVT_CONNECTED:
            _on_connect(p_ble_evt);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:

            _on_disconnect();
            break;
            
        case BLE_GATTS_EVT_WRITE:
            _on_write(p_ble_evt);
            break;

				case BLE_GAP_EVT_SEC_PARAMS_REQUEST:

            break;

        case BLE_GAP_EVT_AUTH_STATUS:
						Y_SPRINTF("[BLE] AUTH GAP EVT - write CCCD");
            break;

				case BLE_GAP_EVT_TIMEOUT:

            if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_ADVERTISING)
            { 
                if (r->btle_State == BTLE_STATE_ADVERTISING) {
									if (r->adv_mode == BLE_ADV_SLEEP) {
										r->btle_State = BTLE_STATE_IDLE;
									} else {
										BTLE_execute_adv(FALSE);
										Y_SPRINTF("[BTLE] Advertising timeout ");
									}
                } 
            }
						break;
				        
				case BLE_GATTC_EVT_TIMEOUT:
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
				
						Y_SPRINTF("[BTLE] GATTS - BLE disconnect");
				    if(BTLE_is_connected())
						  BTLE_disconnect(BTLE_DISCONN_REASON_GATT_TIMEOUT);
            break;

        default:
            break;
    }
}
#endif
BOOLEAN BTLE_tx_buffer_available()
{
	BLE_CTX *r = &cling.ble;
	
	if (r->tx_buf_available > 0)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN BTLE_Send_Packet(I8U *data, I8U len)
{
#ifndef _CLING_PC_SIMULATION_
	BLE_CTX *r = &cling.ble;
	uint16_t               hvx_len;
	ble_gatts_hvx_params_t hvx_params;
	I8U ch_idx = 0;
	uint32_t err_code;
	
	// Input parameter check
	if (data == NULL || (len != CP_UUID_PKT_SIZE))
		return FALSE;
	
	N_SPRINTF("[BTLE] %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,",
	data[2], data[3], data[4],data[5], data[6], data[7], data[8], data[9],
	data[10], data[11], data[12], data[13], data[14],data[15], data[16], data[17], data[18], data[19],data[20], data[21], );
	
	if (data[1] == 0xe1) {
		ch_idx = 0;
	} else if (data[1] == 0xe2) {
		ch_idx = 1;
	} else if (data[1] == 0xe3) {
		ch_idx = 2;
	} else if (data[1] == 0xe4) {
		ch_idx = 3;
	} else {
		return FALSE;
	}
	
	hvx_len = len-2;

	memset(&hvx_params, 0, sizeof(hvx_params));
	
	hvx_params.handle   = r->char_handles[ch_idx].value_handle;
	hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
	hvx_params.offset   = 0;
	hvx_params.p_len    = &hvx_len;
	hvx_params.p_data   = data+2;
	
	
	
	err_code = sd_ble_gatts_hvx(r->conn_handle, &hvx_params);
	
	if (NRF_SUCCESS != err_code) {
		N_SPRINTF("[BTLE] packet sent failed!");
		
		if (BLE_ERROR_NO_TX_BUFFERS == err_code) {
			r->tx_buf_available = 0;
		}
		return FALSE;
	}
	
	if (r->tx_buf_available>1) {
		// Decrease the buffer
		r->tx_buf_available --;
	}
	
	N_SPRINTF("[BTLE] packet sent OK (%d)!", r->tx_buf_available);

	return TRUE;
#else
	return TRUE;
#endif
}
#ifndef _CLING_PC_SIMULATION_

// AES has to go through API call as ECB is restricted from SoftDevice
void BTLE_aes_encrypt(I8U *key, I8U *in, I8U *out)
{
	// Set up hal_aes using new key and init vector
	nrf_ecb_hal_data_t ecb_ctx;
	
	// Set ECB key
	memcpy(ecb_ctx.key, key, SOC_ECB_KEY_LENGTH);
	// Set ECB text input
	memcpy(ecb_ctx.cleartext, in, SOC_ECB_CLEARTEXT_LENGTH);
	// Encryption
	sd_ecb_block_encrypt(&ecb_ctx);
	
	// get encrypted text
	memcpy(out, ecb_ctx.ciphertext, SOC_ECB_CLEARTEXT_LENGTH);
}

#endif
//
// The decryption does NOT work at the point as Nordic
// AES ECB hardware block only supports encrypt mode
//
void BTLE_aes_decrypt(I8U *key, I8U *in, I8U *out)
{
	AES_CTX ctx;

	// Generate sbox code table
	AES_generateSBox();

	// Set keys
	AES_set_key(&ctx, key, NULL, AES_MODE_128);

	AES_convert_key(&ctx);

	AES_cbc_decrypt(&ctx, in, out, AES_BLOCKSIZE*4);
}

// 
// -------------- OLD ---------------
//


BOOLEAN BTLE_is_connected()
{
	BLE_CTX *r = &cling.ble;
	
	if (r->btle_State == BTLE_STATE_CONNECTED) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN BTLE_is_advertising()
{
	BLE_CTX *r = &cling.ble;
	
	if (r->btle_State == BTLE_STATE_ADVERTISING) {
		return TRUE;
	} else {
		return FALSE;
	}
}
	
BOOLEAN BTLE_is_idle()
{
	BLE_CTX *r = &cling.ble;
	
	if (r->btle_State == BTLE_STATE_IDLE) {
		return TRUE;
	} else {
		return FALSE;
	}
}
	
BOOLEAN BTLE_is_streaming_enabled(void)
{
	I32U t_curr = CLK_get_system_time();
	
	// If BLE is disconnected, go return FALSE
	if (cling.ble.btle_State != BTLE_STATE_CONNECTED)
		return FALSE;
	
	// If streaming is turned off, go return FALSE;
	if (!(cling.system.mcu_reg[REGISTER_MCU_CTL] & CTL_IM)) {
		return FALSE;
	}
	
	// If Streaming second based activity packets is more than 5, go return FALSE
	if (cling.ble.streaming_second_count > BLE_STREAMING_SECOND_MAX) {
		return FALSE;
	}
	
	if (t_curr > (cling.ble.packet_received_ts+30000))
		return FALSE;

	return TRUE;
}



static void _disconnect_clean_up()
{
	BLE_CTX *r = &cling.ble;
#ifndef _CLING_PC_SIMULATION_
	// Reboot
	if (r->disconnect_evt & BLE_DISCONN_EVT_REBOOT) {
		Y_SPRINTF("[BTLE] disconn event: reboot");
		
		// clear factory reset flag
		r->disconnect_evt &= ~BLE_DISCONN_EVT_REBOOT;
		
		// Jumpt to reset vector
		SYSTEM_restart_from_reset_vector();
	
		return;
	}
	
	// Factory reset
	if (r->disconnect_evt & BLE_DISCONN_EVT_FACTORY_RESET) {
		Y_SPRINTF("[BTLE] disconn event: factory reset");
		// De-authorize
		LINK_deauthorize();

		// System format disk along with all data erasure
		SYSTEM_format_disk(TRUE);

		// Wipe out all user data
		memset(&cling.user_data, 0, sizeof(USER_DATA));
		
		// clear factory reset flag
		r->disconnect_evt &= ~BLE_DISCONN_EVT_FACTORY_RESET;
		
		// Jumpt to reset vector
		SYSTEM_restart_from_reset_vector();
	
		return;
	}
	

#endif
	if (r->disconnect_evt & BLE_DISCONN_EVT_FAST_CONNECT) {
		r->disconnect_evt &= ~BLE_DISCONN_EVT_FAST_CONNECT;
		return;
	}
	
	// If BLE disconnects in middle of OTA, restart the device
	if (OTA_if_enabled()) {
		Y_SPRINTF("[BTLE] OTA reboot at end of disconnect clean-up");
		SYSTEM_restart_from_reset_vector();
	}
}

BOOLEAN BTLE_streaming_authorized()
{
	I32U t_curr = CLK_get_system_time();
  BLE_CTX *r = &cling.ble;

	// Nothing gets streamed out if OTA is turned on
	if (OTA_if_enabled()) {
		if (t_curr > (r->packet_received_ts + 30000)) {
			Y_SPRINTF("[BTLE] OTA no response for 30 seconds -> reboot ");
			SYSTEM_restart_from_reset_vector();
		}
		return FALSE;
	}
		
	// Streaming second based activity packets
	if (r->streaming_second_count > BLE_STREAMING_SECOND_MAX) {
		// Switch to slow connection mode if the streaming part is done
		if (HAL_set_slow_conn_params()) {
			N_SPRINTF("Streaming ends: Slow Connection interval(%d)", cling.system.reconfig_cnt);
		}
		return FALSE;
	} else {
		if (t_curr > (r->packet_received_ts + 20000)) {
#ifndef _CLING_PC_SIMULATION_
			BTLE_reset_streaming();
#endif
			// if we haven't seen anything in about 20 seconds, switch to slow connection mode
			if (HAL_set_slow_conn_params()) {
				N_SPRINTF("No packets: Slow Connection interval");
			}
		} else {
				// Update amount of data needs to be streamed
				if (!r->streaming_minute_scratch_amount) {
					BTLE_update_streaming_minute_entries();
				}
		}
	}
		
	// Make sure streaming is allowed
	if (!(cling.system.mcu_reg[REGISTER_MCU_CTL] & CTL_IM)) {
		return FALSE;
	}


	// Minute data has higher priority
	if (r->streaming_minute_scratch_index < r->streaming_minute_scratch_amount) {
		#if 0
		// If elapsed time is greater than 1 hour, switch to fast mode.
		if ((t_curr > (r->streaming_ts + 3600000)) || (r->streaming_ts == 0)) {
			if (HAL_set_conn_params(TRUE)) {
				Y_SPRINTF("Streaming starts: FAST Connection interval");
			}
		}
		#endif
		if (CP_create_streaming_minute_msg(SYSTEM_TRACKING_SPACE_SIZE)) {
			return TRUE;
		}
	}


	// All the past activity data in file system
	if (r->streaming_minute_file_index < r->streaming_minute_file_amount) {
		if (r->minute_file_entry_index < MINUTE_STREAMING_FILE_ENTRIES) {
			if (CP_create_streaming_file_minute_msg(MINUTE_STREAMING_FILE_SIZE)) {
				return TRUE;
			}
		}

		// Move onto next file, and upload its content
		r->streaming_minute_file_index++;
		if (r->streaming_minute_file_index < r->streaming_minute_file_amount) {
			r->minute_file_entry_index = 0;
			N_SPRINTF("--- get next minute file: %d, %d --", r->streaming_minute_file_index, r->streaming_minute_file_amount);
			r->streaming_file_addr = FILE_get_next_file_with_prefix((I8U *)"epoch", 5, TRUE);
			if (r->streaming_file_addr) {
				CP_create_streaming_file_minute_msg(MINUTE_STREAMING_FILE_SIZE);

				return TRUE;
			}
		} else {

			FILE_get_next_file_with_prefix((I8U *)"epoch", 5, TRUE);
			N_SPRINTF("--- delete minute file --");
		}
	}
	
	// then come the second data 
	if (t_curr > (r->streaming_ts + 1000)) {

		// Streaming seconds means we are done with syncing, so switch to slow connection mode
		HAL_set_slow_conn_params();

		// For background activity streaming, we default media to be BLE.
		CP_create_streaming_daily_msg();
		r->streaming_ts = t_curr;	// Recording streaming time stamp
		r->streaming_second_count++;
		N_SPRINTF("[BLE] activity streaming (%d)", r->streaming_second_count);//streaming one packet");
	}

	return FALSE;
}

void BTLE_update_streaming_minute_entries()
{
	BLE_CTX *r = &cling.ble;
	
	//
	// 32 KB scratch pad, so that is equivalent to 2048 entries
	r->streaming_minute_scratch_amount = (SYSTEM_TRACKING_SPACE_SIZE>>4);
	r->streaming_minute_scratch_index = 0;
	
	// Each file is 4 KB, so that is equivalent to 256 entries
	//
	r->streaming_minute_file_amount = FILE_exists_with_prefix((I8U *)"epoch", 5);
	r->streaming_minute_file_index = 0;
	Y_SPRINTF("Streaming amount, flash: %d, file: %d", r->streaming_minute_scratch_amount, r->streaming_minute_file_amount);
	if (r->streaming_minute_file_amount)
	{
		// Get the start address and reset entry index
		r->streaming_file_addr = FILE_get_next_file_with_prefix((I8U *)"epoch", 5, FALSE);
		r->minute_file_entry_index = 0;
		#if 0
		if (HAL_set_conn_params(TRUE)) {
			N_SPRINTF("Streaming Files: FAST Connection interval");
		}
		#endif
	}
	//
	// Reset all the streaming flags
	//
	cling.gcp.streaming.pending = FALSE;
	cling.gcp.streaming.packet_need_ack = FALSE;
	
}

void BTLE_state_machine_update()
{
#ifndef _CLING_PC_SIMULATION_

  BLE_CTX *r = &cling.ble;

	switch (r->btle_State) {
		case BTLE_STATE_IDLE:
			break;
		case BTLE_STATE_DISCONNECTED:
			// If device is disconnected, check on all the pending events
			if (r->disconnect_evt) {
				_disconnect_clean_up();
			}
			if (r->adv_mode == BLE_FAST_ADV) {
				// For whatever reason device is disconnected we should start advertising again
				BTLE_execute_adv(TRUE);
			} else {
				BTLE_execute_adv(FALSE);
			}
			break;
		case BTLE_STATE_DISCONNECTING:
			// Wait for disconnected
			break;
		case BTLE_STATE_CONNECTING:
			break;
		case BTLE_STATE_CONNECTED:
			if (LINK_is_authorized()) {
				
				// Streaming control
				BTLE_streaming_authorized();
			}
			break;
		case BTLE_STATE_ADVERTISING:
			// Nothing, waiting for either advertising timeout or connected
			break;
		default:
			break;
	}
#endif
}

/*
 * Input: pointer of radio software version (minor release build number)
 *        the initial version number is 0, maximum version number is 127
 *
 * Nordic BLE SDK:  current version - 4.4.2.33551
 * Return: TRUE -- radio software version number is accessed
 *         FALSE -- radio software version number isn't accessed
 */
BOOLEAN BTLE_get_radio_software_version(I8U *radio_sw_ver)
{
#ifndef _CLING_PC_SIMULATION_
	ble_version_t ver;
	
	sd_ble_version_get(&ver);

	*radio_sw_ver = ver.subversion_number;
#endif
	return TRUE;
}

