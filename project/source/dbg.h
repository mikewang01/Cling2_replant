//
//  File: main.h
//  
//  Description: the main header contains release info, and global variables
//
//  Created on Feb 6, 2014
//
#ifndef __DEBUG_HEADER__
#define __DEBUG_HEADER__

typedef struct tagDEBUG_CTX {
	I8U b_log;
	I8U b_pending;
	I8U b_read;
	I8U b_write;
	I8U len;
	I8U buf[32];
} DEBUG_CTX;

#define SYSLOG_DEBUG_MESSAGE(...) \
	{ char buf[128]; \
		I8U len;\
		if (cling.dbg.b_log) {\
			len = sprintf(buf, __VA_ARGS__);\
			DEBUG_create_dbg_msg((I8U *)buf, len);\
		}\
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
		len += sprintf(buf+len, __VA_ARGS__);\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}

#define UTC_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, "%d ", \
			cling.time.time_since_1970);\
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
	I8U len = sprintf(buf, __VA_ARGS__); \
	len += sprintf(buf + len, "\r\n"); \
	fprintf(fLog, "%s", buf); \
}

#define PURE_DEBUG_LOG(...) \
	{ char buf[128]; \
		I8U len = sprintf(buf, __VA_ARGS__);\
		len += sprintf(buf+len, "\r\n"); \
		UART_print(buf);\
	}

//#define _ENABLE_BLE_DEBUG_

#define _ENABLE_UART_
	
#if defined(_ENABLE_UART_)

#define N_SPRINTF(...) 
#define Y_SPRINTF(...) TIMED_DEBUG_LOG(__VA_ARGS__)
#define I_SPRINTF(...) PURE_DEBUG_LOG(__VA_ARGS__)
#define B_SPRINTF(...) SYSLOG_DEBUG_MESSAGE(__VA_ARGS__)
	
#elif defined(_ENABLE_BLE_DEBUG_)
	
#define N_SPRINTF(...)
#define Y_SPRINTF(...) 
#define I_SPRINTF(...)
#define B_SPRINTF(...) SYSLOG_DEBUG_MESSAGE(__VA_ARGS__)

#else

#define N_SPRINTF(...)
#define Y_SPRINTF(...)
#define I_SPRINTF(...)
#define B_SPRINTF(...) 
#endif

void DEBUG_create_dbg_msg(I8U *msg, I8U len);
void DBG_event_processing(void);

#endif
