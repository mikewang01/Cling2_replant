
#ifndef MPBLEDEMO2
#define MPBLEDEMO2

#include "ble_wechat_util.h"
#include <stdbool.h>

#define CMD_BUTTON_NONE 0
#define CMD_BUTTON_CLICK 1
#define CMD_BUTTON_LONG_PRESS_START 2
#define CMD_BUTTON_LONG_PRESS_END 3
#define CMD_AUTH 4
#define CMD_INIT 5
#define CMD_SENDDAT_TEST 6

#define DEVICE_TYPE "gh_1bafe245c2cb"
#define DEVICE_ID "mpbledemo2"
#define PROTO_VERSION 0x010002
#define AUTH_PROTO 1
#define AUTH_METHOD 1
#define MAC_ADDR "C5:B1:83:86:B5:1D"


#define MD5_TYPE_AND_ID 0x79A3B59E726AAC941559CCBB1DE42FDD

typedef struct {
		int button_cmd;

} mpbledemo2_info;
 typedef struct {
		bool indication_state;
		bool light_state;
		bool auth_state;
		bool init_state;
		bool auth_send;
		bool init_send;
}mpbledemo2_state;


extern data_handler mpbledemo2_data_handler;
#endif
