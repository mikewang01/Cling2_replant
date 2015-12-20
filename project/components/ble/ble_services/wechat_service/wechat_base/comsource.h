#ifndef __COMSOURCE_H__
#define __COMSOURCE_H__
#include <stdio.h>
#include <string.h>
//#include "app_uart.h"
#include "nrf51_bitfields.h"
#include "ble.h"
//#include "ble_bondmngr.h"
#include "nrf_soc.h"
#include "ble_advdata.h"
#include "ble_wechat_service.h"
//#include "ble_stack_handler.h"
//#include "app_button.h"
#include "app_timer.h"
//#include "app_gpiote.h"
#include "ble_conn_params.h"
#include "ble_flash.h"
#include "app_error.h"
#include "mpbledemo2.h"


#define DEAD_BEEF											0xDEADBEEF             											/**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define	COMPANY_IDENTIFIER						0x0056

#define BUTTON_DETECTION_DELAY               APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)  /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define APP_TIMER_PRESCALER                  0                                         /**< Value of the RTC1 PRESCALER register. */

#define APP_TIMER_MAX_TIMERS                 8                                         /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              10                                         /**< Size of timer operation queues. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (20 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define FLASH_PAGE_SYS_ATTR                  (BLE_FLASH_PAGE_END - 3)                  /**< Flash page used for bond manager system attribute information. */
#define FLASH_PAGE_BOND                      (BLE_FLASH_PAGE_END - 1)                  /**< Flash page used for bond manager bonding information. */
#define BOND_DELETE true 																														/*�Ƿ�ɾ������Ϣ*/

#define MIN_CONN_INTERVAL                    MSEC_TO_UNITS(500, UNIT_1_25_MS)          /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL                    MSEC_TO_UNITS(1000, UNIT_1_25_MS)         /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                        0                                         /**< Slave latency. */
#define CONN_SUP_TIMEOUT                     MSEC_TO_UNITS(4000, UNIT_10_MS)           /**< Connection supervisory timeout (4 seconds). */

#define DEVICE_NAME                          "mydevice"                              /**< Name of device. Will be included in the advertising data. */

#define APP_GPIOTE_MAX_USERS                 5                                         /**< Maximum number of users of the GPIOTE handler. */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */



#define RX_PIN_NUMBER  0
#define TX_PIN_NUMBER  2
#define LED0 18
#define SYS_RESET 16


void uart_init(void);
void advertising_start(void);
void  wehchat_resource_init(void);
void wait_app_event(void);
void get_mac_addr(uint8_t *p_mac_addr);
void wechat_ble_evt_dispatch_local(ble_evt_t * p_ble_evt);
#endif

