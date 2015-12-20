/***************************************************************************/
/**
 * File: dbg.c
 * 
 * Description: All debugging code comes in here
 *
 * Created on Feb 26, 2014
 *
 ******************************************************************************/
#include "main.h"
#include "uicoTouch.h"

void DEBUG_create_dbg_msg(I8U *msg, I8U len)
{
	CP_CTX *g = &cling.gcp;
	CP_TX_CTX *t = &g->tx;

	// Check whether we still have packets to deliver
	// our BLE message has to wait
	if (g->state == CP_MCU_STAT_TX_SENDING)
		return;

	// In case that an ack packet is needed.
	if (t->state != CP_TX_IDLE)
		return;

	// Creat a new message
	t->msg_id ++;

	// Get message length
	//    
	t->msg_len = len;

	t->pkt.uuid[0] = (UUID_TX_START >> 8) & 0xff;
	t->pkt.uuid[1] = (UUID_TX_START & 0xff);
	t->pkt.id = t->msg_id;

	// Get message length (fixed length: 2 bytes)
	t->pkt.len[0] = 0;
	t->pkt.len[1] = 0;
	t->pkt.len[2] = len;

	// Filling up the message buffer
	t->msg_filling_offset = 0;
	t->msg_pos = 0;
	t->msg_fetching_offset = 0;

	// Type
	t->msg[t->msg_filling_offset++] = CP_MSG_TYPE_DEBUG_MSG;
	
	// message content
	memcpy(t->msg+1, msg, len);
	t->msg_filling_offset += len;

	// Create packet payload
	// Pending message delivery
	t->msg_type = CP_MSG_TYPE_DEBUG_MSG;
	t->state = CP_TX_PACKET_PENDING_SEND;
	g->state = CP_MCU_STAT_TX_SENDING;

	memcpy(&t->pkt.data, t->msg, CP_PAYLOAD_SIZE);

	t->msg_pos += CP_PAYLOAD_SIZE;
	t->msg_fetching_offset += CP_PAYLOAD_SIZE;
}

enum {
	ENABLE_OTA_DEBUG,
	TOUCH_OTA_READ,
	TOUCH_OTA_WRITE,
};

#if defined(_ENABLE_BLE_DEBUG_) || defined(_ENABLE_UART_)


void DBG_event_processing()
{
	I8U i, len;
	I8U buf[128];
	char displayBuf[128];
	
	if (!cling.dbg.b_pending)
		return;
	
	cling.dbg.b_pending = FALSE;
	
	if (cling.dbg.b_read) {
		cling.dbg.b_read = FALSE;
	}
	
	if (cling.dbg.b_write) {
		cling.dbg.b_write = FALSE;
		
		UICO_dbg_write_read(buf, cling.dbg.len, cling.dbg.buf);
		
		len = 0;
		for(i = 0; i < cling.dbg.len; i++)
			len+=sprintf(displayBuf+len, "%02x,",buf[i]);
		B_SPRINTF("[DBG] %s", displayBuf);
	}
}

#endif
