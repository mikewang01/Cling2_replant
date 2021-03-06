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

/* Attention!
*  To maintain compliance with Nordic Semiconductor ASA�s Bluetooth profile
*  qualification listings, this section of source code must not be modified.
*/
#include "main.h"
#include "wechat_service_sports_simplified.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "wechat_port.h"
//#include <absacc.h>
#define INVALID_BATTERY_LEVEL 255

#define WECHAT_DATA_STEPS 		0X01
#define WECHAT_DATA_DISTANCE 	0X02
#define WECHAT_DATA_CALORIES  0X04
#define WECHAT_DATA_LENTH     0X03

#define UICO_UUID_SERVICE       0x5500
#define UICO_LEN_NOTIFY_UUID    0x5501
#define UICO_FRAME1_UUID        0x5502
#define UICO_FRAME2_UUID        0x5503
#define FIRST_HANDLE    0x0001
#define LAST_HANDLE     0xFFFF

/*WECHAT service and charator uuid*/
#define WECHAT_SERVICE_UUID       							0xFEE7
#define CURENT_PEDOMETER_MEASUREMENT_CHAR_UUID  0XFEA1
#define TRGET_STEPS_UUID_CHAR_UUID					  	0xFEA2

typedef enum {
    READ_REQ,      /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ      /**< Type identifying that this tx_message is a write request. */
} tx_request_t;

/**@brief Structure for writing a message to the peer, i.e. CCCD.
 */
typedef struct {
    uint8_t                  gattc_value[20];  /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                       /**< The GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding the data that will be transmitted to the connected central.
 */
typedef struct {
    uint16_t     conn_handle;  /**< Connection handle to be used when transmitting this message. */
    tx_request_t type;         /**< Type of message. (read or write). */
    union {
        uint16_t       read_handle;  /**< Read request handle. */
        write_params_t write_req;    /**< Write request message. */
    } req;
} tx_message_t;


struct current_pedometer_measurement {
    uint8_t flag;
    uint8_t step_count[WECHAT_DATA_LENTH];
    uint8_t distance[WECHAT_DATA_LENTH];
    uint8_t calories[WECHAT_DATA_LENTH];
};

/*data structor for target uuid*/
struct target_steps {
    uint8_t flag;
    uint8_t step_count[WECHAT_DATA_LENTH];
};



//static ble_uico_c_t * mp_ble_bas_c;                 /**< Pointer to the current instance of the BAS Client module. The memory for this is provided by the application.*/
static tx_message_t  m_tx_buffer[4];  /**< Transmit buffer for the messages that will be transmitted to the central. */
static uint32_t      m_tx_insert_index = 0;        /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t      m_tx_index        = 0;        /**< Current index in the transmit buffer containing the next message to be transmitted. */

#define TX_BUFFER_MASK 0x03
/**@brief Function for passing any pending request from the buffer to the stack.
 */

static void tx_buffer_process(ble_wechat_sports_t * p_bas);
static ble_wechat_sports_t * p_bas_local = NULL;
static void tx_send(ble_wechat_sports_t * p_ble_nus_c, uint16_t handle, uint8_t *str , uint16_t len);
int transfer_data_to_uico_touch(uint8_t *buffer_p, uint16_t tx_lenth, uint16_t *rx_lenth_p);
static int (*ble_recievd_callback)(uint8_t *, uint16_t) = NULL;
static uint16_t value_steps_char_update(ble_wechat_sports_t * p_bas, ble_gatts_char_handles_t handle, uint32_t steps, ...);

static bool seam_buffer = false;

#define IS_BUIFFER_LOCKED() (seam_buffer == true)
#define LOCK_BUFFER() (seam_buffer = true)
#define UNLOCK_BUFFER() (seam_buffer = false)

union sports_data {
    uint32_t data_orinal;
    uint8_t  data_array[sizeof(uint32_t)];
};

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_wechat_sports_t * p_bas, ble_evt_t * p_ble_evt)
{
    p_bas->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//    union sports_data i;

    for(int i = 0; i < WECHAT_SPORTS_SOPPURTED_CHAR_COUNT; i++) {
        /*on each connection, reset the value of target */
        value_steps_char_update(p_bas,  p_bas->wechat_sports_handles[i], wechat_get_step_count());	
        p_bas->is_char_handle_notify_enabled[i] = FALSE;
    }
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_wechat_sports_t * p_bas, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_bas->conn_handle = BLE_CONN_HANDLE_INVALID;

    /*notify up layer the mail of tets tool disconnected*/
    ble_uico_evt_t evt;
    evt.evt_type = BLE_UICO_EVT_TEST_TOOL_DISCONNECTED ;
    p_bas->evt_handler(p_bas, &evt);

		/*once disconect clear, steps  value*/
    for(int i = 0; i < WECHAT_SPORTS_SOPPURTED_CHAR_COUNT; i++) {
        /*RESET WECHAT HANDLE TO DEFAULT STATE WHEN GET DISCONNECTED*/
        //p_bas->wechat_sports_handles[i].cccd_handle  = BLE_GATT_HANDLE_INVALID;
        //p_bas->wechat_sports_handles[i].value_handle = BLE_GATT_HANDLE_INVALID;
        value_steps_char_update(p_bas,  p_bas->wechat_sports_handles[i], 0);	
        p_bas->is_char_handle_notify_enabled[i] = FALSE;
    }

}


/**@brief Function for notidfying pc the data over ble
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

int wechat_ble_notify_data_to_pc(ble_wechat_sports_t * p_bas, ble_gatts_char_handles_t handle, uint8_t *buffer, uint16_t lenth)
{
    tx_send(p_bas, handle.value_handle, buffer , lenth);
    return TRUE;
}
/**@brief Function for notidfying pc the data over ble
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

int ble_uico_recieve_data_from_pc_register(int (*p)(uint8_t *, uint16_t))
{
    if(p != NULL) {
        ble_recievd_callback = p;
    }

    return true;
}
/**@brief Function for handling the Write event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static uint16_t value_steps_char_update(ble_wechat_sports_t * p_bas, ble_gatts_char_handles_t handle, uint32_t steps, ...)
{
	
	  /*once disconeected reset the value of steps */
    union sports_data i;
    /*get current steps*/
    i.data_orinal = steps;
    struct current_pedometer_measurement para_upload;
    memset(&para_upload, 0, sizeof(struct current_pedometer_measurement));
    para_upload.flag |= WECHAT_DATA_STEPS;
    memcpy(para_upload.step_count, i.data_array, sizeof(para_upload.step_count));	
	
	 /*intiate gatts value*/
    ble_gatts_value_t gatts_value;
    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.len     = sizeof(struct target_steps);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t*)&para_upload;

    // Update database.
    return sd_ble_gatts_value_set(p_bas->conn_handle, handle.value_handle, &gatts_value);
}


static void on_write(ble_wechat_sports_t * p_bas, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    int i = 0;
#if 0
    Y_SPRINTF("[UICO BLE] recieved data lenth = %d handle = %d, org= %d", p_evt_write->len, p_evt_write->handle, p_bas->wechat_sports_handles[i].cccd_handle);
    for(int k = 0; k < p_evt_write->len ; k++) {
        Y_SPRINTF("0x%02x", p_evt_write->data[k]);
    }
#endif
#define NOTIFICATION_ENABLED_MESSAGE_LENTH  2
    for(i = 0; i < WECHAT_SPORTS_SOPPURTED_CHAR_COUNT ; i++) {
        int k = 0;
        /*this means this is a nofication enbale message*/
        if((p_evt_write->handle == p_bas->wechat_sports_handles[i].cccd_handle) && p_evt_write->len == NOTIFICATION_ENABLED_MESSAGE_LENTH) {
            Y_SPRINTF("[UICO BLE] nofification enable %d DATA RECIEVED lenth = %d", i, p_evt_write->len);
            for(k = 0; k < p_evt_write->len ; k++) {
                Y_SPRINTF("0x%02x", p_evt_write->data[k]);
            }

            /*notice uuper layer this device has been connected with pc tool*/
            ble_uico_evt_t evt;
						/*once notification is enable set the latest charactor value*/
            if(ble_srv_is_notification_enabled(p_evt_write->data)) {
								value_steps_char_update(p_bas,  p_bas->wechat_sports_handles[i], wechat_get_step_count());							
                evt.evt_type = BLE_UICO_EVT_NOTIFICATION_ENABLED;
                p_bas->is_char_handle_notify_enabled[i] = TRUE;
            } else {
							  /*on notification disablity clear steps charactor value
									this is specially for the situation when this decvice is connected to app
									which means even when you leave the pulic acount the connection is still alive
							*/
								value_steps_char_update(p_bas,  p_bas->wechat_sports_handles[i], 0);							
                evt.evt_type = BLE_UICO_EVT_NOTIFICATION_DISABLED;
                p_bas->is_char_handle_notify_enabled[i] = FALSE;
            }

            p_bas->evt_handler(p_bas, &evt);
            break;
        } else if((p_evt_write->handle == p_bas->wechat_sports_handles[i].value_handle) && p_evt_write->len != NOTIFICATION_ENABLED_MESSAGE_LENTH) {
            /* this means this is a normal write process*/
            Y_SPRINTF("[UICO BLE] normal write value_handle %d DATA RECIEVED lenth = %d", i, p_evt_write->len);
            break;
        }
    }
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_bas       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */

void ble_wechat_sports_on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_wechat_sports_t * p_bas = p_bas_local;
    N_SPRINTF("[bLE SPORTS] BLE EVENT id = %d", p_ble_evt->header.evt_id);
    switch(p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_bas, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_bas, p_ble_evt);
            break;

        case  BLE_EVT_TX_COMPLETE:
            N_SPRINTF("[sports]:BLE_EVT_TX_COMPLETEP\r\n");
            tx_buffer_process(p_bas);
            break;

        case BLE_GATTS_EVT_WRITE:
            N_SPRINTF("[sports]:BLE_GATTS_EVT_WRITE\r\n");
            on_write(p_bas, p_ble_evt);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            N_SPRINTF("[sports]:BLE_GATTC_EVT_WRITE_RSP\r\n");
            tx_buffer_process(p_bas);
            break;

        case BLE_GATTC_EVT_READ_RSP:
            N_SPRINTF("[sports]:BLE_GATTC_EVT_READ_RSP\r\n");
            break;

        case BLE_GATTS_EVT_HVC:
            N_SPRINTF("[sports]:BLE_GATTS_EVT_HVC recieved");
            //on_indicate_comfirm(p_wcs,p_ble_evt,p_data_handler);
            tx_buffer_process(p_bas);
            break;

        default:
            // No implementation needed.
            break;
    }

}


/**@brief Function for adding the Battery Level characteristic.
 *
 * @param[in]   p_bas        Battery Service structure.
 * @param[in]   p_bas_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

static uint32_t wechat_sport_char_add(ble_wechat_sports_t * p_bas, const ble_uico_init_t * p_bas_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
//    uint8_t             initial_battery_level[20];
//    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
//    uint8_t             init_len;
    int i = 0;

#if 1
    p_bas->is_notification_supported =   p_bas_init->support_notification;

    // Add Battery Level characteristic
    if(p_bas->is_notification_supported) {
        memset(&cccd_md, 0, sizeof(cccd_md));
        // According to BAS_SPEC_V10, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_bas_init->battery_level_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

#endif
    /*=================================================
    			current_pedometer_measurement char
    		uuid		    :0xFEA1
    		property		:Read;Indicate,Notify
    		neccessary 	:yes
    		informaiton :	realtime steps distance calories
    =================================================*/
    memset(&char_md, 0, sizeof(char_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    char_md.char_props.write =  1;
    char_md.char_props.read   = 1;
    char_md.char_props.indicate = 1; /*enable indication*/
    char_md.char_props.notify = (p_bas->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_bas->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, CURENT_PEDOMETER_MEASUREMENT_CHAR_UUID);

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_bas_init->battery_level_char_attr_md.read_perm;
    attr_md.write_perm = p_bas_init->battery_level_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1; /*enbale  Variable length attribute. */

//    initial_battery_level[0] = p_bas_init->initial_batt_level[0];

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    union sports_data t;
    /*get current steps*/
    t.data_orinal = wechat_get_step_count();
    struct current_pedometer_measurement para_upload;
    memset(&para_upload, 0, sizeof(struct current_pedometer_measurement));
    para_upload.flag |= WECHAT_DATA_STEPS;
    memcpy(para_upload.step_count, t.data_array, sizeof(para_upload.step_count));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(para_upload.step_count) + sizeof(para_upload.flag);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = 20;
    attr_char_value.p_value   = (uint8_t*)&para_upload;//initial_battery_level;

    p_bas->is_char_handle_notify_enabled[i] = FALSE;
    err_code = sd_ble_gatts_characteristic_add(p_bas->service_handle, &char_md,
               &attr_char_value,
               &p_bas->wechat_sports_handles[i++]);

    if(err_code != NRF_SUCCESS) {
        return err_code;
    }

    /*=====second charactor=============================================================================*/
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write =  1;
    char_md.char_props.read   = 1;
    char_md.char_props.indicate = 1; /*enable indication*/
    char_md.char_props.notify = (p_bas->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_bas->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    p_bas->is_char_handle_notify_enabled[i] = FALSE;
    BLE_UUID_BLE_ASSIGN(ble_uuid, TRGET_STEPS_UUID_CHAR_UUID);
    err_code = sd_ble_gatts_characteristic_add(p_bas->service_handle, &char_md,
               &attr_char_value,
               &p_bas->wechat_sports_handles[i++]);


    p_bas->report_ref_handle = BLE_GATT_HANDLE_INVALID;


    return NRF_SUCCESS;
}


void ble_unico_transmit_complete_handler(void * p_context)
{
    ble_wechat_sports_t * p_bas = (ble_wechat_sports_t *)p_context;
    /*notify up layer the message that data has been sended successfully*/
    ble_uico_evt_t evt;
    evt.evt_type = BLE_UICO_EVT_BUFFER_SENDED_SUCESSFULLY;
    p_bas->evt_handler(p_bas, &evt);

}
/**@brief Function for adding the Battery Level characteristic.
 *
 * @param[in]   p_bas        Battery Service structure.
 * @param[in]   p_bas_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
//static app_timer_id_t ble_wechat_sports_timer_id;
uint32_t ble_wechat_sports_init(ble_wechat_sports_t * p_bas, const ble_uico_init_t * p_bas_init, uint16_t service_handle)
{
//    uint32_t   err_code;
//    ble_uuid_t ble_uuid;

    p_bas_local = p_bas;
    // Initialize service structure
    p_bas->evt_handler               = p_bas_init->evt_handler;
    p_bas->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_bas->is_notification_supported = p_bas_init->support_notification;
    //p_bas->battery_level_last[0]        = NULL;

#if 0
    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, UICO_UUID_SERVICE);
    /*16bytes */
    ble_uuid.type = p_bas->uuid_type;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_bas->service_handle);
    if(err_code != NRF_SUCCESS) {
        return err_code;
    }
#endif

    p_bas->service_handle = service_handle;
    // Add battery level characteristic
    return wechat_sport_char_add(p_bas, p_bas_init);
}


/*********************************************************************
 * @fn      send_message
 *
 * @brief   This function is used for sending messge to specific task.
 *
 * @param   task_id,message_type.
 *
 * @return  LX_OK ,LX_OK
 */

static void tx_buffer_process(ble_wechat_sports_t * p_bas)
{
    if(m_tx_index != m_tx_insert_index) {
        uint32_t err_code;

        if(m_tx_buffer[m_tx_index].type == READ_REQ) {
            err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        } else {
            /*
            err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
            */
            /*if buffer is locked*/
            if(IS_BUIFFER_LOCKED()) {
                return;
            }
            LOCK_BUFFER();
            {
                ble_gatts_value_t gatts_value;
                // Initialize value struct.
                memset(&gatts_value, 0, sizeof(gatts_value));

                gatts_value.len     = m_tx_buffer[m_tx_index].req.write_req.gattc_params.len;
                gatts_value.offset  = m_tx_buffer[m_tx_index].req.write_req.gattc_params.offset;
                gatts_value.p_value = m_tx_buffer[m_tx_index].req.write_req.gattc_value;


                // Update database.
                err_code = sd_ble_gatts_value_set(m_tx_buffer[m_tx_index].conn_handle,
                                                  m_tx_buffer[m_tx_index].req.write_req.gattc_params.handle,
                                                  &gatts_value);


                if(err_code != NRF_SUCCESS) {
                    return ;
                }

                ble_gatts_hvx_params_t hvx_params;

                memset(&hvx_params, 0, sizeof(hvx_params));

                hvx_params.handle = m_tx_buffer[m_tx_index].req.write_req.gattc_params.handle;
                hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
                hvx_params.offset = 0;//20-lenth-1;//gatts_value.offset;
                hvx_params.p_len  = &(gatts_value.len);
                hvx_params.p_data = gatts_value.p_value;
                Y_SPRINTF("[ble_uico]: data lenth = %d..\r\n", *(hvx_params.p_len));


                err_code = sd_ble_gatts_hvx(m_tx_buffer[m_tx_index].conn_handle, &hvx_params);
                //err_code = sd_ble_gatts_hvx(m_tx_buffer[m_tx_index].conn_handle, &hvx_params);
            }
        }

        if(err_code == NRF_SUCCESS) {
            N_SPRINTF("[ble_uico]: SD Read/Write API returns Success..\r\n");
            m_tx_index++;
            m_tx_index &= TX_BUFFER_MASK;
            UNLOCK_BUFFER();
            // tx_buffer_process(p_bas);
        } else {
            UNLOCK_BUFFER();
            N_SPRINTF("[ble_uico]: SD Read/Write API returns error. This message sending will be "
                      "attempted again..");
        }
    } else {
#if 1

        /*notify up layer the message that data has been sended successfully*/
        ble_uico_evt_t evt;
        evt.evt_type = BLE_UICO_EVT_BUFFER_SENDED_SUCESSFULLY;
        p_bas->evt_handler(p_bas, &evt);
#else
        //app_timer_start(ble_wechat_sports_timer_id, SYSCLK_INTERVAL_10MS, p_bas);
#endif
    }
}

/*********************************************************************
 * @fn      tx_send
 *
 * @brief   send message
 *
 * @param   task_id,message_type.
 *
 * @return  LX_OK ,LX_OK
 */
static void tx_send(ble_wechat_sports_t * p_ble_nus_c, uint16_t handle, uint8_t *str , uint16_t size_l)
{
    tx_message_t * p_msg;

    if(size_l > 20) {
        return ;
    }

    /*if buffer is locked no operation should take place*/
    if(IS_BUIFFER_LOCKED()) {
        return;
    }
    /*lock buffer to occupy it*/
    LOCK_BUFFER();

    //????
    p_msg = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;
    //????
    p_msg->req.write_req.gattc_params.handle = handle;

    Y_SPRINTF("[ble uico]: sended handle= %d lenth = %d", p_msg->req.write_req.gattc_params.handle, size_l);
    p_msg->req.write_req.gattc_params.len = size_l;
    //??
    p_msg->req.write_req.gattc_params.p_value = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset = 0;
    //???
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    //????
    p_msg->conn_handle = p_ble_nus_c->conn_handle;
    //???????
    p_msg->type = WRITE_REQ;

    memcpy(p_msg->req.write_req.gattc_value, str, size_l);
    UNLOCK_BUFFER();
    //connected_pc_tool
    //????
    tx_buffer_process(p_ble_nus_c);
}

/*********************************************************************
 * @fn      wechat_sports_update_deamon
 *
 * @brief   wechat update routine used to maintain charactor1 used to update steps
 *
 * @param   task_id,message_type.
 *
* @return  TRUE sucessfully FALSE:FAILED
 */
#define WECHAT_SPORTS_INFORMATION_UPLOAD_CHARACTOR  0
#define WECHAT_SPORTS_TARGET_SET_CHARACTOR  1
uint16_t  wechat_sports_update_deamon()
{
    ble_wechat_sports_t *p_bas = p_bas_local;
    static uint32_t last_steps_get = 0;
    /*webchat sports uploaded routine*/
    if(wechat_delay() == TRUE) {
        if(p_bas->is_char_handle_notify_enabled[WECHAT_SPORTS_INFORMATION_UPLOAD_CHARACTOR] == TRUE &&  p_bas->conn_handle != BLE_CONN_HANDLE_INVALID) {
            union sports_data i;
            /*get current steps*/
            i.data_orinal = wechat_get_step_count();
            /*if steps changed, then update steps to wechat*/
            if(last_steps_get != i.data_orinal) {
                struct current_pedometer_measurement para_upload;
                memset(&para_upload, 0, sizeof(struct current_pedometer_measurement));
                /*set data type flag*/
                para_upload.flag |= WECHAT_DATA_STEPS;
                memcpy(para_upload.step_count, i.data_array, sizeof(para_upload.step_count));
                Y_SPRINTF("[ble uico]  steps  changed, update it");
                wechat_ble_notify_data_to_pc(p_bas,  p_bas->wechat_sports_handles[WECHAT_SPORTS_INFORMATION_UPLOAD_CHARACTOR], (uint8_t*)&para_upload, 4);
                last_steps_get = i.data_orinal;
            } else {
                Y_SPRINTF("[ble uico]  steps not changed");
            }
        } else if(p_bas->is_char_handle_notify_enabled[WECHAT_SPORTS_TARGET_SET_CHARACTOR] == TRUE &&  p_bas->conn_handle != BLE_CONN_HANDLE_INVALID) {
            /*target steps changed message*/
            //wechat_ble_notify_data_to_pc(p_bas,  p_bas->wechat_sports_handles[WECHAT_SPORTS_INFORMATION_UPLOAD_CHARACTOR], (uint8_t*)&para_upload, 4);
#if 0
            if(t % 3 == 0) {
                Y_SPRINTF("[ble uico] set target steps");
                //wechat_ble_notify_data_to_pc(p_bas,  p_bas->wechat_sports_handles[WECHAT_SPORTS_TARGET_SET_CHARACTOR], (uint8_t*)&B, sizeof(struct target_steps));
            }
#endif
        }
    }
    return TRUE;
}
/*********************************************************************
 * @fn      register_app_layer_callback
 *
 * @brief   regiter app layer uploading data callback function
 *
 * @param   task_id,message_type.
 *
* @return  TRUE sucessfully FALSE:FAILED
 */
static uint16_t (*app_layer_upload_stream_call_back)(uint16_t order) = NULL;
uint16_t register_upload_stream_callback(uint16_t (*p)(uint16_t))
{
    if(p == NULL) {
        app_layer_upload_stream_call_back = p;

    }
    return TRUE;
}

/*********************************************************************
 * @fn      register_app_layer_callback
 *
 * @brief   regiter app layer uploading data callback function
 *
 * @param   task_id,message_type.
 *
* @return  TRUE sucessfully FALSE:FAILED
 */
static uint16_t (*app_layer_obtain_system_data_call_back)(uint16_t order) = NULL;
uint16_t register_obtain_system_data_call_back_callback_function(uint16_t (*p)(uint16_t))
{
    if(p == NULL) {
        app_layer_obtain_system_data_call_back = p;
    }
    return TRUE;
}

