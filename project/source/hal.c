/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_hrs_main main.c
 * @{
 * @ingroup ble_sdk_app_hrs
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "main.h"
#ifndef _CLING_PC_SIMULATION_
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_sdm.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_error_log.h"
//#include "ble_radio_notification.h"
//#include "ble_debug_assert_handler.h"
#include "ble_stack_handler_types.h"
#include "softdevice_handler.h"
#include "pstorage_platform.h"
#include "pstorage.h"
#include "ble_db_discovery.h"
#endif
#include "sysflash_rw.h"
#include "wechat_port.h"

#define _TOUCH_IC_PROGRAM_

#define IS_SRVC_CHANGED_CHARACT_PRESENT     0                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

// Required by Apple: adverting interval of 1000 ms for at least 60 seconds
//
#define APP_ADV_INTERVAL_FAST                     160                                      /**< The advertising interval (in units of 0.625 ms. This value corresponds to 100 ms), Apple suggested. */
#define APP_ADV_TIMEOUT_FAST_IN_SECONDS           30                                        /**< The advertising timeout in units of seconds. */

#define APP_ADV_INTERVAL_SLOW                     1636                                      /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1022.5 ms), Apple suggested. */
#define APP_ADV_TIMEOUT_SLOW_IN_SECONDS           240                                        /**< The advertising timeout in units of seconds. */

#ifndef _CLING_PC_SIMULATION_
#define FIRST_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(600*1000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (600 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY        APP_TIMER_TICKS(500, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT         5                                          /**< Number of attempts before giving up the connection parameter negotiation. */
#define MANUAL_CONN_PARAMS_UPDATE_DELAY      5000 //10000

//
#if 1
const I16U conn_param_active[4] = {
    MSEC_TO_UNITS(20, UNIT_1_25_MS),           /**< Minimum acceptable connection interval (40 milli-seconds). */
    MSEC_TO_UNITS(40, UNIT_1_25_MS),          /**< Maximum acceptable connection interval (1 second). */
    4,                                          /**< Slave latency. */
    MSEC_TO_UNITS(2000, UNIT_10_MS)           /**< Connection supervisory timeout (4 seconds). */
};
#else
const I16U conn_param_active[4] = {
    MSEC_TO_UNITS(380, UNIT_1_25_MS),           /**< Minimum acceptable connection interval (40 milli-seconds). */
    MSEC_TO_UNITS(400, UNIT_1_25_MS),          /**< Maximum acceptable connection interval (1 second). */
    4,                                          /**< Slave latency. */
    MSEC_TO_UNITS(6000, UNIT_10_MS)           /**< Connection supervisory timeout (4 seconds). */
};
#endif
#if 1
const I16U conn_param_idle[4] = {
    MSEC_TO_UNITS(250, UNIT_1_25_MS),           /**< Minimum acceptable connection interval (40 milli-seconds). */
    MSEC_TO_UNITS(400, UNIT_1_25_MS),          /**< Maximum acceptable connection interval (1 second). */
    4,                                          /**< Slave latency. */
    MSEC_TO_UNITS(6000, UNIT_10_MS)           /**< Connection supervisory timeout (4 seconds). */
};
#else
const I16U conn_param_idle[4] = {
    MSEC_TO_UNITS(40, UNIT_1_25_MS),           /**< Minimum acceptable connection interval (40 milli-seconds). */
    MSEC_TO_UNITS(250, UNIT_1_25_MS),          /**< Maximum acceptable connection interval (1 second). */
    4,                                          /**< Slave latency. */
    MSEC_TO_UNITS(6000, UNIT_10_MS)           /**< Connection supervisory timeout (4 seconds). */
};
#endif
static dm_application_instance_t             m_app_handle;                              /**< Application identifier allocated by device manager */

static ble_gap_adv_params_t                  m_adv_params;                              /**< Parameters to be passed to the stack when starting advertising. */
#endif
#ifndef _CLING_PC_SIMULATION_

static dm_handle_t               m_peer_handle;                            /**< Identifies the peer that is currently connected. */
#endif
#ifdef _ENABLE_ANCS_
static  ble_db_discovery_t        m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */
APP_TIMER_DEF(m_ancs_pair_timer_id);                     /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
static  ble_uuid_t m_adv_uuids[] = {{ANCS_UUID_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}}; /**< Universally unique service identifiers. */

/* Security requirements for this application. */
ble_gap_sec_params_t m_ancs_sec_params = {

    /* Perform bonding. */
    .bond = SEC_PARAM_BOND,
    /* Man In The Middle protection not required. */
    .mitm = SEC_PARAM_MITM,
    /* No I/O capabilities. */
    .io_caps = BLE_GAP_IO_CAPS_NONE,
    /* Out Of Band data not available. */
    .oob = SEC_PARAM_OOB,
    /* Minimum encryption key size. */
    .min_key_size = SEC_PARAM_MIN_KEY_SIZE,
    /* Maximum encryption key size. */
    .max_key_size = SEC_PARAM_MAX_KEY_SIZE
};

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void _sec_req_timeout_handler(void * p_context)
{
	dm_security_status_t status;
	I32U err_code;

	if(cling.gcp.host_type != HOST_TYPE_IOS)
		return;

	if(cling.ble.conn_handle != BLE_CONN_HANDLE_INVALID) {
		// Inquire current device whether has paired.
		err_code = dm_security_status_req(&m_peer_handle, &status);
		APP_ERROR_CHECK(err_code);

		if (status == NOT_ENCRYPTED) {
			// Start request pair with IOS phone.
			err_code = sd_ble_gap_authenticate(cling.ble.conn_handle, &m_ancs_sec_params);

			if(err_code == NRF_SUCCESS) {
				// Pair successed.
				Y_SPRINTF("[HAL] Successfully initiated authentication procedure.");
				cling.ancs.bond_state = BOND_STATE_SUCCESSED;
			}
			APP_ERROR_CHECK(err_code);
		}
	}
}


/**@brief Function for initializing the pair timer module.
 */
static void _ancs_pair_req_timer_init(void)
{
	I32U err_code;

	err_code = app_timer_create(&m_ancs_pair_timer_id,
															APP_TIMER_MODE_SINGLE_SHOT,
															_sec_req_timeout_handler);
	APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the database discovery module.*/
static void _ancs_service_discovery_init(void)
{
	I32U err_code;

	err_code = ble_db_discovery_init();
	APP_ERROR_CHECK(err_code);
}


#endif
#ifndef _CLING_PC_SIMULATION_

static uint32_t _device_manager_evt_handler(dm_handle_t const * p_handle,
        dm_event_t const  * p_event,
        ret_code_t        event_result)
{
  I32U err_code;

#ifdef _ENABLE_ANCS_
	if(event_result == DM_DEVICE_CONTEXT_FULL){
		// If it's not IOS phone,do nothing.
		if(cling.gcp.host_type == HOST_TYPE_IOS){
		  // If pair error,delete bond infomation and reset system.
		  Y_SPRINTF("[HAL] device manger error event result :%04x",event_result);
		  cling.ancs.bond_state = BOND_STATE_ERROR;					
		}
	}
#endif
	
  switch(p_event->event_id) {
		
		case DM_EVT_LINK_SECURED:
		{
			if(event_result != BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP)
			  APP_ERROR_CHECK(event_result);
			
#ifdef _ENABLE_ANCS_
			if(cling.gcp.host_type == HOST_TYPE_IOS){
        // Start discovery ancs service.
				err_code = ble_db_discovery_start(&m_ble_db_discovery, cling.ble.conn_handle);
				APP_ERROR_CHECK(err_code);
			}
#endif
			break;
		}
		case DM_EVT_CONNECTION:
		{
			if(event_result != BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP)
			  APP_ERROR_CHECK(event_result);
			
#ifdef _ENABLE_ANCS_
			m_peer_handle = (*p_handle);
			if (cling.gcp.host_type == HOST_TYPE_IOS) {
				// Start do pairing.
				err_code = app_timer_start(m_ancs_pair_timer_id, SECURITY_REQUEST_DELAY, NULL);
				APP_ERROR_CHECK(err_code);
			}
#endif
			break;
		}
		default:
			break;
    }

    return NRF_SUCCESS;
}


void HAL_disconnect_for_fast_connection()
{
#if 1
	BLE_CTX *r = &cling.ble;
	I32U t_curr = CLK_get_system_time();
	ble_gap_conn_params_t params;
	// If connection parameters ever get updated, then, disconnect and switch to high speed mode

#if 0
	if (!r->b_conn_params_updated) {
		Y_SPRINTF("[HAL] STILL IN fast connection");
		return;
	}
#endif
	
#if 0
	if (t_curr < (cling.system.conn_params_update_ts + MANUAL_CONN_PARAMS_UPDATE_DELAY))
		return;
#endif
	cling.system.conn_params_update_ts = t_curr;

	N_SPRINTF("[HAL] disconnect BLE for a fast connection");
	r->adv_mode = BLE_FAST_ADV;

#if 1
	if(cling.gcp.host_type == HOST_TYPE_IOS) {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_IOS);
	} else if(cling.gcp.host_type == HOST_TYPE_ANDROID) {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_ANDOIRD);
		r->disconnect_evt |= BLE_DISCONN_EVT_FAST_CONNECT;
	} else {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_NULL);
		r->disconnect_evt |= BLE_DISCONN_EVT_FAST_CONNECT;
	}
#endif
	
	//conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_IOS);
	params.min_conn_interval = conn_param_active[0];
	params.max_conn_interval = conn_param_active[1];
	params.slave_latency = conn_param_active[2];
	params.conn_sup_timeout = conn_param_active[3];
	//current_params = params;
	if(ble_conn_params_com_conn_params(params, true) == true) {
		N_SPRINTF("[HAL] STILL IN fast connection");
		return ;
	}
	
	uint32_t err_code = ble_conn_params_change_conn_params(&params);

	if (err_code == NRF_SUCCESS) {
		r->b_conn_params_updated = FALSE;
		Y_SPRINTF("[HAL] connection params update -- HIGH --");
		//err_code = app_timer_start(m_conn_params_timer_id, APP_TIMER_TICKS(1*1000, APP_TIMER_PRESCALER), &current_params);
		sd_ble_tx_buffer_count_get(&r->tx_buf_available);
	}
#endif
}
#endif

BOOLEAN HAL_set_slow_conn_params()
{
#ifndef _CLING_PC_SIMULATION_
	ble_gap_conn_params_t params;
	I32U err_code;
	BLE_CTX *r = &cling.ble;
	I32U t_curr = CLK_get_system_time();

	if (OTA_if_enabled())
		return FALSE;
#if 0
	if (r->b_conn_params_updated)
		return TRUE;
#endif
	if (t_curr < (cling.system.conn_params_update_ts + MANUAL_CONN_PARAMS_UPDATE_DELAY))
		return FALSE;


	r->adv_mode = BLE_SLOW_ADV;
	cling.system.conn_params_update_ts = t_curr;

	params.min_conn_interval = conn_param_idle[0];
	params.max_conn_interval = conn_param_idle[1];
	params.slave_latency = conn_param_idle[2];
	params.conn_sup_timeout = conn_param_idle[3];

	/*ckeck connection parameter*/
	if(ble_conn_params_com_conn_params(params, false) == true) {
		N_SPRINTF("[HAL] still in slow connection");
		return TRUE;
	}

#if 1
	if(cling.gcp.host_type == HOST_TYPE_IOS) {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_IOS);
	} else if(cling.gcp.host_type == HOST_TYPE_ANDROID) {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_ANDOIRD);
	} else {
		conn_params_mgr_set_device_type(CONN_PARAMS_MGR_DEVICE_NULL);
	}
#endif
	
	err_code = ble_conn_params_change_conn_params(&params);

	if (err_code == NRF_SUCCESS) {
		// Connection parameters get updated, which means BLE is in a slow connection mode
		r->b_conn_params_updated = TRUE;
		Y_SPRINTF("[HAL] connection params updated -- SLOW --");
		sd_ble_tx_buffer_count_get(&r->tx_buf_available);
	}
#endif
	
	return TRUE;
}

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
#ifndef _CLING_PC_SIMULATION_
	//nrf_gpio_pin_set(ASSERT_LED_PIN_NO);

	// This call can be used for debug purposes during development of an application.
	// @note CAUTION: Activating this code will write the stack to flash on an error.
	//                This function should NOT be used in a final product.
	//                It is intended STRICTLY for development/debugging purposes.
	//                The flash write will happen EVEN if the radio is active, thus interrupting
	//                any communication.
	//                Use with care. Uncomment the line below to use.
	//                ble_debug_assert_handler(error_code, line_num, p_file_name);
	Y_SPRINTF("---- hard fault (crash) ----");
	Y_SPRINTF((char *)p_file_name);
	N_SPRINTF("\r\n");
	Y_SPRINTF("err: %d, line: %d", error_code, line_num);
	BASE_delay_msec(1000);
	// On assert, the system can only recover with a reset.
	sd_nvic_SystemReset();
#endif
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function shall be used to setup all the necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void _gap_params_init(void)
{
#ifndef _CLING_PC_SIMULATION_
	I32U                err_code;
	ble_gap_conn_params_t   gap_conn_params;
	ble_gap_conn_sec_mode_t sec_mode;
	I8U DEVICE_NAME[20];

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

	// Get device name
	I8U len = SYSTEM_get_ble_device_name(DEVICE_NAME);

	N_SPRINTF("[SYSTEM] +++ ble GAP name: %s", DEVICE_NAME);

	err_code = sd_ble_gap_device_name_set(&sec_mode,
																				(const uint8_t *)DEVICE_NAME,
																				10);

	APP_ERROR_CHECK(err_code);

	// Set device as a generic outdoor device
	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR);
	APP_ERROR_CHECK(err_code);

	memset(&gap_conn_params, 0, sizeof(gap_conn_params));

	gap_conn_params.min_conn_interval = conn_param_active[0];
	gap_conn_params.max_conn_interval = conn_param_active[1];
	gap_conn_params.slave_latency     = conn_param_active[2];
	gap_conn_params.conn_sup_timeout  = conn_param_active[3];

	cling.system.conn_params_update_ts = CLK_get_system_time();

	err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 *
 * Required by Apple, the advertising data should at least include:
 *
 * 1) Flags, 2) TX Power level, 3) Local name, 4) Services
 *
 */
static void _advertising_init(uint8_t       flags)
#ifndef __WECHAT_SUPPORTED__
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t      err_code;
	ble_advdata_t advdata;
	ble_advdata_t srdata;
	ble_uuid_t adv_uuids[] = {
			{0xffe0,         BLE_UUID_TYPE_BLE}
	};

	// Build and set advertising data.
	memset(&advdata, 0, sizeof(advdata));

	// Keep minimum set of data during advertising
	advdata.flags				            = flags;
	advdata.uuids_complete.uuid_cnt = 1;
	advdata.uuids_complete.p_uuids  = adv_uuids;

#ifdef _ENABLE_ANCS_
	advdata.uuids_solicited.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
	advdata.uuids_solicited.p_uuids  = m_adv_uuids;
#endif
	
	// Put full name in scan response data, saving power
	memset(&srdata, 0, sizeof(srdata));
	srdata.name_type 								= BLE_ADVDATA_FULL_NAME;

	err_code = ble_advdata_set(&advdata, &srdata);
	APP_ERROR_CHECK(err_code);
#endif
}
#else
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t      err_code;
	ble_advdata_t advdata;
	ble_advdata_t rspdata;
	uint8_t	 	m_addl_adv_manuf_data[BLE_GAP_ADDR_LEN];
	ble_uuid_t adv_uuids[] = {
			{BLE_UUID_WECHAT_SERVICE,         BLE_UUID_TYPE_BLE},
			{0xffe0,         									BLE_UUID_TYPE_BLE},
	};

#if 1
	/*fill up advertisement configration*/
	uint8_t mac_address[BLE_GAP_ADDR_LEN];
	get_mac_addr(mac_address);
	memcpy(m_addl_adv_manuf_data, mac_address, BLE_GAP_ADDR_LEN);
	// Build and set advertising data
	memset(&advdata, 0, sizeof(advdata));
	advdata.name_type               = BLE_ADVDATA_NO_NAME;
	advdata.include_appearance      = false;
	advdata.flags                   = (flags);
	advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
	advdata.uuids_complete.p_uuids  = adv_uuids;

#endif
#ifdef _ENABLE_ANCS_
	advdata.uuids_solicited .uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
	advdata.uuids_solicited.p_uuids  = m_adv_uuids;
#endif
	/*initiate mac information and company id  as the requirement of wechat*/
	ble_advdata_manuf_data_t        manuf_data;
	manuf_data.company_identifier = COMPANY_IDENTIFIER;
	manuf_data.data.size          = sizeof(m_addl_adv_manuf_data);
	manuf_data.data.p_data        = m_addl_adv_manuf_data;	
	// Put full name in scan response data and mac information needed by wechat, saving power
	memset(&rspdata, 0, sizeof(rspdata));
	rspdata.name_type               = BLE_ADVDATA_FULL_NAME;
	rspdata.p_manuf_specific_data = &manuf_data;
	//ble_advdata_manuf_data_t
	err_code = ble_advdata_set(&advdata, &rspdata);
	APP_ERROR_CHECK(err_code);
#endif
}
#endif

void _ble_adv_params_state_machine()
{
#ifndef _CLING_PC_SIMULATION_
	// Initialize advertising parameters (used when starting advertising).
	memset(&m_adv_params, 0, sizeof(m_adv_params));


	m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
	m_adv_params.p_peer_addr = NULL;                           // Undirected advertisement.
	m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;

	Y_SPRINTF("------------ adv_mode: %d", cling.ble.adv_mode);

	switch(cling.ble.adv_mode) {
			case BLE_FAST_ADV:
					m_adv_params.interval    = APP_ADV_INTERVAL_FAST;
					m_adv_params.timeout     = APP_ADV_TIMEOUT_FAST_IN_SECONDS;
					cling.ble.adv_mode = BLE_SLOW_ADV;
					break;
			case BLE_SLOW_ADV:
					m_adv_params.interval    = APP_ADV_INTERVAL_SLOW;
					m_adv_params.timeout     = APP_ADV_TIMEOUT_SLOW_IN_SECONDS;
					cling.ble.adv_mode = BLE_ADV_SLEEP;
					break;
			default:
					// any other state, broadcasting at a slow rate
					m_adv_params.interval    = APP_ADV_INTERVAL_SLOW;
					m_adv_params.timeout     = APP_ADV_TIMEOUT_SLOW_IN_SECONDS;
					break;
	}
#endif
}

/**@brief Function for starting advertising.
 */
void HAL_advertising_start(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;

#if 0
	ble_gap_whitelist_t  whitelist;

	err_code = ble_bondmngr_whitelist_get(&whitelist);
	APP_ERROR_CHECK(err_code);
	if ((whitelist.addr_count != 0) || (whitelist.irk_count != 0)) {
		m_adv_params.fp          = BLE_GAP_ADV_FP_FILTER_CONNREQ;
		m_adv_params.p_whitelist = &whitelist;

		_advertising_init(BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED);

		Y_SPRINTF("[HAL] white list ...");
	} else
#endif
	{
		_advertising_init(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
	}

	_ble_adv_params_state_machine();

	err_code = sd_ble_gap_adv_start(&m_adv_params);
	APP_ERROR_CHECK(err_code);
#endif
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */

static void _services_init(void)
{
#ifndef _CLING_PC_SIMULATION_
	I32U err_code;

	// Cling private service add
	err_code = BTLE_services_init();
	APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
#ifndef _CLING_PC_SIMULATION_
  APP_ERROR_HANDLER(nrf_error);
#endif
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void _conn_params_init(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t               err_code;
	ble_conn_params_init_t cp_init;

	memset(&cp_init, 0, sizeof(cp_init));

	cp_init.p_conn_params                  = NULL;
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	cp_init.disconnect_on_fail             = true;
	cp_init.evt_handler                    = NULL;
	cp_init.error_handler                  = conn_params_error_handler;

	err_code = ble_conn_params_init(&cp_init);
	APP_ERROR_CHECK(err_code);
#endif
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
#ifndef _CLING_PC_SIMULATION_
static void _ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
	N_SPRINTF("[HAL] ble evt dispatch: event: %d", p_ble_evt->header.evt_id);

	dm_ble_evt_handler(p_ble_evt);
	
#ifdef _ENABLE_ANCS_
	if (!OTA_if_enabled()) {
		if (cling.gcp.host_type == HOST_TYPE_IOS) {
			ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
		}
	}
#endif
	
	ble_conn_params_on_ble_evt(p_ble_evt);

#ifdef _ENABLE_ANCS_
	if (!OTA_if_enabled()) {
		if (cling.gcp.host_type == HOST_TYPE_IOS) {
			ANCS_on_ble_evt(p_ble_evt);
		}
	}
#endif
	
	BTLE_on_ble_evt(p_ble_evt);
	
	wechat_ble_evt_dispatch(p_ble_evt);
}
#endif


/**@brief Handler for doing post actions on storage access complete
*/
static void storage_access_complete_handler(void)
{
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void _sys_evt_dispatch(uint32_t sys_evt)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t count;
	uint32_t err_code;

	pstorage_sys_event_handler(sys_evt);

	SYSFLASH_drv_event_handler(sys_evt);

	// Check if storage access is in progress.
	err_code = pstorage_access_status_get(&count);
	if ((err_code == NRF_SUCCESS) && (count == 0)) {
			storage_access_complete_handler();
	}
#endif
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void _ble_stack_init(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;

	// Initialize the SoftDevice handler module.
	// 8000 ms to calibrate the time (also minize power consumption)
	SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_8000MS_CALIBRATION, NULL);

	ble_enable_params_t ble_enable_params;
	memset(&ble_enable_params, 0, sizeof(ble_enable_params));

	ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
	err_code = sd_ble_enable(&ble_enable_params);
	APP_ERROR_CHECK(err_code);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_ble_evt_handler_set(_ble_evt_dispatch);
	APP_ERROR_CHECK(err_code);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_sys_evt_handler_set(_sys_evt_dispatch);
	APP_ERROR_CHECK(err_code);
#endif
}

/**@brief Function for the Bond Manager initialization.
 */
void HAL_device_manager_init(BOOLEAN b_delete)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t            err_code;
	dm_init_param_t        init_data;
	dm_application_param_t register_param;

	// Initialize persistent storage module.
	err_code = pstorage_init();
	APP_ERROR_CHECK(err_code);

	// Initialize the Bond Manager.
	init_data.clear_persistent_data            = b_delete; // Delete all bond information from flash.

	err_code = dm_init(&init_data);

	APP_ERROR_CHECK(err_code);

	memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

	register_param.sec_param.bond         = SEC_PARAM_BOND;
	register_param.sec_param.mitm         = SEC_PARAM_MITM;
	register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
	register_param.sec_param.oob          = SEC_PARAM_OOB;
	register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
	register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
	register_param.evt_handler            = _device_manager_evt_handler;
	register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

	err_code = dm_register(&m_app_handle, &register_param);
	APP_ERROR_CHECK(err_code);
#endif
}

#if 0
/**@brief Function for initializing the Radio Notification event.
 */
static void radio_notification_init(void)
{
	uint32_t err_code;

	err_code = ble_radio_notification_init(NRF_APP_PRIORITY_HIGH,
																				 NRF_RADIO_NOTIFICATION_DISTANCE_4560US,
																				 ble_flash_on_radio_active_evt);
	APP_ERROR_CHECK(err_code);
}
#endif

static void _ble_init()
{
#ifndef _CLING_PC_SIMULATION_

	// BLE stack init, enable softdevice
  _ble_stack_init();

#ifdef _ENABLE_ANCS_
  _ancs_service_discovery_init();
#endif

  _gap_params_init();

#ifdef _ENABLE_ANCS_
  // ANCS service add
  ANCS_service_add();
#endif
  _services_init();

#ifdef __WECHAT_SUPPORTED__
  wechat_init();
#endif
  //_conn_params_init();
#endif
}

/**@brief Function for application main entry.
 */
void HAL_init(void)
{
  GPIO_system_powerup();

#ifdef _ENABLE_UART_
	 // UART initialization/
	UART_init();
#else
	UART_disabled();
#endif
	// Timer init
  RTC_Init();
	
	// BLE initialization
	_ble_init();
	
	// GPIO initializaiton
	GPIO_init();
	

	
#ifdef _ENABLE_ANCS_
	//ANCS pairing req initialization
	_ancs_pair_req_timer_init();
#endif

#ifndef _CLING_PC_SIMULATION_
	// Enable TWI I2C 1
	GPIO_twi_init(1);
#endif
	//TOUCH_init();
	// UV sensor initialization
#ifdef _ENABLE_UV_
	//UV_Init();
#endif

#ifdef _ENABLE_PPG_
	// PPG sensor initialization
	//PPG_init();
#endif

  // Nor Flash initialize
  NFLASH_init();

  // System flash initialization
  SYSFLASH_drv_init();

	// Enable GIO interrupt
	GPIO_interrupt_enable();

	// Sensor initialization
	SENSOR_init();

	// Power measurement init
	BATT_init();

	// Initialize thermistor module
	THERMISTOR_init();

	// Keypad init
	HOMEKEY_click_init();

#ifdef _ENABLE_TOUCH_
	BASE_delay_msec(600);
	// Touch controller init
	//TOUCH_init();
#endif
}

/**
 * @}
 */
