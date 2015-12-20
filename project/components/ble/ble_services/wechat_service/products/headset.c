
#include "mpbledemo2.h"
#include "epb_MmBp.h"
#include "ble_wechat_util.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "nrf_gpio.h"
#include <stdio.h>

mpbledemo2_info m_info = {CMD_BUTTON_NONE};
uint8_t md5_type_and_id[16] = {0x79, 0xA3, 0xB5, 0x9E, 0x72, 0x6A, 0xAC, 0x94, 0x15, 0x59, 0xCC, 0xBB, 0x1D, 0xE4, 0x2F, 0xDD};//0x79A3B59E726AAC941559CCBB1DE42FDD

mpbledemo2_state mpbledemo2Sta = {false,false,false,false,false,false};

void mpbledemo2_data_free_func(uint8_t *data, int len)
{
		free(data);
}

void mpbledemo2_data_produce_func(void *args, uint8_t **r_data, int *r_len)
{
		mpbledemo2_info *info = (mpbledemo2_info *)args;
		BaseRequest basReq = {NULL};
		static unsigned short seq = 0;
		int fix_head_len = sizeof(BpFixHead);
//htons the data before sent data
		BpFixHead fix_head = {0xFE, 1, 0, htons(ECI_req_ctlCmd), 0};
		switch (info->button_cmd)
		{
		case CMD_AUTH:
			{
			
				AuthRequest authReq = {&basReq, {md5_type_and_id, 16}, PROTO_VERSION, AUTH_PROTO, AUTH_METHOD, {NULL, 0}, false, {NULL, 0}, false, {NULL, 0}, false, {NULL, 0}};
				
				*r_len = epb_auth_request_pack_size(&authReq) + fix_head_len;
				*r_data = (uint8_t *)malloc(*r_len);
				//pack the authReq, ande the package's adress is from *r_data+fix_head_len.
				//for the fix_head is not packed, so the adress of authReq package shoule have an offset(fix_head_len).
				epb_pack_auth_request(&authReq, *r_data+fix_head_len, *r_len-fix_head_len);
		
				fix_head.nCmdId = htons(ECI_req_auth);
				fix_head.nLength = htons(*r_len);
				//the req and resp's nReq shoud not be zero
				fix_head.nSeq = htons(++seq);
				memcpy(*r_data, &fix_head, fix_head_len);
				
				return;
		}
		case CMD_INIT:
			{
			
				InitRequest initReq = {&basReq,false, {NULL, 0},false, {NULL, 0}};

				*r_len = epb_init_request_pack_size(&initReq) + fix_head_len;
				*r_data = (uint8_t *)malloc(*r_len);
				//pack the authReq, ande the package's adress is from *r_data+fix_head_len.
				//for the fix_head is not packed, so the adress of authReq package shoule have an offset(fix_head_len).
				
				epb_pack_init_request(&initReq, *r_data+fix_head_len, *r_len-fix_head_len);
		
				fix_head.nCmdId = htons(ECI_req_init);
				fix_head.nLength = htons(*r_len);
				//the req and resp's nReq shoud not be zero
				fix_head.nSeq = htons(++seq);
				memcpy(*r_data, &fix_head, fix_head_len);
				return;
		}
		case CMD_SENDDAT_TEST:
			{
				SendDataRequest sendDatReq ={&basReq,{NULL, 0}, false, NULL};

				*r_len = epb_send_data_request_pack_size(&sendDatReq) + fix_head_len;
				*r_data = (uint8_t *)malloc(*r_len);
				//pack the authReq, ande the package's adress is from *r_data+fix_head_len.
				//for the fix_head is not packed, so the adress of authReq package shoule have an offset(fix_head_len).
				
				epb_pack_send_data_request(&sendDatReq, *r_data+fix_head_len, *r_len-fix_head_len);
		
				fix_head.nCmdId = htons(ECI_req_sendData);
				fix_head.nLength = htons(*r_len);
				//the req and resp's nReq shoud not be zero
				fix_head.nSeq = htons(++seq);
				memcpy(*r_data, &fix_head, fix_head_len);
				return;
		}
	}
		ControlRequest controlReq = {&basReq, ECCT_buttonClick, false, {NULL, 0}};
		switch(info->button_cmd) {
		case CMD_BUTTON_CLICK:
					controlReq.control_cmd_type = ECCT_buttonClick;

				break;
		case CMD_BUTTON_LONG_PRESS_START:
					controlReq.control_cmd_type = ECCT_buttonLongPressStart;

				break;
		case CMD_BUTTON_LONG_PRESS_END: 
					controlReq.control_cmd_type = ECCT_buttonLongPressEnd;

				break;
		default:
				break;
		}
		
		*r_len = epb_control_request_pack_size(&controlReq) + fix_head_len;
		*r_data = (uint8_t *)malloc(*r_len);
		epb_pack_control_request(&controlReq, *r_data+fix_head_len, *r_len-fix_head_len);
		
		fix_head.nLength = htons(*r_len);
		fix_head.nSeq = htons(++seq);
		memcpy(*r_data, &fix_head, fix_head_len);
		
}

int mpbledemo2_data_consume_func(uint8_t *data, int len)
{
//	RecvDataPush recvDatPush ={};
		BpFixHead *fix_head = (BpFixHead *)data;
//		nrf_gpio_cfg_output(19);
		int fix_head_len = sizeof(BpFixHead);
		#ifdef CATCH_LOG
		printf("\r\n##Received data: ");
		uint8_t *d = data;
		for(uint8_t i=0;i<len;++i){
		printf(" %x",d[i]);}
		printf("\r\n CMDID: %d", ((fix_head->nCmdId >> 8)&0xff) | ((fix_head->nCmdId << 8) & 0xff00));
		printf("\r\n len: %d", ((fix_head->nLength >> 8)&0xff) | ((fix_head->nLength << 8) & 0xff00));
		printf("\r\n Seq: %d", ((fix_head->nSeq >> 8)&0xff) | ((fix_head->nSeq << 8) & 0xff00));
		#endif
	
		switch(((fix_head->nCmdId >> 8)&0xff) | ((fix_head->nCmdId << 8) & 0xff00))
	{
		case ECI_none:
		{
		}
			break;
		case ECI_resp_auth:
			{
				mpbledemo2Sta.auth_state = true;
				AuthResponse* authResp;
				authResp = epb_unpack_auth_response(data+fix_head_len,len-fix_head_len);
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'AuthResp'\r\n");
				#endif
				if(!authResp)
				{
					return 1;
				}
				#ifdef CATCH_LOG
				printf("\r\n unpack the 'ECI_resp_auth' successfully! \r\n");
				if(authResp->base_response == NULL)
				{
					printf("\r\n authResp->base_response is NULL! \r\n");
				}
				else 
				{
					printf("\r\n authResp->base_response is not NULL! \r\n");
				}
				printf("\r\n authResp->aes_session_key.data: %x \r\n",*authResp->aes_session_key.data);
				printf("\r\n authResp->aes_session_key.len: %x \r\n",authResp->aes_session_key.len);
				#endif		
				
			}
			break;
		case ECI_resp_sendData:
			{
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'ECI_resp_sendData'\r\n");
				#endif
			}
			break;
		case ECI_resp_init:
			{
				mpbledemo2Sta.init_state = true;
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'ECI_resp_init'\r\n");
				#endif			
			}
			break;
		case ECI_resp_ctlCmd:
		{
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'ECI_resp_ctlCmd'\r\n");
				#endif
		}
			break;
		case ECI_push_recvData:
		{
//		BasePush basPush = {NULL};
			RecvDataPush *recvDatPush;// = (RecvDataPush *)malloc(len-fix_head_len);
//		memset(recvDatPush,0,len-fix_head_len);
			recvDatPush = epb_unpack_recv_data_push(data+fix_head_len, len-fix_head_len);
			#ifdef CATCH_LOG
			printf("\r\n@@Received the 'ECI_push_recvData'\r\n");
			#endif
			if(!recvDatPush)
			{
				return 1;
			}
			#ifdef CATCH_LOG
			printf("\r\n unpack the 'ECI_push_recvData' successfully! \r\n");
			if(recvDatPush->base_push == NULL)
			{
				printf("\r\n recvDatPush->base_push is NULL! \r\n");
			}
			else 
			{
				printf("\r\n recvDatPush->base_push is not NULL! \r\n");
			}
			printf("\r\n recvDatPush->data.data: %x \r\n",*recvDatPush->data.data);
			printf("\r\n recvDatPush->data.len: %x \r\n",recvDatPush->data.len);
			if(recvDatPush->has_type)
			{
				printf("\r\n recvDatPush->has_type is ture! \r\n");
			}
			else 
			{
				printf("\r\n recvDatPush->has_type is false! \r\n");
			}
			#endif		
		}
			break;
		case ECI_push_switchView:
		{
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'ECI_push_switchView'\r\n");
				#endif
		}
			break;
		case ECI_push_switchBackgroud:
		{
				#ifdef CATCH_LOG
				printf("\r\n@@Received the 'ECI_push_switchBackgroud'\r\n");
				#endif
		}
			break;
		case ECI_err_decode:
			break;
	}
//		 = 0,
//		 = 10001,
//		 = 10002,
//		 = 10003,
//		 = 10004,
//		 = 20001,
//		 = 20002,
//		 = 20003,
//		 = 20004,
//		 = 30001,
//		 = 30002,
//		 = 30003,
//		 = 29999
		return 0;
}

data_handler mpbledemo2_data_handler = {
		.m_product_type = PRODUCT_TYPE_MPBLEDEMO2,
		.m_data_produce_func = &mpbledemo2_data_produce_func,
		.m_data_free_func = &mpbledemo2_data_free_func,
		.m_data_consume_func = &mpbledemo2_data_consume_func,
		.m_data_produce_args = &m_info,
		.next = NULL
};
