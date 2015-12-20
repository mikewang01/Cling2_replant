/***************************************************************************/
/**
 * File: user.c
 * 
 * Description: USER related data processing functions
 *
 * Created on Feb 21, 2014
 *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"

static BOOLEAN _is_idle_alert_allowed()
{	
	if (OTA_if_enabled()) {
		return FALSE;
	}
	
  if (cling.sleep.state==SLP_STAT_LIGHT || cling.sleep.state==SLP_STAT_SOUND) {
		return FALSE;
  }
	
	if (!TOUCH_is_skin_touched()) {
		return FALSE;
	}
	
	return TRUE;
}

void USER_data_init()
{
	USER_DATA *u = &cling.user_data;

	memset(&u->profile, 0, sizeof(USER_PROFILE_CTX));

	// Initialize stride length and weight for distance calculation
	u->profile.stride_in_cm = 75; // In center meters
	u->profile.weight_in_kg = 70; // in KG
	u->profile.height_in_cm = 170; // in center meters

	// default distance
	u->profile.metric_distance = FALSE;

	// user dynamic data initialization
	u->calories_factor = 36; // Calorie factor normalized by 1000

	// User pedometer state filtering thresholds
	u->ppg_day_interval = 900000; // 15 minutes
	u->ppg_night_interval = 1800000; // 30 minutes
	u->skin_temp_day_interval = 600000; // 10 minutes
	u->skin_temp_night_interval = 600000; // 10 minutes
#ifdef _CLING_PC_SIMULATION_
	// gesture recognition
	u->b_screen_wrist_flip = FALSE;
	u->b_screen_press_hold_1 = FALSE;
	u->b_screen_press_hold_3 = FALSE;
	u->b_screen_tapping = FALSE;

	u->b_navigation_tapping = FALSE;
	u->b_navigation_wrist_shake = FALSE;
#else
	// gesture recognition
	u->b_screen_wrist_flip = FALSE;
	u->b_screen_press_hold_1 = TRUE;
	u->b_screen_press_hold_3 = FALSE;
	u->b_screen_tapping = FALSE;

	u->b_navigation_tapping = FALSE;
	u->b_navigation_wrist_shake = FALSE;
#endif
	// Reset alert time
	u->idle_time_in_minutes = 0;
	u->idle_time_start = 0;
	u->idle_time_end = 0;
	u->idle_state = IDLE_ALERT_STATE_IDLE;
	
	// Screen ON time
	u->screen_on_general = 5;
	u->screen_on_heart_rate = 25;
	
	// Sleep detection sensitivity mode
//cling.sleep.m_sensitive_mode = SLEEP_SENSITIVE_HIGH;
	cling.sleep.m_sensitive_mode = SLEEP_SENSITIVE_LOW;
	
	// Reminder off during weekends
	u->b_reminder_off_weekends = FALSE;
}

void USER_store_device_param(I8U *data)
{
	I8U *pdata = data+1; // Leave the first byte for the length
	I8U setting_length = 0;
	USER_DATA *u = &cling.user_data;

	*pdata++ = (u->ppg_day_interval>>24)&0xff; // day interval
	*pdata++ = (u->ppg_day_interval>>16)&0xff; // day interval
	*pdata++ = (u->ppg_day_interval>>8)&0xff; // day interval
	*pdata++ = (u->ppg_day_interval&0xff); // day interval
	setting_length += 4;
	
	*pdata++ = (u->ppg_night_interval>>24)&0xff; // night interval
	*pdata++ = (u->ppg_night_interval>>16)&0xff; // night interval
	*pdata++ = (u->ppg_night_interval>>8)&0xff; // night interval
	*pdata++ = (u->ppg_night_interval&0xff); // night interval
	setting_length += 4;

	*pdata++ = (u->skin_temp_day_interval>>24)&0xff; // skin temp day interval
	*pdata++ = (u->skin_temp_day_interval>>16)&0xff; // skin temp day interval
	*pdata++ = (u->skin_temp_day_interval>>8)&0xff; // skin temp day interval
	*pdata++ = (u->skin_temp_day_interval&0xff); // skin temp day interval
	setting_length += 4;
	
	*pdata++ = (u->skin_temp_night_interval>>24)&0xff; // skin temp night interval
	*pdata++ = (u->skin_temp_night_interval>>16)&0xff; // skin temp night interval
	*pdata++ = (u->skin_temp_night_interval>>8)&0xff; // skin temp night interval
	*pdata++ = (u->skin_temp_night_interval&0xff); // skin temp night interval
	setting_length += 4;

	// gesture recognition
	*pdata++ = u->b_screen_wrist_flip;
	*pdata++ = u->b_screen_press_hold_1;
	*pdata++ = u->b_screen_press_hold_3;
	*pdata++ = u->b_screen_tapping;
	setting_length += 4;
	
	*pdata++ = u->b_navigation_tapping;
	*pdata++ = u->b_navigation_wrist_shake;
	setting_length += 2;

	*pdata++ = u->idle_time_in_minutes;
	*pdata++ = u->idle_time_start; // Idle alert - start time
	*pdata++ = u->idle_time_end; // Idle alert - end time
	setting_length += 3;
	
	*pdata++ = u->screen_on_general;
	*pdata++ = u->screen_on_heart_rate;
	setting_length += 2;
	
	*pdata++ = cling.sleep.m_sensitive_mode;
	setting_length ++;
	
	*pdata++ = u->b_reminder_off_weekends;
	setting_length ++;
	
	// Finally, we put in the length
	data[0] = setting_length;
	
	N_SPRINTF("[USER] critical store device param: %d", setting_length);
}

void USER_setup_profile(I8U *data)
{
	USER_DATA *u = &cling.user_data;
	I16U height_in_cm, weight_in_kg, stride_in_cm;
	I8U *pdata = data;

	height_in_cm = *pdata++; // height
	height_in_cm |= (*pdata++)<<8; // 
	
	weight_in_kg = *pdata++; // weight
	weight_in_kg |= (*pdata++)<<8; // 
	
	stride_in_cm = *pdata++; // stride
	stride_in_cm |= (*pdata++)<<8; // 
	
	N_SPRINTF("[USER] %d, %d, %d", height_in_cm, weight_in_kg, stride_in_cm);
	
	// Initialize stride length and weight for distance calculation
	u->profile.stride_in_cm = stride_in_cm; // In center meters
	u->profile.weight_in_kg = weight_in_kg; // in KG
	u->profile.height_in_cm = height_in_cm; // in center meters

}

void USER_setup_device(I8U *data, I8U setting_length)
{
	USER_DATA *u = &cling.user_data;
	I8U *pdata = data;
	I32U value;

	// Setting length
	//
	// Type: 1 B
	// PPG day interval: 4 B
	// PPG night interval: 4 B
	// SKIN TEMP day interval: 4 B
	// SKIN TEMP night interval: 4 B
	// Screen activation: 4 B
	// Navigation: 2 B
	//
	// New added:
	//
	// Idle alert: 1 B
	
	value = *pdata++; // day interval
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	// 5 minutes is the minimum
#if 0 // Not used
	if (value < 300000) {
		Y_SPRINTF("[USER] illegal setting(day-hr): %d", value);
		//return;
	}
	u->ppg_day_interval = value;
#endif
	setting_length -= 4;

	value = *pdata++; // night interval
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	// 5 minutes is the minimum
#if 0 // not used
	if (value < 300000) {
		Y_SPRINTF("[USER] illegal setting(night-hr): %d", value);
		//return;
	}
	u->ppg_night_interval = value;
#endif
	setting_length -= 4;

	value = *pdata++; // night interval
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	// 5 minutes is the minimum
#if 0 // Not used
	if (value < 300000) {
		Y_SPRINTF("[USER] illegal setting(day-temp): %d", value);

		//return;
	}
	u->skin_temp_day_interval = value;
#endif
	setting_length -= 4;

	value = *pdata++; // night interval
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	value <<= 8; // 
	value |= *pdata++; // 
	// 5 minutes is the minimum
#if 0 // Not used
	if (value < 300000) {
				Y_SPRINTF("[USER] illegal setting(night-temp): %d", value);

		//return;
	}
	u->skin_temp_night_interval = value;
	u->skin_temp_day_interval = 5000;
#endif
	setting_length -= 4;
	// gesture recognition
	u->b_screen_wrist_flip = *pdata++;
	u->b_screen_press_hold_1 = *pdata++;
	u->b_screen_press_hold_3 = *pdata++;
	u->b_screen_tapping = *pdata++;
	u->b_screen_tapping = FALSE; // TURN off screen tapping by default
	setting_length -= 4;
	
	u->b_navigation_tapping = *pdata++;
	u->b_navigation_wrist_shake = *pdata++;
	
	// No longer support navigation setting
	u->b_navigation_tapping = FALSE;
	u->b_navigation_wrist_shake = FALSE;
	setting_length -= 2;

	Y_SPRINTF("\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
		u->ppg_day_interval,
		u->ppg_night_interval,
		u->skin_temp_day_interval,
		u->skin_temp_night_interval,
		u->b_screen_wrist_flip,
		u->b_screen_press_hold_1,
		u->b_screen_press_hold_3,
		u->b_screen_tapping,
		u->b_navigation_tapping,
		u->b_navigation_wrist_shake);
		
	if (setting_length >= 3) {
		u->idle_time_in_minutes = *pdata++;
		u->idle_time_start = *pdata++; // Idle alert - start time
		u->idle_time_end = *pdata++; // Idle alert - end time
		
		u->idle_state = IDLE_ALERT_STATE_IDLE;

		setting_length -= 3;
		Y_SPRINTF("\n\n idle alert: %d, %d, %d", u->idle_time_in_minutes, u->idle_time_start, u->idle_time_end);
	} else {
		return;
	}
	
	if (setting_length >= 2) {
		u->screen_on_general = *pdata++;
		u->screen_on_heart_rate = *pdata++;
		
		setting_length -= 2;
		N_SPRINTF("\n screen on: %d,%d\n", u->screen_on_general, u->screen_on_heart_rate);
	} else {
		return;
	}
	
	if (setting_length >= 1) {
		cling.sleep.m_sensitive_mode = (SLEEP_SENSITIVE_MODE)(*pdata++);
		setting_length --;
		N_SPRINTF("\n sleep sensitivity level: %d", cling.sleep.m_sensitive_mode);
	}
	
	if (setting_length >= 1) {
		u->b_reminder_off_weekends = *pdata++;
		setting_length --;
		N_SPRINTF("\n reminder off: %d\n", u->b_reminder_off_weekends);
	}
}

void USER_state_machine()
{
	USER_DATA *u = &cling.user_data;
	
	switch (u->idle_state) {
		case IDLE_ALERT_STATE_IDLE:
			if (u->idle_time_in_minutes > 0) {
				if (cling.time.local.hour>=cling.user_data.idle_time_start) {
					if (cling.time.local.hour < cling.user_data.idle_time_end) {
						u->idle_minutes_countdown = u->idle_time_in_minutes;
						u->idle_step_countdown = 64;
						u->idle_state = IDLE_ALERT_STATE_COUNT_DOWN;
						Y_SPRINTF("[USER] reset idle alert: %d, %d", u->idle_minutes_countdown, u->idle_step_countdown);
					}
				}
			}
			break;

		case IDLE_ALERT_STATE_COUNT_DOWN:
			if (u->idle_minutes_countdown ==0) {
				if (u->idle_time_in_minutes == 0) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else if (cling.time.local.hour < cling.user_data.idle_time_start) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else if (cling.time.local.hour >= cling.user_data.idle_time_end) {
					u->idle_state = IDLE_ALERT_STATE_IDLE;
				} else {
					if (_is_idle_alert_allowed()) {
						u->idle_state = IDLE_ALERT_STATE_NOTIFY;
						Y_SPRINTF("[USER] start idle alert");
					}
				}
	    }
			break;

		case IDLE_ALERT_STATE_NOTIFY:
			NOTIFIC_start_idle_alert();
			u->idle_state = IDLE_ALERT_STATE_IDLE;
			break;
			
		default:
			u->idle_state = IDLE_ALERT_STATE_IDLE;
			break;
  }
}

