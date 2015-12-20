//
//  File: main.h
//  
//  Description: the main header contains release info, and global variables
//
//  Created on Feb 26, 2014
//
#ifndef __MAIN_HEADER__
#define __MAIN_HEADER__

/* Generic macros (do not change across platforms) */
#define LOOP_FOREVER    1         /* makes while() statements more legible */

//#define _CLINGBAND_NFC_MODEL_

#define _CLINGBAND_UV_MODEL_

#ifdef _CLINGBAND_UV_MODEL_

#define _ENABLE_UV_

#endif

// 
// Global device id address
//
#define GLOBAL_DEVICE_ID_LEN 20

#include "standards.h"
#include "sim.h"
#include "system.h"
#include "base.h"
#include "hal.h"
#include "lis3dh.h"
#include "tracking.h"
#ifndef _CLING_PC_SIMULATION_
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_soc.h"
#endif
#include "fs_memcfg.h"
#include "fs_nflash_rw.h"
#include "fs_flash.h"
#include "fs_fat.h"
#ifndef _CLING_PC_SIMULATION_
#include "ble_flash.h"
#endif
#include "user.h"
#include "cp.h"
#include "RTC.h"
#include "link.h"
#include "sys_reg.h"
#include "btle.h"
#include "Watchdog.h"
#include "nflash_spi.h"
#include "sensor.h"
#include "ui.h"
#include "oled.h"
#include "thermistor.h"
#include "release_number.h"
#include "sysclk.h"
#include "uart.h"
#include "dbg.h"
#include "gpio.h"
#include "spi_master_api.h"
#include "batt.h"
#ifndef _CLING_PC_SIMULATION_
#include "nrf_drv_twi.h"
#include "nrf_adc.h"
#include "uv_calib.h"
#include "ble_db_discovery.h"
#endif
#include "uv.h"
#include "touch.h"
#include "butterworth.h"
#include "ppg.h"
#include "weather.h"
#include "ota.h"
#include "sleep.h"
#include "reminder.h"
#include "notific.h"
#ifdef _ENABLE_ANCS_
#include "ancs.h"
#include "ble_ancs_c.h"
#endif
#include "homekey.h"
#include "Font.h"
#include "ppg.h"

#define TWI_MASTER_UV       1
#define TWI_MASTER_UICO     1
#define TWI_MASTER_PPG      1

enum {
	MUTEX_NOLOCK_VALUE = 0,
	MUTEX_IOS_LOCK_VALUE,
	MUTEX_MCU_LOCK_VALUE,
};

enum {
	PSYS_ERR_OK,
	PSYS_ERR_FS_OPEN_FAIL,
	PSYS_ERR_FS_CREATE_FILE_FAIL,
	PSYS_ERR_FS_WRONG_DATA_FAT,
	PSYS_ERR_FS_WRONG_NAME_FAT,
	PSYS_ERR_FS_WRONG_FCLOSE,
	PSYS_ERR_ALTIMETER_INIT_FAIL,
};

typedef struct tagCLING_TIME_CTX {
	// Time difference
	I32U time_since_1970;
	// Tick count
	I32U tick_count;
	I8S time_zone;
	I8U local_day;
	I8U local_minute;
	I8U local_hour;
	SYSTIME_CTX local;

	// System clock
	I32U system_clock_in_sec;	// Sourced from RTC 
	
	// System clock interval
	BOOLEAN operation_clk_enabled;
	I32U operation_clk_start_in_ms;
	
	//
	BOOLEAN local_minute_updated;
	BOOLEAN local_noon_updated;
	BOOLEAN local_day_updated;
	
} CLING_TIME_CTX;

typedef struct tagLOW_POWER_STATIONARY_CTX {
	I32U ts;
	BOOLEAN b_low_power_mode;
	BOOLEAN b_accelerometer_fifo_mode;
	I8U int_count; // Interrupt counter
} LOW_POWER_STATIONARY_CTX;

typedef struct tagWHOAMI_CONTEXT {
	I8U accelerometer;
	I8U hssp;
	I8U nor[2];
	I8U touch_ver[3];
} WHOAMI_CTX;

typedef struct tagSYSTEM_CTX {
	// Connection parameter update
	I32U conn_params_update_ts;
	
	// Reset count - overall reset time since the factory reset
	// The count only works for authorized device.
	I16U reset_count;
	
	// System power status
	BOOLEAN b_powered_up;
	
	/// MCU Registers
	I8U mcu_reg[SYSTEM_REGISTER_MAX];
	
	// MCU peripheral requirements
	BOOLEAN b_spi_0_ON;  // SPI 0 (dedicated for OLED, accelerometer, flash)
	BOOLEAN b_twi_1_ON;  // SPI 0 (dedicated for OLED, accelerometer, flash)
} SYSTEM_CTX;

typedef struct tagCLING_MAIN_CTX {
	
	// Cling system related
	SYSTEM_CTX system;
	
	// weather context (5 days)
	WEATHER_CTX weather[MAX_WEATHER_DAYS];

	// System diagonostic variables
	WHOAMI_CTX whoami; 

	// Low power stationary mode
	LOW_POWER_STATIONARY_CTX lps;

	// Activity information including all daily total/minute, and on-going activity status.
	TRACKING_CTX activity;

	// Timing info
	CLING_TIME_CTX time;

	// Battery context
	BATT_CTX batt;

	// User related data
	USER_DATA user_data;
	
#if defined(_ENABLE_BLE_DEBUG_) || defined(_ENABLE_UART_)
	// Debug context
	DEBUG_CTX dbg;
#endif

	// Generic communication protocol
	CP_CTX gcp;

	// Pairing and authentication
	LINK_CTX link;

	// Touch panel context
	TOUCH_CTX touch;
	
	// Radio BKE state
	BLE_CTX ble;
#ifdef _ENABLE_ANCS_
	// ANCS (apple notification center service)
	ANCS_CONTEXT ancs;
#endif
	// UI state
	UI_ANIMATION_CTX ui;
	
	// LED state machine
	CLING_OLED_CTX oled;

	// Over the air update
	OTA_CTX ota;

  // HEART RATE state machine
  HEARTRATE_CTX hr;
	
	// Thermistor state context
	THERMISTOR_CTX therm;
	
	// Sleep monitoring state context
	SLEEP_CTX sleep;	
	
	// Reminder
	REMINDER_CTX reminder;

	// Notific 
	NOTIFIC_CTX notific;
	
	// Homekey state
	HOMEKEY_CLICK_STAT key;
	
	// uv index
	UV_CTX uv;
	
	// font context
	FONT_CTX font;
	
} CLING_MAIN_CTX;

// CLING main context
extern CLING_MAIN_CTX cling;


#endif
