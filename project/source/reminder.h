//
//  File: reminder.h
//  
//  Description: the header contains all reminder information
//
//  Created on Jan 6, 2014
//
#ifndef __REMINDER_HEADER__
#define __REMINDER_HEADER__

typedef enum {
	REMINDER_STATE_IDLE,
	REMINDER_STATE_ON,
	REMINDER_STATE_OFF,
	REMINDER_STATE_REPEAT,
	REMINDER_STATE_SECOND_REMINDER,
	REMINDER_STATE_CHECK_NEXT_REMINDER,
} REMINDER_STATE_TYPE;

typedef struct tagREMINDER_CTX {
	I32U ts;
	I8U hour;
	I8U minute;
	I8U b_valid;
	I8U vibrate_time;
	I8U second_vibrate_time;
	I8U ui_hh;
	I8U ui_mm;
	I8U ui_alarm_on;
	I8U total;
	REMINDER_STATE_TYPE state;
} REMINDER_CTX;

void REMINDER_set_next_reminder(void);
void REMINDER_state_machine(void);
I8U REMINDER_get_time_at_index(I8U index);
void REMINDER_setup(I8U *msg);

#endif  // __REMINDER_HEADER__
