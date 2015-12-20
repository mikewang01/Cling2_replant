//
//  File: main.h
//  
//  Description: the main header contains release info, and global variables
//
//  Created on Feb 6, 2014
//
#ifndef __DEBUG_HEADER__
#define __DEBUG_HEADER__


#include "stdio.h"
#include "stdlib.h"

typedef struct tagDEBUG_CTX {
	BOOLEAN b_log;
} DEBUG_CTX;

extern FILE *fLog;

#define SYSLOG_DEBUG_MESSAGE(...) \
	{ I8U buf[128]; \
		I8U len;\
		cling.dbg.b_log = FALSE;\
		len = sprintf(buf, __VA_ARGS__);\
		DEBUG_create_dbg_msg(buf, len);\
	}

#define TIMED_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, "%d-%d-%d, %d:%d:%d ", \
			cling.time.local.year,\
			cling.time.local.month,\
			cling.time.local.day,\
			cling.time.local.hour,\
			cling.time.local.minute,\
			cling.time.local.second);\
		cling.dbg.b_log = FALSE;\
		len += sprintf(buf+len, __VA_ARGS__);\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}

#define UTC_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, "%d ", \
			cling.time.time_since_1970);\
		cling.dbg.b_log = FALSE;\
		len += sprintf(buf+len, __VA_ARGS__);\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}

#define SYSCLK_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, "%d ", \
			CLK_get_system_time());\
		len += sprintf(buf+len, __VA_ARGS__);\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}
	
#define TIMED_PC_LOG(...) \
{ char buf[128]; \
	I8U len = sprintf(buf, "%d-%d-%d, %d:%d:%d ", \
	cling.time.local.year, \
	cling.time.local.month, \
	cling.time.local.day, \
	cling.time.local.hour, \
	cling.time.local.minute, \
	cling.time.local.second); \
	len += sprintf(buf + len, __VA_ARGS__); \
	len += sprintf(buf + len, "\r\n"); \
	printf(buf); \
}

#define TIMED_PC_FILE_LOG(...) \
{ char buf[128]; \
	I8U len = sprintf(buf, "%d-%d-%d, %d:%d:%d ", \
	cling.time.local.year, \
	cling.time.local.month, \
	cling.time.local.day, \
	cling.time.local.hour, \
	cling.time.local.minute, \
	cling.time.local.second); \
	len += sprintf(buf + len, __VA_ARGS__); \
	len += sprintf(buf + len, "\r\n"); \
	fprintf(fLog, "%s", buf); \
}

#define PURE_PC_FILE_LOG(...) \
{ char buf[128]; \
	I8U len = sprintf(buf, "%d-%d-%d, %d:%d:%d ", \
	cling.time.local.year, \
	cling.time.local.month, \
	cling.time.local.day, \
	cling.time.local.hour, \
	cling.time.local.minute, \
	cling.time.local.second); \
	len += sprintf(buf+len, __VA_ARGS__); \
	len += sprintf(buf + len, "\r"); \
	fprintf(fLog, "%s", buf); \
}

#define PURE_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, __VA_ARGS__);\
		cling.dbg.b_log = FALSE;\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}

//#define __DEBUG_BASE__

#ifdef __DEBUG_BASE__

#define N_SPRINTF(...)
#define Y_SPRINTF(...) TIMED_DEBUG_LOG(__VA_ARGS__)
#define I_SPRINTF(...) PURE_DEBUG_LOG(__VA_ARGS__)
	
#define SYSLOG_DEBUG(...) \
	{ if (cling.dbg.b_log) {\
			SYSLOG_DEBUG_MESSAGE(__VA_ARGS__);}\
	}
	
#else

#define N_SPRINTF(...)
#define Y_SPRINTF(...) PURE_PC_FILE_LOG(__VA_ARGS__)
#define I_SPRINTF(...)
#define B_SPRINTF(...)
#endif

void DBG_led_blinky(I32U interval);
void DBG_gpio_toggling(I8U idx);
void DBG_led_sim(void);
void DEBUG_create_dbg_msg(I8U *msg, I8U len);

void DBG_gpio_test_pin(I8U idx);
void DBG_gpio_set(I8U idx);
void DBG_gpio_clear(I8U idx);
void DBG_ui_demo(void);

#endif
