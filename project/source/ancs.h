/***************************************************************************//**
 * File ancs.h
 * 
 * Description: ancs service context.
 *
 *
 ******************************************************************************/
#ifndef _ANCS_H__
#define _ANCS_H__

#define _ENABLE_ANCS_

#ifndef _CLING_PC_SIMULATION_
#ifdef _ENABLE_ANCS_
#include "standard_types.h"

#define   ANCS_FILTERING_OLD_MSG_DELAY_TIME            6000     /*6s*/ 
#define   ANCS_DISCOVERY_FAIL_DISCONNECT_DELAY_TIME    60000    /*60s*/  
#define   ANCS_SUPPORT_MAX_TITLE_LEN                   64       /*64 byte*/
#define   ANCS_SUPPORT_MAX_MESSAGE_LEN                 192      /*192 byte*/

typedef enum
{
	PARSE_STAT_COMMAND_ID,
	PARSE_STAT_NOTIFICATION_UID_1,
	PARSE_STAT_NOTIFICATION_UID_2,
	PARSE_STAT_NOTIFICATION_UID_3,
	PARSE_STAT_NOTIFICATION_UID_4,
	PARSE_STAT_ATTRIBUTE_TITLE_ID,
	PARSE_STAT_ATTRIBUTE_TITLE_LEN1,
	PARSE_STAT_ATTRIBUTE_TITLE_LEN2,
	PARSE_STAT_ATTRIBUTE_TITLE_READY,
	PARSE_STAT_ATTRIBUTE_MESSAGE_ID,
	PARSE_STAT_ATTRIBUTE_MESSAGE_LEN1,
	PARSE_STAT_ATTRIBUTE_MESSAGE_LEN2,
	PARSE_STAT_ATTRIBUTE_MESSAGE_READY
} ANCS_PARSE_STATES;


enum
{
  BLE_ANCS_STATE_IDLE,   	
  BLE_ANCS_STATE_DISCOVER_COMPLETE,   
  BLE_ANCS_STATE_DISCOVER_FAILED,            
  BLE_ANCS_STATE_NOTIF,                      
  BLE_ANCS_STATE_NOTIF_ATTRIBUTE        
};


typedef struct tagANCS_PACKET {
	
	I8U  title_len;
	I8U  message_len;
	I8U  buf[254];
} ANCS_PACKET;


enum {
	BOND_STATE_UNCLEAR=0,
	BOND_STATE_SUCCESSED,
	BOND_STATE_ERROR
};


typedef struct tagANCS_CONTEXT {
	// Global state
	I8U  state;	

	// Parse attrs state
  ANCS_PARSE_STATES	 parse_state;
	
  // Stored notific	data
  ANCS_PACKET  pkt;	
	
	// The number of the notification
	I8U  message_total;		
	
	// Notify reminder switch by user set.
	I16U supported_categories;
	
	// Filtering old notify flag.
  I8U filtering_flag;
	
	// Bond infomation delete state.
	I8U bond_state;
} ANCS_CONTEXT;


void  ANCS_service_add(void);
void  ANCS_nflash_store_one_message(I8U *data);
void  ANCS_state_machine(void);
#endif

#endif

#endif // _ANCS_H__

/** @} */

