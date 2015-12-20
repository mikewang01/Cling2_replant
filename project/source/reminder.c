/***************************************************************************/
/**
 * File: reminder.c
 * 
 * Description: user reminder processing
 *
 * Created on May 15, 2014
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>


#include "main.h"

#define REMINDER_ON_TIME_IN_MS 200
#define REMINDER_OFF_TIME_IN_MS 400

#define REMINDER_SECOND_REMINDER_LATENCY 2000

#define REMINDER_VIBRATION_REPEAT_TIME 2
#define SECOND_REMINDER_TIME 8

//static BOOLEAN reminder_testing_flag = TRUE;

void REMINDER_setup(I8U *msg)
{
	I32U data[32];
	I8U *pdata = (I8U*)data;

	FLASH_erase_App(SYSTEM_REMINDER_SPACE_START);
	BASE_delay_msec(50); // Latency before refreshing reminder space. (Erasure latency: 50 ms)
	cling.reminder.total = (msg[0] >> 1);
	if (cling.reminder.total > 32)
		cling.reminder.total = 32;

  if (cling.reminder.total>0)
		REMINDER_get_time_at_index(0);

	memcpy(pdata, msg + 1, 64);
	FLASH_Write_App(SYSTEM_REMINDER_SPACE_START, pdata, 64); // Maximum 32 entries
	Y_SPRINTF("[CP] reminder update, len: %d", cling.reminder.total);
	cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
}

void REMINDER_set_next_reminder()
{
	I8U i, hour, minute;
	BOOLEAN b_found = FALSE;
	I32U buffer[32];
	I8U *data = (I8U *)buffer;
	
	if (!LINK_is_authorized()) {
		cling.reminder.b_valid = FALSE;
		cling.reminder.hour = 0;
		cling.reminder.minute = 0;
		return;
	}
	
	if (cling.user_data.b_reminder_off_weekends) {
		// Check if today is Saturday or Sunday
		if (cling.time.local.dow > 4) {
			cling.reminder.b_valid = FALSE;
			cling.reminder.hour = 0;
			cling.reminder.minute = 0;
			return;
		}
	}
	
	if (cling.reminder.total) {
		FLASH_Read_App(SYSTEM_REMINDER_SPACE_START, (I8U *)buffer, 64);

		for (i = 0; i < cling.reminder.total; i++) {
			hour = *data++;
			minute = *data++;
			
			Y_SPRINTF("[REMINDER] current time: %d:%d, reminder: %d:%d", cling.time.local.hour, cling.time.local.minute, hour, minute);

			if (hour >= 24) break;
			if (minute >= 60) break;
			
			if (hour == cling.time.local.hour) {
				if (minute > cling.time.local.minute) {
					b_found = TRUE;				
					break;
				}
			} else if (hour > cling.time.local.hour) {
				b_found = TRUE;
				break;
			}
		}
	}

	if (b_found) {
		cling.reminder.b_valid = TRUE;
		cling.reminder.hour = hour;
		cling.reminder.minute = minute;
		
		N_SPRINTF("[REMINDER] found reminder: %d:%d", cling.reminder.hour, cling.reminder.minute);
	} else {
		cling.reminder.b_valid = FALSE;
		cling.reminder.hour = 0;
		cling.reminder.minute = 0;

		N_SPRINTF("[REMINDER] No new reminder is found");
	}
	
	if (cling.reminder.total>0) {
		REMINDER_get_time_at_index(0);
  }

	N_SPRINTF("[REMINDER] Set next reminder - done");
}

static BOOLEAN _check_reminder()
{
	if (!cling.reminder.b_valid)
		return FALSE;
	
	if ((cling.time.local.hour >= cling.reminder.hour) && (cling.time.local.minute >= cling.reminder.minute))
	{
		return TRUE;
	} else {
		return FALSE;
	}
}

void REMINDER_state_machine()
{
	I32U t_curr = CLK_get_system_time();
	
	if (OTA_if_enabled())
		return;

	if (cling.reminder.state != REMINDER_STATE_IDLE) {
		// Start 20 ms timer 
		RTC_start_operation_clk();
	}
	
	switch (cling.reminder.state) {
		case REMINDER_STATE_IDLE:
		{
			if (_check_reminder()) {
				Y_SPRINTF("[REMINDER] reminder is hit @ %d:%d", cling.time.local.hour, cling.time.local.minute);
				cling.reminder.state = REMINDER_STATE_ON;
				// Reset vibration times
				cling.reminder.vibrate_time = 0;
				cling.reminder.second_vibrate_time = 0;
				cling.reminder.ui_alarm_on = TRUE;
				UI_turn_on_display(UI_STATE_REMINDER, 1000);

			} else {
				if (cling.notific.state == NOTIFIC_STATE_IDLE) {
					GPIO_vibrator_set(FALSE);
				}
			}
			break;
		}
		case REMINDER_STATE_ON:
		{
			N_SPRINTF("[REMINDER] vibrator is ON, %d", t_curr);
			cling.reminder.ts = t_curr;
			GPIO_vibrator_on_block(REMINDER_ON_TIME_IN_MS);
			cling.reminder.state = REMINDER_STATE_OFF;
			break;
		}
		case REMINDER_STATE_OFF:
		{
			if (t_curr > (cling.reminder.ts + REMINDER_ON_TIME_IN_MS)) {
				N_SPRINTF("[REMINDER] vibrator is OFF, %d", t_curr);
				GPIO_vibrator_set(FALSE);
				cling.reminder.state = REMINDER_STATE_REPEAT;
				cling.reminder.ts = t_curr;
				cling.reminder.vibrate_time ++;
			}
			break;
		}
		case REMINDER_STATE_REPEAT:
		{
			if (t_curr > (cling.reminder.ts + REMINDER_OFF_TIME_IN_MS)) {
				if (cling.reminder.vibrate_time >= REMINDER_VIBRATION_REPEAT_TIME) {
					cling.reminder.state = REMINDER_STATE_SECOND_REMINDER;
					cling.reminder.second_vibrate_time ++;
					N_SPRINTF("[REMINDER] go second reminder, %d, %d", cling.reminder.second_vibrate_time, cling.reminder.vibrate_time);
				} else {
					cling.reminder.state = REMINDER_STATE_ON;
					N_SPRINTF("[REMINDER] vibrator repeat, %d, %d", cling.reminder.vibrate_time, t_curr);
				}
			}
			break;
		}
		case REMINDER_STATE_SECOND_REMINDER:
		{
			if (t_curr > (cling.reminder.ts + REMINDER_SECOND_REMINDER_LATENCY)) {
				if (cling.reminder.second_vibrate_time >= SECOND_REMINDER_TIME) {
					cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
					Y_SPRINTF("[REMINDER] Go check next: %d", cling.reminder.second_vibrate_time);
				} else {
					N_SPRINTF("[REMINDER] second reminder on");
					cling.reminder.state = REMINDER_STATE_ON;					
					// Reset vibration times
					cling.reminder.vibrate_time = 0;
				}
			}
			break;
		}
		case REMINDER_STATE_CHECK_NEXT_REMINDER:
		{
			REMINDER_set_next_reminder();
			cling.reminder.state = REMINDER_STATE_IDLE;
			N_SPRINTF("[REMINDER] STATE: CHECK NEXT");
			break;
		}
		default:
		{
			REMINDER_set_next_reminder();
			cling.reminder.state = REMINDER_STATE_IDLE;
			N_SPRINTF("[REMINDER] STATE: DEFAULT: %d", cling.reminder.state);
			break;
		}
	}
}

I8U REMINDER_get_time_at_index(I8U index)
{
	I8U new_idx, hh, mm;
	I32U i32_buf[32];
	I8U *data;;

	FLASH_Read_App(SYSTEM_REMINDER_SPACE_START, (I8U *)i32_buf, 64);

	data = (I8U *)i32_buf;
	new_idx = index;
	N_SPRINTF("[REMINDER] maximum entry: %d, %d", cling.reminder.total, new_idx);
	
	if (cling.reminder.total) {
		if (new_idx >= cling.reminder.total) {
			N_SPRINTF("[REMINDER] reset entry: %d, %d", cling.reminder.total, new_idx);
			new_idx = 0;
		}
		hh = data[new_idx<<1];
		mm = data[(new_idx<<1)+1];

    if (hh>=24 || mm>=60) {
			cling.reminder.ui_hh = 0xff;
			cling.reminder.ui_mm = 0xff;
			return 0;
	  }

		cling.reminder.ui_hh = hh;
		cling.reminder.ui_mm = mm;
		
		N_SPRINTF("[REMINDER] ui display: %d:%d", hh, mm);
		
		// In case that some rediculous number comes up, go reset
	} else {
		cling.reminder.ui_hh = 0xff;
		cling.reminder.ui_mm = 0xff;
		new_idx = 0;
	}
	
	return new_idx;
}

