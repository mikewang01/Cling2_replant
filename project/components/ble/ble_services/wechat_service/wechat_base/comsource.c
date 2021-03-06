/*
*	WeChat mpbledemo2 common source
*
*	modified  by anqiren  2014/12/02  V1.0bat
*
**/

#include <stdio.h>
#include <string.h>
//#include "app_uart.h"
#include "nrf51_bitfields.h"
#include "ble.h"
//#include "ble_bondmngr.h"
#include "nrf_soc.h"
#include "ble_advdata.h"
#include "..\ble_wechat_service.h"
//#include "ble_stack_handler.h"
//#include "app_button.h"
#include "app_timer.h"
//#include "app_gpiote.h"
#include "ble_conn_params.h"
#include "ble_flash.h"
#include "app_error.h"
//#include "led.h"
#include "mpbledemo2.h"
#include "comsource.h"
#include "wechat_service_sports_simplified.h"
#include "wechat_port.h"



//static uint8_t	 															m_addl_adv_manuf_data[BLE_GAP_ADDR_LEN];
uint16_t               								m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
ble_gap_sec_params_t   								m_sec_params;                               /**< Security requirements for this application. */
ble_gap_adv_params_t  								m_adv_params;																/**< Parameters to be passed to the stack when starting advertising. */

extern data_handler 									*m_mpbledemo2_handler;

ble_wechat_t 													m_ble_wechat;

static uint16_t wechat_handle = 0;
static void ble_wechat_sports_parameter_init(uint16_t service_handle);

/**@brief Function for initializing the services that will be used by the application.
 *
 * @details Initialize the wechat services.
 */


static void wechat_services_init(void)
{

    uint32_t err_code;
#if (WECHAT_COMPELEX_PROTOCOL_SUPPORTED == 1 || WECHAT_SIMPLIFIED_PROTOCOL_SUPPORTED == 1)
    err_code = ble_wechat_add_service(&m_ble_wechat);
    APP_ERROR_CHECK(err_code);
#endif

#if (WECHAT_COMPELEX_PROTOCOL_SUPPORTED == 1 || WECHAT_SIMPLIFIED_PROTOCOL_SUPPORTED == 1)
    /*get wechat server handle number*/
    err_code = ble_wechat_add_characteristics(&m_ble_wechat);
    APP_ERROR_CHECK(err_code);
#endif

#if (WECHAT_SIMPLIFIED_PROTOCOL_SUPPORTED == 1)
    wechat_handle = ble_wechat_get_service_handle(&m_ble_wechat);
    ble_wechat_sports_parameter_init(wechat_handle);
#endif
}


/******************************************************************************
 * FunctionName : ble_uico_debug_evt_handler
 * Description  : uico layer message reciever and pricesser
 * Parameters   : ble_wechat_sports_t * p_bas uico ble object ble_uico_evt_t * specific event sended
 * Returns      : none
*******************************************************************************/
void ble_wechat_sports_evt_handler(ble_wechat_sports_t * p_bas, ble_uico_evt_t * p_evt)
{

    switch(p_evt->evt_type) {
            /**< Battery value notification enabled event. */

        case BLE_UICO_EVT_NOTIFICATION_ENABLED:

            break;
        case BLE_UICO_EVT_TEST_TOOL_DISCONNECTED:
        case BLE_UICO_EVT_NOTIFICATION_DISABLED:
            break;

        case BLE_UICO_EVT_BUFFER_SENDED_SUCESSFULLY:

            break;

        default:
            // No implementation needed.
            break;
    }

}

/******************************************************************************
 * FunctionName : ble_wechat_sports_parameter_init
 * Description  : initiate wechat simplified protocol
 * Parameters   : void
 * Returns      : none
*******************************************************************************/
static ble_wechat_sports_t   m_sports = {0};
static void ble_wechat_sports_parameter_init(uint16_t service_handle)
{
    uint32_t       err_code;

    ble_uico_init_t bas_init;
    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));
    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.cccd_write_perm);
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_char_attr_md.write_perm);

    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bas_init.battery_level_report_read_perm);
    bas_init.evt_handler          = ble_wechat_sports_evt_handler;
    bas_init.support_notification = true;
    bas_init.p_report_ref         = NULL;
    err_code = ble_wechat_sports_init(&m_sports, &bas_init, service_handle);
    APP_ERROR_CHECK(err_code);

}


/******************************************************************************
 * FunctionName : wechat_ble_evt_dispatch
 * Description  : event dispatch function used to distrivuted realted message
 * Parameters   : ble_evt_t * p_ble_evt: event object
 * Returns      : none
*******************************************************************************/
void wechat_ble_evt_dispatch_local(ble_evt_t * p_ble_evt)
{
#if (WECHAT_COMPELEX_PROTOCOL_SUPPORTED == 1)
    //ble_bondmngr_on_ble_evt(p_ble_evt);
    ble_wechat_on_ble_evt(&m_ble_wechat, p_ble_evt, m_mpbledemo2_handler);
    //ble_conn_params_on_ble_evt(p_ble_evt);
    m_mpbledemo2_handler->m_data_on_ble_evt_func(&m_ble_wechat, p_ble_evt);
#endif

#if (WECHAT_SIMPLIFIED_PROTOCOL_SUPPORTED == 1)
    //on_ble_evt(p_ble_evt);
    ble_wechat_sports_on_ble_evt(p_ble_evt);
#endif
}

/******************************************************************************
 * FunctionName : register_all_products
 * Description  : register realted callback function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
//function for register all products
void register_all_products(void)
{
    REGISTER(mpbledemo2);
}

void	data_handler_init(data_handler** p_data_handler, uint8_t product_type)
{
    if (*p_data_handler == NULL) {
        *p_data_handler = get_handler_by_type(product_type);
    }
}


/******************************************************************************
 * FunctionName : wehchat_resource_init
 * Description  : wechat resource init function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void wehchat_resource_init(void)
{
    //bond_manager_init();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n bond manager initi");
#endif

    register_all_products();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n register products!");
#endif

    data_handler_init(&m_mpbledemo2_handler,	PRODUCT_TYPE_MPBLEDEMO2);
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n data handler init!");

#endif

    APP_ERROR_CHECK(m_mpbledemo2_handler->m_data_init_func());
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n mpbledemo2 init");
#endif

//    gap_params_init();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n gap params init");
#endif

//    advertising_init();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n advertising init");
#endif

    wechat_services_init();
    //services_init();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n services init");
#endif

    // sec_params_init();
#ifdef CATCH_LOG
    Y_SPRINTF("\r\n sec params init");
#endif


}



