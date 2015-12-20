//
//  File: hal.h
//  
//  Description: Hardware abstruct layer header
//
//  Created on Feb 26, 2014
//
#ifndef __HAL_HEADER__
#define __HAL_HEADER__

#ifndef _CLING_PC_SIMULATION_	
#include "device_manager.h"
#include "ancs.h"
#endif
#include "standard_types.h"

#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)      /**< Delay after connection until security request is sent, if necessary (ticks). */

#define APP_TIMER_PRESCALER                  0                                          /**< Value of the RTC1 PRESCALER register. */

#define FLASH_PAGE_SYS_ATTR                 (BLE_FLASH_PAGE_END - 3)                    /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                     (BLE_FLASH_PAGE_END - 1)                    /**< Flash page used for bond manager bonding information. */

#define DEAD_BEEF                            0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

void HAL_init(void);
void HAL_device_manager_init(BOOLEAN b_delete);
void HAL_advertising_start(void);
BOOLEAN HAL_set_conn_params(BOOLEAN b_fast);
BOOLEAN HAL_set_slow_conn_params(void);
void HAL_disconnect_for_fast_connection(void);

#endif
