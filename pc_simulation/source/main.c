/***************************************************************************/
/**
 * File: main.c
 * 
 * Description: Main Application for Lemon Band
 *
 * Created on Mar 25, 2015
 *
 ******************************************************************************/



#include <stdint.h>
#include <string.h>

#include "main.h"
#include "uicoTouch.h"

//#define _POWER_TEST_
//#define _NOR_FLASH_SPI_TEST_

CLING_MAIN_CTX cling;

#ifdef _POWER_TEST_

static void _power_test_powerdown(BOOLEAN b_sd_enabled)
{	
	GPIO_init();
	
	NRF_UART0->ENABLE= 0;//0x1;   
	NRF_SPI0->ENABLE = 0;//0x1;   
	NRF_TWI0->ENABLE = 0;//0x1;   
	NRF_SPI1->ENABLE = 0;//0x1;   
	NRF_TWI1->ENABLE = 0;//0x1;   
	NRF_SPIS1->ENABLE = 0;//0x1;   
	NRF_ADC->ENABLE = 0;//0x1;  
	NRF_AAR->ENABLE = 0;//0x1;   
	NRF_CCM->ENABLE = 0;//0x1;   
	NRF_QDEC->ENABLE = 0;//0x1;   
	NRF_LPCOMP->ENABLE = 0;//0x1; 

	GPIO_system_powerdown();

	// Enter main loop.
	while (LOOP_FOREVER)
	{
		if (b_sd_enabled) {
			// Feed watchdog every 4 seconds upon RTC interrupt
			Watchdog_Feed();
			sd_app_evt_wait();
			GPIO_system_powerdown();
		} else {
		  __wfe();
		}
	}
}
#endif

#ifdef _NOR_FLASH_SPI_TEST_
void _nor_flash_spi_test()
{
	I32U i, j, err_cnt;
	I32U add = 0;
	I32U buf[32];
	I8U *temp;
	I32U index, len, cnt;
	
	// Nor Flash initialize
	NOR_init();

	// Read ID
	NOR_readID((I8U *)buf);
	temp = (I8U *)buf;
	N_SPRINTF("ID: %02x, %02x", temp[0], temp[1]);
	
	// Erase the whole chip
	#if 0
	add = 0;
	for (i = 0; i < FLASH_SECTOR_COUNT; i ++) {
			N_SPRINTF("[FS] erase: %d, 64K", add);
		NOR_erase_block_64k(add);
		add += 65536;
		Watchdog_Feed();
	}
	#else
		NOR_ChipErase();

	#endif
	temp = (I8U *)buf;
	
	// Read out one byte just to confirm
	NOR_readData(0x0fffff, 1, temp);
	N_SPRINTF("Value at 0x00ffff: 0x%02x", temp[0]);
	
	cnt = 1000;
	
	// Program Flash
	add = 160;
	len = 128;
	for (i = 0; i < cnt; i++) {
		index = 0;
		for (j = 0; j < len; j++) {
			temp[j] = index ++;
		}
		NOR_pageProgram(add, len, temp);
		add += len;
		Watchdog_Feed();
	}
	
	// Read out the data
	add = 160;
	len = 128;
	err_cnt = 0;
	for (i = 0; i < cnt; i++) {
		temp = (I8U *)buf;
		NOR_readData(add, len, temp);
		for (j = 0; j < len; j++) {
			if (j != temp[j]) {
				err_cnt ++;
				N_SPRINTF("[MAIN] %d, %d", j, temp[j]);
			}
		}
		/*
		temp = (I8U *)&buf[16];
			N_SPRINTF("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,", 
				temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
			*/
		add += len;
		Watchdog_Feed();
	}
	
		N_SPRINTF("----  test nor flash: %d errors", err_cnt);

#if 0
	// Erase the whole chip
	add = 0;
	NOR_ChipErase();
	
	// Write the data
	add = 0;
	for (i = 0; i < 1; i++) {
		index = 0;
		for (j = 0; j < 128; j++) {
			temp[j] = index ++;
		}
		NOR_pageProgram(add, 32, (I8U *)buf);
		add += 128;
	}
		
	// Read out the data
	add = 0;
	for (i = 0; i < 30; i++) {
		NOR_readData(add, 8, (I8U *)buf);
		N_SPRINTF("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,", 
			temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
		add += 8;
	}
	#endif
	return;
	
	// Read out the data
	add = 0;
	err_cnt = 0;
	for (i = 0; i < 4096; i++) {
		index = 0;
		for (j = 0; j < 32; j++) {
			buf[j] = 0;
		}

		NOR_readData(add, 128, (I8U *)buf);
		
		for (j = 0; j < 128; j++) {
			if (j != temp[j]) {
				err_cnt ++;
			//N_SPRINTF("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,", 
				//temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
			}
		}
		
		add += 128;
	}
	N_SPRINTF("---- first test: %d errors", err_cnt);
	
#if 0
	
	N_SPRINTF("---- second test ---------");
	// Erase 32 K block
	add = 0;
	NOR_ChipErase();
	
	// Write the data
	add = 0;
	index = 0x11;
	for (i = 0; i < 30; i++) {
		for (j = 0; j < 8; j++) {
			temp[j] = index ++;
		}
		NOR_pageProgram(add, 8, (I8U *)buf);
		add += 8;
	}
		
	// Read out the data
	add = 0;
	for (i = 0; i < 30; i++) {
		NOR_readData(add, 8, (I8U *)buf);
		N_SPRINTF("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,", 
			temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
		add += 8;
	}
#endif	
}

#endif

void _power_manager_process(void)
{
	I32U err_code;
	
	// Turn off operation clock also
	RTC_stop_operation_clk();

	// If there is a pending packet, go straight out
	if (cling.gcp.b_new_pkt)
		return;
	
	if (cling.gcp.streaming.packet_is_acked) {
		cling.gcp.streaming.packet_is_acked = FALSE;
		return;
	}
	
	N_SPRINTF("[MAIN] EVT WAIT");
	
	if (LINK_is_authorized()) {
		// Configure TWI to be input to reduce power consumption
		GPIO_twi_disable(1);

		// Disable SPI bus
		spi_master_disable();
	}
#if 1
	// Main power management process
	err_code = sd_app_evt_wait();
	APP_ERROR_CHECK(err_code);
#else
	// using 		__WFI() if softdevice is not enabled.

	__WFI();
#endif
	N_SPRINTF("[MAIN] ^^^^^ ms:%d", CLK_get_system_time());
}

static void _cling_global_init()
{
	// Clean the cling global structure
	memset(&cling, 0, sizeof(CLING_MAIN_CTX));

	// Initialize the low power variables 
	cling.lps.b_low_power_mode = FALSE;

	// Version initialization
	cling.system.mcu_reg[REGISTER_MCU_HW_REV] = 0; // Board version is yet to be supported, initialized to 0.
	
	// BATTERY LEVEL
	cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 50; 
	
	// Software version number
	cling.system.mcu_reg[REGISTER_MCU_REVH] = BUILD_MAJOR_RELEASE_NUMBER;  // SW VER (HI)
	cling.system.mcu_reg[REGISTER_MCU_REVH] <<= 4;
	cling.system.mcu_reg[REGISTER_MCU_REVH] |= (BUILD_MINOR_RELEASE_NUMBER>>8)&0x0f;
	cling.system.mcu_reg[REGISTER_MCU_REVL] = (I8U)(BUILD_MINOR_RELEASE_NUMBER&0xff);  // SW VER (LO)
	
	// Restoring the time zone info
	// Shanghai: UTC +8 hours (units in 15 minutes) -> 8*(60/15) = 32 minutes
	cling.time.time_zone = 32;
	cling.time.time_since_1970 = 1400666400;
	RTC_get_local_clock(&cling.time.local);

	// Get current local minute
	cling.time.local_day = cling.time.local.day;
	cling.time.local_minute = cling.time.local.minute;

	// Cling reminder state machine
	cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;

	// Disable chip peripherals before starting up the system
	NRF_UART0->ENABLE= 0;//0x1;   
	NRF_SPI0->ENABLE = 0;//0x1;   
	NRF_TWI0->ENABLE = 0;//0x1;   
	NRF_SPI1->ENABLE = 0;//0x1;   
	NRF_TWI1->ENABLE = 0;//0x1;   
	NRF_SPIS1->ENABLE = 0;//0x1;   
	NRF_ADC->ENABLE = 0;//0x1;  
	//NRF_AAR->ENABLE = 0;//0x1;   
	//NRF_CCM->ENABLE = 0;//0x1;   
	NRF_QDEC->ENABLE = 0;//0x1;   
	NRF_LPCOMP->ENABLE = 0;//0x1;   
}

static void _system_startup()
{
	//
	// Algorithms (core pedometer)
	//
	// -- activity detection and classification
	// 
	// -- High sensitivity when system boots up
	//
	//
	TRACKING_initialization();
	PEDO_set_step_detection_sensitivity(TRUE);
	
	// Set system to low power mode
	// TO DO
	sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
	
	//
	// BTLE start advertising, switch state to "advertising", and wait for
	// either advertising timeout or connecteed.
	BTLE_execute_adv(TRUE);
	
	// UI init
	UI_init();
	
	// Configure power mode: 
	TOUCH_power_set(TOUCH_POWER_HIGH_20MS);

}

static void _system_module_poweroff()
{
			NRF_SPI0->ENABLE = 0;//0x1;   
			NRF_TWI0->ENABLE = 0;//0x1;   
			NRF_SPI1->ENABLE = 0;//0x1;   
			NRF_TWI1->ENABLE = 0;//0x1;   
			NRF_SPIS1->ENABLE = 0;//0x1;   
			NRF_ADC->ENABLE = 0;//0x1;  
	
			NRF_QDEC->ENABLE = 0;//0x1;   
			NRF_LPCOMP->ENABLE = 0;//0x1; 
			NRF_UART0->ENABLE = 0;
	// Have to investiage why these two module cannot be turned off
#if 0
			NRF_CCM->ENABLE = 0;//0x1;   

			NRF_AAR->ENABLE = 0;//0x1;
#endif
}

/**@brief Function for application main entry.
 */
int main(void)
{
	// Initialize Wristband firmware gloal structure
	_cling_global_init();
	
#ifdef _POWER_TEST_
	_power_test_powerdown(FALSE);
#endif
	// Hardware abstruct layer initialize.
	//
	// Including: UART, Keypad, Display, NOR flash, Physical IO, SPI, I2C, Sesnors
	//
	HAL_init();
	
#ifdef _NOR_FLASH_SPI_TEST_
	// Nor Flash testing ...
	_nor_flash_spi_test();
#endif
	
	// -------------------------
	// System modules initializaiton.
	//
	// -- Watch dog, Power management, file system, communication protocol
	//
	SYSTEM_init();

#ifdef _ENABLE_FONT_TRANSFER_
	FONT_flash_setup();
#endif

	//
	// Start up system
	//
	_system_startup();
	
	Y_SPRINTF("[MAIN] Entering main loop");
	
	// Enter main loop.
	while (LOOP_FOREVER)
	{		
		// Feed watchdog every 4 seconds upon RTC interrupt
		Watchdog_Feed();

		// Home key event detection
		HOMEKEY_click_check();

		if (cling.system.b_powered_up) {

			// Main power management process
			_power_manager_process();
			
			// For unauthorized device, we should shut it down if battery is lower than a threshold
			if (BATT_device_unauthorized_shut_down())
				continue;
					
		} else {
			// Turn off all system modules
			_system_module_poweroff();
			
			// Main power management process
			_power_manager_process();
			
			// If device is plugged into power, go restart the system.
			if (BATT_is_charging()) {
				SYSTEM_restart_from_reset_vector();
			}
			
			// detect a possible state change
			HOMEKEY_check_on_hook_change();
			
			N_SPRINTF("[MAIN] shut down event");
			
			continue;
		}

		// Battery charging state machine
		BATT_monitor_state_machine();
		
		// OLED state machine
		OLED_state_machine();

		// UI animation update
		UI_state_machine();

		// Bluetooth state machine update, 
		// this is low level BTLE transportation layer, check any incoming BTLE packet
		BTLE_state_machine_update();

		//
		// Running Generic communiation protocol at normal rate
		// --- update, packet processing, and send out pending packets
		//
    CP_state_machine_update();

		// Authorization state machine
		LINK_state_machine();

#ifdef _ENABLE_ANCS_	
    // ANCS state machine
    ANCS_state_machine();
#endif

		// User data save and store
		TRACKING_data_logging();
		
		// Interrupt response, including touch panel and homekey events
		GPIO_interrupt_handle();

		// Accelerometer data reading and algorithms
		SENSOR_accel_processing();

		// Idle alert state machine
		USER_state_machine();

		// If device enters low battery state, skip reminder & notification
		if (!BATT_is_low_battery()) {
			// Reminder state mahcine
			REMINDER_state_machine();
			
			// Notific state machine
			NOTIFIC_state_machine();
		}
		
#if 0
		HOMEKEY_sim();
#endif
		
#ifdef _ENABLE_PPG_
		// Heart rate measurement state machine
  	PPG_state_machine();
#endif

#ifdef _ENABLE_UV_
		// UV index measure
  	UV_state_machine();
#endif

    THERMISTOR_state_machine();
#if defined(_ENABLE_BLE_DEBUG_) || defined(_ENABLE_UART_)
		// Debug processing
		DBG_event_processing();
#endif
	}
}

/** 
 * @}
 */
/** 
 * @}
 */
