/***************************************************************************//**
 * File: ui.c
 * 
 * Description: master UI controller
 *
 ******************************************************************************/

#include "main.h"
#include "assets.h"

#ifdef _CLINGBAND_NFC_MODEL_
#include "ui_matrix_nfc.h"
#else
#include "ui_matrix.h"
#endif

#define RENDERING_MAX_FRAMES     11

void UI_init()
{
	// Turn on OLED panel
	OLED_set_panel_on();

	// UI initial state
	UI_switch_state(UI_STATE_CLING_START, 2000);
	
	// UI prefer state
	cling.ui.prefer_state = UI_STATE_HOME;
	cling.ui.true_display = TRUE;
}

static void _set_animation(I8U mode, I8U dir)
{
	UI_ANIMATION_CTX *u = &cling.ui;

	UI_switch_state(UI_STATE_ANIMATING, 0);

	u->animation_index = 0;
	u->direction = dir;
	u->frame_prev_idx = u->frame_index;
	u->animation_mode = mode;
}

static I8U _get_max_detail_depth()
{
	char data[128];
	I8U  acc_data_pos, curr_line_pos, curr_pixel_pos;
	
	NOTIFIC_get_app_message_detail(cling.ui.level_1_index, data);

	if (data[0] == 0)
		return 0;
	
	// First, we record the line seperates
	curr_line_pos = 0;
	curr_pixel_pos = 0;
	acc_data_pos = 0;
	while(acc_data_pos < 128)
	{
		// ASCII code encode format.
		if((data[acc_data_pos] >= 0x20)&&(data[acc_data_pos]<=0x7e)){
		
			if (curr_pixel_pos >= 112) {
				curr_line_pos++;
				curr_pixel_pos = 0;
			}
      // One ascii characters occupy 1 Byte.			
			acc_data_pos += 1;
			curr_pixel_pos += 8;
		}
		// Chinese characters Utf-8 encode format.
		else if(((data[acc_data_pos]&0xF0) == 0xE0)&&((data[acc_data_pos+1]&0xC0) == 0x80)&&((data[acc_data_pos+2]&0xC0) == 0x80)){

			if (curr_pixel_pos >= 104) {
				curr_line_pos++;
				curr_pixel_pos = 0;
			}
			// One UTF-8 chinese characters occupy 3 Bytes.
			acc_data_pos += 3;
			curr_pixel_pos += 16;
		}
		else
			// Is not within the scope of the can display,continue to read the next.
			acc_data_pos++;
	}
	if (curr_pixel_pos > 0)
		curr_line_pos ++;
	
	Y_SPRINTF("[UI] depth: %d, %d, %d, %d", curr_line_pos, (curr_line_pos+1)>>1, len, acc_data_pos);
	
	return ((curr_line_pos+1)>>1);
}

static void _update_horizontal_level_1_index(UI_ANIMATION_CTX *u, BOOLEAN b_up)
{
	I8U index = u->frame_prev_idx;
	I8U max_frame_num=0;
	
	if (index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		return;
	} else if (index == UI_DISPLAY_SMART_APP_NOTIF) {
#ifdef _ENABLE_ANCS_				
		max_frame_num = NOTIFIC_get_message_total();		
#endif		
		if (max_frame_num == 0) {
			u->level_1_index = 0;
			return;
		}
		
		// Update vertical index
		if (b_up) {
			u->level_1_index++;
			
			if (u->level_1_index >= max_frame_num) {
				u->level_1_index = 0;
			}
			
		} else {
			if (u->level_1_index == 0) {
				u->level_1_index = max_frame_num - 1;
			} else {
				u->level_1_index --;
			}
		}
	} else {
		u->level_1_index = 0;
	}
}

static void _update_level_1_index(UI_ANIMATION_CTX *u, BOOLEAN b_up)
{
	I8U index = u->frame_prev_idx;
	I8U max_frame_num;
	
	if (index == UI_DISPLAY_HOME) {
		max_frame_num = FRAME_DEPTH_CLOCK;
	} else if (index == UI_DISPLAY_TRACKER_STEP) {
		max_frame_num = FRAME_DEPTH_STEP;
	} else if (index == UI_DISPLAY_TRACKER_SLEEP) {
		max_frame_num = FRAME_DEPTH_SLEEP; 
	} else if (index == UI_DISPLAY_TRACKER_DISTANCE) {
		max_frame_num = FRAME_DEPTH_DISTANCE;
	} else if (index == UI_DISPLAY_TRACKER_CALORIES) {
		max_frame_num = FRAME_DEPTH_CALORIES;
	} else if (index == UI_DISPLAY_SMART_REMINDER) {
		if (cling.reminder.ui_alarm_on)
			return;
		max_frame_num = cling.reminder.total;
		N_SPRINTF("[UI] max reminder num: %d", max_frame_num);
	} else if (index == UI_DISPLAY_SMART_WEATHER) {
		max_frame_num = FRAME_DEPTH_WEATHER;
	} else if (index == UI_DISPLAY_SMART_INCOMING_MESSAGE) {
		return;
	} else if (index == UI_DISPLAY_SMART_APP_NOTIF) {
		return;
	} else if (index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		return;
	} else {
		max_frame_num = 0;
	}
	
	if (max_frame_num == 0) {
		u->level_1_index = 0;
		return;
	}
	
	// Update vertical index
	if (b_up) {
		u->level_1_index++;
		
		if (u->level_1_index >= max_frame_num) {
			u->level_1_index = 0;
		}
		
	} else {
		if (u->level_1_index == 0) {
			u->level_1_index = max_frame_num - 1;
		} else {
			u->level_1_index --;
		}
	}
	
	// Read out reminder index from Flash
	if (index == UI_DISPLAY_SMART_REMINDER) {
		cling.ui.level_1_index = REMINDER_get_time_at_index(cling.ui.level_1_index);
		Y_SPRINTF("[UI] new vertical index(Reminder): %d", cling.ui.level_1_index);
	}
	
	if (index == UI_DISPLAY_HOME) {
		cling.ui.clock_orientation ++;
		if (cling.ui.clock_orientation >= max_frame_num) {
			cling.ui.clock_orientation = 0;
		}
	}
	
	N_SPRINTF("[UI] new vertical index: %d", cling.ui.level_1_index);
}

static void _operation_mode_switch(I8U gesture)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U index = u->frame_prev_idx;

	if (gesture == TOUCH_FINGER_RIGHT){
		
		if (index == UI_DISPLAY_SMART_INCOMING_CALL) {
			
			// Stop vibration in incoming call state
			cling.notific.state = NOTIFIC_STATE_IDLE;
			GPIO_vibrator_set(FALSE);
			return;
		}
		
		if (index == UI_DISPLAY_STOPWATCH_START) {
			cling.activity.workout_time_stamp_start = CLK_get_system_time();
			cling.activity.workout_time_stamp_stop = cling.activity.workout_time_stamp_start;
			cling.activity.workout_Calories_start = cling.activity.day.calories;
			cling.activity.workout_Calories_acc = 0;
			cling.activity.b_workout_active = TRUE;
			return;
		}
		
		if (index == UI_DISPLAY_STOPWATCH_STOP) {
			cling.activity.workout_time_stamp_stop = CLK_get_system_time();
			cling.activity.workout_type = WORKOUT_NONE;
			cling.activity.workout_place = WORKOUT_PLACE_NONE;
			cling.activity.b_workout_active = FALSE;
			return;
		}
	}
	
	if (gesture == TOUCH_FINGER_RIGHT) {
		
		if (index == UI_DISPLAY_WORKOUT_INDOOR) {
			cling.activity.workout_place = WORKOUT_PLACE_INDOOR;
			return;
		}
		
		if (index == UI_DISPLAY_WORKOUT_OUTDOOR) {
			cling.activity.workout_place = WORKOUT_PLACE_OUTDOOR;
			return;
		}
		
		if (index == UI_DISPLAY_WORKOUT_WALKING) {
			cling.activity.workout_type = WORKOUT_WALK;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_RUNNING) {
			cling.activity.workout_type = WORKOUT_RUN;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_ROW) {
			cling.activity.workout_type = WORKOUT_ROWING;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_ELLIPTICAL) {
			cling.activity.workout_type = WORKOUT_ELLIPTICAL;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_STAIRS) {
			cling.activity.workout_type = WORKOUT_STAIRS;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_CYCLING) {
			cling.activity.workout_type = WORKOUT_CYCLING;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_PILOXING) {
			cling.activity.workout_type = WORKOUT_PILOXING;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_AEROBIC) {
			cling.activity.workout_type = WORKOUT_AEROBIC;
			return;
		}
		if (index == UI_DISPLAY_WORKOUT_OTHERS) {
			cling.activity.workout_type = WORKOUT_OTHER;
			return;
		}
	}
}

static BOOLEAN _ui_vertical_animation(I8U index)
{	
	if ((index >= UI_DISPLAY_TRACKER_STEP) && (index <= UI_DISPLAY_TRACKER_CALORIES)) {
		return TRUE;
	}
	
	if  (index == UI_DISPLAY_SMART_REMINDER) {
		return TRUE;
	} else if (index == UI_DISPLAY_SMART_WEATHER) {
		return TRUE;
	//} else if (index == UI_DISPLAY_SMART_APP_NOTIF) {
		//return TRUE;
	} else if (index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		return TRUE;
	} else {
		return FALSE;
	}
}


static void _update_level_2_index(UI_ANIMATION_CTX *u, BOOLEAN b_left)
{	
	I8U max_frame_num;

	if (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF) {
		max_frame_num = _get_max_detail_depth();

		Y_SPRINTF("LEVEL 2: %d, %d", max_frame_num, u->level_2_index);
		
		// Update horizontal index
		if (b_left) {
			u->level_2_index++;
			
			if (u->level_2_index >= max_frame_num) {
				u->level_2_index = 0;
			}
			
		} else {
			if (u->level_2_index == 0) {
				u->level_2_index = max_frame_num - 1;
			} else {
				u->level_2_index --;
			}
		}
		
	}
}

static void _perform_ui_with_a_swipe(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;

	// Erase "more" icon
	u->b_detail_page = FALSE;

	// Make sure the UI frame can take swipe gesture
	if (ui_gesture_constrain[u->frame_index] & UFG_SWIPE_PANNING) {
		if (gesture == TOUCH_SWIPE_LEFT) {
			p_matrix = ui_matrix_swipe_left;
			if (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF)
				_set_animation(ANIMATION_PANNING_IN, TRANSITION_DIR_LEFT);
			else
				_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_LEFT);
					
			// Update frame index
			u->frame_index = p_matrix[u->frame_index];
			u->frame_next_idx = u->frame_index;
			
			// Get correct level index - level 1
			_update_horizontal_level_1_index(u, TRUE);
			
			// Get correct level index - level 2
//			_update_level_2_index(u, TRUE);
			u->level_2_index = 0;
			Y_SPRINTF("[UI] swipe left: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);
		} else  {
			p_matrix = ui_matrix_swipe_right;
			if (u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF)
				_set_animation(ANIMATION_PANNING_IN, TRANSITION_DIR_RIGHT);
			else
				_set_animation(ANIMATION_PANNING_OUT, TRANSITION_DIR_RIGHT);
								
			// Update frame index
			u->frame_index = p_matrix[u->frame_index];
			u->frame_next_idx = u->frame_index;
			
			// Get correct level index - level 1
			_update_horizontal_level_1_index(u, FALSE);
			
			// Get correct level index - level 2
			//_update_level_2_index(u, FALSE);
			u->level_2_index = 0;
			
			Y_SPRINTF("[UI] swipe right: %d, %d, %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);
		}
	} else {
		// Animation is not needed
		u->frame_next_idx = u->frame_index;
		
		// No further process
		return;
	}
}

static void _perform_ui_with_button_click(UI_ANIMATION_CTX *u)
{
	const I8U *p_matrix = NULL;
	
	u->b_detail_page = FALSE;
	
	if (ui_gesture_constrain[u->frame_index] & UFG_BUTTON_SINGLE) {
		_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
		p_matrix = ui_matrix_button;
		
		// Update frame index
		if (!cling.activity.b_workout_active)
			cling.activity.workout_type = WORKOUT_NONE;
		
		if (cling.activity.workout_type > WORKOUT_NONE) {
			u->frame_index = UI_DISPLAY_STOPWATCH_STOP;
		} else {
			u->frame_index = p_matrix[u->frame_index];
		}
		
		// Update next index as well
		u->frame_next_idx = u->frame_index;
		// Reset level - 1 index
		u->level_1_index = 0; 
		// Reset level - 2 index
		u->level_2_index = 0; 
		Y_SPRINTF("[UI] button single: %d", u->frame_index);
	}
}

static void _perform_ui_with_button_press_hold(UI_ANIMATION_CTX *u)
{
	// Erase "more" icon
	u->b_detail_page = FALSE;

	if (ui_gesture_constrain[u->frame_index] & UFG_BUTTON_HOLD) {
		_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
		u->frame_index = UI_DISPLAY_CAROUSEL_1;
		u->frame_next_idx = UI_DISPLAY_CAROUSEL_1;
		// Reset level - 1 index
		u->level_1_index = 0; 
		// Reset level - 2 index
		u->level_2_index = 0; 
		Y_SPRINTF("[UI] button press and hold: %d", u->frame_index);
	}
}

static void _perform_ui_with_a_finger_touch(UI_ANIMATION_CTX *u, I8U gesture)
{
	const I8U *p_matrix = NULL;
	I8U prev_index;
	
	// Make sure current UI frame supports finger touch
	if (!(ui_gesture_constrain[u->frame_index] & UFG_FINGER_IRIS)) {
		return;
	}
		
	// Perform UI update with finger down on left/right/middle

	if (gesture == TOUCH_FINGER_MIDDLE) {
		u->b_detail_page = TRUE;
					
		Y_SPRINTF("[UI] finger middle: %d, %d", u->frame_index, u->b_detail_page);

		p_matrix = ui_matrix_finger_middle;
		
		_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);

		// Update frame index
		u->frame_index = p_matrix[u->frame_index];
		u->frame_next_idx = u->frame_index;
	} else if (gesture == TOUCH_FINGER_LEFT) {
					
		u->b_detail_page = TRUE;
		
		Y_SPRINTF("[UI] finger left: %d, %d", u->frame_index, u->b_detail_page);

		p_matrix = ui_matrix_finger_left;
	
		_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
		// Update frame index
		prev_index = u->frame_index;
		u->frame_index = p_matrix[u->frame_index];
		u->frame_next_idx = u->frame_index;
		
		// If landing page changes to clock, do not show detail page
		if (u->frame_index == UI_DISPLAY_HOME) {
			if (prev_index != UI_DISPLAY_HOME) {
				u->b_detail_page = FALSE;
				
				Y_SPRINTF("[UI] Landing to clock");
			}
		}
	} else if (gesture == TOUCH_FINGER_RIGHT) {
	

		p_matrix = ui_matrix_finger_right;
		
		if (u->b_detail_page) {
				if (_ui_vertical_animation(u->frame_index)) {
					_set_animation(ANIMATION_TILTING_OUT, TRANSITION_DIR_UP);
				} else {
					_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
				}
				p_matrix = ui_matrix_finger_right;
				_update_level_1_index(u, TRUE);
				
				_update_level_2_index(u, TRUE);
		} else {
			u->b_detail_page = TRUE;
			
			_set_animation(ANIMATION_IRIS, TRANSITION_DIR_NONE);
			
			if ((u->frame_index == UI_DISPLAY_SMART_MESSAGE) ||
					(u->frame_index == UI_DISPLAY_SMART_APP_NOTIF) ||
					(u->frame_index == UI_DISPLAY_SMART_DETAIL_NOTIF))
			{
				return;
			}
		}
		
		Y_SPRINTF("[UI] finger right: %d, %d", u->frame_index, u->b_detail_page);

		// Update frame index
		if (u->frame_index == UI_DISPLAY_SMART_INCOMING_CALL) {
			if (!cling.activity.b_workout_active)
				cling.activity.workout_type = WORKOUT_NONE;
			
			if (cling.activity.workout_type > WORKOUT_NONE) {
				u->frame_index = UI_DISPLAY_STOPWATCH_STOP;
			} else {
				u->frame_index = p_matrix[u->frame_index];
			}
		} else {
			u->frame_index = p_matrix[u->frame_index];
		}
		u->frame_next_idx = u->frame_index;
	}
}

static I8U _ui_touch_sensing()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U gesture;
	
	gesture = TOUCH_get_gesture_panel();
	
	// UI state switching based on valid gesture from Cypress IC
	switch (gesture) {
		case TOUCH_SWIPE_LEFT:
		{
			_perform_ui_with_a_swipe(u, gesture);
			break;
		}
		case TOUCH_SWIPE_RIGHT:
		{
			_perform_ui_with_a_swipe(u, gesture);
			break;
		}
		case TOUCH_BUTTON_PRESS_HOLD:
		{
			_perform_ui_with_button_press_hold(u);
			break;
		}
		case TOUCH_BUTTON_SINGLE:
		{
			_perform_ui_with_button_click(u);
			break;
		}
		case TOUCH_FINGER_MIDDLE:
		{
			_perform_ui_with_a_finger_touch(u, TOUCH_FINGER_MIDDLE);
			break;
		}
		case TOUCH_FINGER_LEFT:
		{
			_perform_ui_with_a_finger_touch(u, TOUCH_FINGER_LEFT);
			break;
		}
		case TOUCH_FINGER_RIGHT:
		{
			_perform_ui_with_a_finger_touch(u, TOUCH_FINGER_RIGHT);
			break;
		}
		case TOUCH_DOUBLE_TAP:
		{
			UI_switch_state(UI_STATE_APPEAR, 0);
			Y_SPRINTF("[UI] double tapped: %d", u->frame_index);
			u->b_detail_page = FALSE;
			break;
		}
		case TOUCH_BUTTON_PRESS_SOS:
		{
			u->state = UI_STATE_SOS;
			u->state_init = TRUE;
			u->b_detail_page = FALSE;
			break;
		}
		default:
			break;
	}
	
	if ((gesture >= TOUCH_SWIPE_LEFT) && (gesture <= TOUCH_BUTTON_SINGLE))
	{
		if (cling.reminder.state != REMINDER_STATE_IDLE) {
			Y_SPRINTF("[UI] check for next reminder upon touch");
			cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;
		}
	}

	// Operation mode update
	_operation_mode_switch(gesture);
	
	N_SPRINTF("[UI] curr: %d, index: %d, next: %d", u->frame_prev_idx, u->frame_index, u->frame_next_idx);

	return gesture;
}

static void _render_one_icon(I8U len, I8U *p_out, const I8U *p_in)
{
	I8U j;
	I8U *p_out_0 = p_out;
	I8U *p_out_1 = p_out_0+128;
	
	// Render the left side
	for (j = 0; j < len; j++) {
			*p_out_0++ = (*p_in++);
			*p_out_1++ = (*p_in++);
	}
}

static __INLINE void _rotate_8_bytes_core(I8U *in_data, I8U *out_data)
{
	I8U i, j;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i ++) {
			*(out_data+i) |= (((*in_data)<<i) & 0x80)>>(7-j);
		}
		
		in_data ++;
	}
}
static void _rotate_8_bytes_opposite_core(I8U *in_data, I8U *out_data)
{
	I8U i, j;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i ++) {
			*(out_data-i) |= (((*in_data)<<i) & 0x80)>>j;
		}
		
		in_data ++;
	}
}

static void _rotate_90_degree(I8U *in, I8U *out)
{
	I8U *in_data;
	I8U *out_data;
	
	// first 8 bytes
	out_data = out;
	in_data = in;
	_rotate_8_bytes_core(in_data, out_data);
	
	// 2nd 8 bytes
	out_data = out + 128;
	in_data = in+8;
	_rotate_8_bytes_core(in_data, out_data);

	// 3rd 8 bytes
	out_data = out + 256;
	in_data = in + 16;
	_rotate_8_bytes_core(in_data, out_data);

	// 4th 8 bytes
	out_data = out + 384;
	in_data = in + 24;
	_rotate_8_bytes_core(in_data, out_data);
}

static void _rotate_270_degree(I8U *in, I8U *out)
{
	I8U *in_data;
	I8U *out_data;
	
	// first 8 bytes
	out_data = out+7;
	in_data = in;
	_rotate_8_bytes_opposite_core(in_data, out_data);
	
	// 2nd 8 bytes
	out_data = out +7 - 128;
	in_data = in+8;
	_rotate_8_bytes_opposite_core(in_data, out_data);

	// 3rd 8 bytes
	out_data = out + 7 - 256;
	in_data = in + 16;
	_rotate_8_bytes_opposite_core(in_data, out_data);

	// 4th 8 bytes
	out_data = out + 7 - 384;
	in_data = in + 24;
	_rotate_8_bytes_opposite_core(in_data, out_data);
}

static void _render_vertical_time(I8U *string, I8U offset, BOOLEAN b_180_rotation)
{
	I8U *p0, *p1, *p2;
	I8U char_len, i, j;
	I8U buf1[32];
	I8U buf2[32];
	I8U buf3[32];
	const I8U *pin;
	
	// Render the hour
	memset(buf1, 0, 32);
	memset(buf2, 0, 32);
	memset(buf3, 0, 32);
	p0 = buf1;
	p1 = buf2;
	p2 = buf3;
	for (i = 0; i < 2; i++) {

		// Digits in large fonts
		pin = asset_content+asset_pos[512+string[i]];
		char_len = asset_len[512+string[i]];
		for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
		}
		p0 += 3;
		p1 += 3;
		p2 += 3;
	}
	// do the rotation
	if (b_180_rotation) {
		_rotate_270_degree(buf1, cling.ui.p_oled_up+384+offset);
		_rotate_270_degree(buf2, cling.ui.p_oled_up+384+offset+8);
		_rotate_270_degree(buf3, cling.ui.p_oled_up+384+offset+16);
	} else {		
		_rotate_90_degree(buf1, cling.ui.p_oled_up+offset+16);
		_rotate_90_degree(buf2, cling.ui.p_oled_up+offset+8);
		_rotate_90_degree(buf3, cling.ui.p_oled_up+offset);
	}
}

static void _fill_vertical_local_clock(BOOLEAN b_180_rotation)
{
	I8U string[8];
	I8U char_len, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	
	// Render the clock sign
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		sprintf((char *)string, ":");
		
		p0 = cling.ui.p_oled_up+61;
		p1 = p0+128;
		p2 = p1+128;

		// Digits in large fonts
		pin = asset_content+asset_pos[512+':'];
		char_len = asset_len[512+':'];
		for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
		}
	} else {
		cling.ui.clock_sec_blinking = TRUE;
	}
	
	// Render the hour
	sprintf((char *)string, "%02d", cling.time.local.hour);
	if (b_180_rotation)
		_render_vertical_time(string, 32, b_180_rotation);
	else
		_render_vertical_time(string, 72, b_180_rotation);
	
	// Render the minute
	sprintf((char *)string, "%02d", cling.time.local.minute);
	if (b_180_rotation)
		_render_vertical_time(string, 72, b_180_rotation);
	else
		_render_vertical_time(string, 32, b_180_rotation);

}

static I8U _fill_local_clock(char *string)
{
	I8U len;
		
	if (cling.ui.clock_sec_blinking) {
		cling.ui.clock_sec_blinking = FALSE;
		len = sprintf((char *)string, "%02d:%02d", cling.time.local.hour, cling.time.local.minute);
	} else {
		cling.ui.clock_sec_blinking = TRUE;
		len = sprintf((char *)string, "%02d %02d", cling.time.local.hour, cling.time.local.minute);
	}
	
	return len;
}


static I16U _render_middle_section_large(I8U len, I8U *string, I8U margin)
{
	I16U i, j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
	I16U offset=0;
		
	for (i = 0; i < len; i++) {
		// Add extra margin for character '1'
		if (string[i] == '1') {
			offset += 2;
		}
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		p2 = p1+128;
		if (string[i] == 32) {
			char_len = 6;
		} else {
				// Digits in large fonts
				pin = asset_content+asset_pos[512+string[i]];
				char_len = asset_len[512+string[i]];
				for (j = 0; j < char_len; j++) {
						*p0++ = (*pin++);
						*p1++ = (*pin++);
						*p2++ = (*pin++);
				}
		}
		if (i != (len-1))
			offset += char_len + margin;
		else
			offset += char_len;
		
		// Add extra margin for character '1'
		if (string[i] == '1') {
			offset += 3;
		}
	}

	return offset;
}

static void _render_icon_more(I16U offset)
{
	I16U j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	I8U char_len;
	
	p0 = cling.ui.p_oled_up+offset+128;
	p1 = p0+128;
	p2 = p1+128;

	// "More" icon
	pin = asset_content+ICON_TOP_MORE;
	char_len = ICON_TOP_MORE_LEN;
	for (j = 0; j < char_len; j++) {
			*p2++ = (*pin++);
	}
}

static void _display_dynamic(I8U *pIn, I8U len2, I8U *string2)
{
	I8U *p, *pin;
	I8U *p6;
	I8U i, j, ptr, offset2, char_len;

	// indicator
	p = pIn;
	offset2 = 0;
	for (i = 0; i < len2; i++) {
		pin = (I8U *)(asset_content+asset_pos[string2[i]]);
		char_len = asset_len[string2[i]];
		for (j = 0; j < char_len; j++) {
				*p++ = (*pin++);
		}
	
		offset2 += char_len;
		if (i < len2) {
			offset2 += 3;
		}
		
		p += 5;
		
	}

	// Center it in the middle
	ptr = (128 - offset2)>>1;
	p = pIn;
	if (ptr > 0) {
		p += 127; p6 = p - ptr;
		for (i = 0; i < 128-ptr; i++) {
			*p-- = *p6--;
		}
		for (; i < 128; i++) {
			*p-- = 0;
		}
	}
}

static void _display_detail_2_row(I8U *data, I8U vertical_idx)
{
//	I8U len;  //The length of the string
	I8U acc_data_pos = 0;
	I8U line_pos[12];
	I8U curr_line_pos;
	I8U curr_pixel_pos;
	I8U string[32];
	I8U start, copy_len;

	if (data[0] == 0) return;

	// First, we record the line seperates
	curr_line_pos = 0;
	curr_pixel_pos = 0;
	memset(line_pos, 0, 10);
	// Maximum string length is 128
	while (acc_data_pos < 128)
	{
		if((data[acc_data_pos] >= 0x20)&&(data[acc_data_pos]<=0x7e))//ascii
		{
			acc_data_pos+=1;//one  ascii characters need 1 bytes
			curr_pixel_pos += 8;
			
			if (curr_pixel_pos >= 112) {
				line_pos[curr_line_pos++] = acc_data_pos;
				curr_pixel_pos = 0;
			}
			continue;
		}
		if((data[acc_data_pos]&0x80)==0x80)//chinese characters  Byte highest=1;	 
		{
			if (curr_pixel_pos >= 104) {
				line_pos[curr_line_pos++] = acc_data_pos;
				curr_pixel_pos = 0;
			}
			acc_data_pos+=3;//one  utf-8  chinese characters need 1 bytes
			curr_pixel_pos += 16;
			continue;
		}
		break;
	}
	
	if (curr_pixel_pos > 0) {
		line_pos[curr_line_pos++] = acc_data_pos;
	}

	curr_line_pos = vertical_idx<<1;
	if (line_pos[curr_line_pos] == 0) {
		curr_line_pos = 0;
	}
	
	if (curr_line_pos == 0) {
		start = 0;
		copy_len = line_pos[curr_line_pos];
	} else {
		start = line_pos[curr_line_pos-1];
		copy_len = line_pos[curr_line_pos] - line_pos[curr_line_pos-1];
	}
	
	// Then, we are going to display
	memset(string, 0, 32);
	memcpy(string, data+start, copy_len);
	FONT_load_characters(cling.ui.p_oled_up, (char *)string, 16, FALSE);
	
	N_SPRINTF("[UI] 2 line: %d, %d, %d", curr_line_pos, line_pos[curr_line_pos], vertical_idx);

	if (line_pos[curr_line_pos+1] == 0) 
		return;
	
	start = line_pos[curr_line_pos];
	copy_len = line_pos[curr_line_pos+1]-line_pos[curr_line_pos];

	memset(string, 0, 32);
	memcpy(string, data+start, copy_len);
	FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, FALSE);
}

static BOOLEAN _middle_row_render(I8U mode, BOOLEAN b_center)
{
	I16U i;
	I16U offset = 0; // Pixel offet at top row
	I8U *p0, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U string[128];
	I8U len=0, ptr, margin;
	WEATHER_CTX weather;
	I32U stat;
	I16U integer, fractional;
	BOOLEAN b_more = FALSE;
	BOOLEAN b_full_screen = FALSE;
	I8U bar_len=0;
	BOOLEAN b_progress_bar = FALSE;

	// Clean up top row
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Set up margin between characters
	margin = 3;
	
	// Render the left side 
	if (mode == UI_MIDDLE_MODE_CLOCK) {
		if (cling.ui.clock_orientation == 1) {
			len = _fill_local_clock((char *)string);
			N_SPRINTF("[UI] flip clock (h): %d", cling.ui.clock_orientation);
		} else if (cling.ui.clock_orientation == 2) {
			N_SPRINTF("[UI] flip clock (v-270): %d", cling.ui.clock_orientation);
			_fill_vertical_local_clock(TRUE);
			len = 0;
			b_center = FALSE;
		} else {
			N_SPRINTF("[UI] flip clock (v-90): %d", cling.ui.clock_orientation);
			_fill_vertical_local_clock(FALSE);
			len = 0;
			b_center = FALSE;
		} 
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_STEPS) {
		TRACKING_get_activity(cling.ui.level_1_index, TRACKING_STEPS, &stat);
		len = sprintf((char *)string, "%d", stat);
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_DISTANCE) {
		TRACKING_get_activity(cling.ui.level_1_index, TRACKING_DISTANCE, &stat);
		integer = stat/1000;
		fractional = stat - integer * 1000;
		fractional /= 10;
		len = sprintf((char *)string, "%d.%02d", integer, fractional);
		
		string[len++]=ICON_MIDDLE_SLEEP_K;
		string[len++] = ICON_MIDDLE_SLEEP_M;
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_CALORIES) {
		TRACKING_get_activity(cling.ui.level_1_index, TRACKING_CALORIES, &stat);
		len = sprintf((char *)string, "%d", stat);
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_INCOMING_CALL) {
		len = NOTIFIC_get_callerID((char *)string);
		string[16] = 0;
		FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, TRUE);
		len = 0;
		b_center = FALSE;
	} else if (mode == UI_MIDDLE_MODE_OTA) {
		b_center = FALSE;
		b_progress_bar = TRUE;
		sprintf((char *)string, "%d %%", cling.ota.percent);
		bar_len = cling.ota.percent;
		FONT_load_characters(cling.ui.p_oled_up+128, (char *)string, 16, TRUE);
		len = 0;
	} else if (mode == UI_MIDDLE_MODE_UV_IDX) {
#ifdef _ENABLE_UV_
  	integer = cling.uv.max_UI_uv;
		len = sprintf((char *)string, "%d.%d", (integer/10), (integer%10));
#endif
	} else if (mode == UI_MIDDLE_MODE_SLEEP) {
		TRACKING_get_activity(cling.ui.level_1_index, TRACKING_SLEEP, &stat);
		// Note: sleep is in seconds, so normalize it to minutes first
		stat /= 60; // Get sleep in minutes, and display in format: HH:MM
		integer = stat/60;
		fractional = stat - integer * 60;
		len = sprintf((char *)string, "%d", integer);
		string[len++]=ICON_MIDDLE_SLEEP_H;
		len += sprintf((char *)(string+len), "%02d", fractional);
		string[len++] = ICON_MIDDLE_SLEEP_M;
		string[len] = 0;
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_MESSAGE) {
		// In case iOS just delete a message
#ifdef _ENABLE_ANCS_				
		len = sprintf((char *)string, "%d", NOTIFIC_get_message_total());
#endif		
		b_more = TRUE;
		N_SPRINTF("[UI] smart message hit +++++");
	} else if (mode == UI_MIDDLE_MODE_INCOMING_MESSAGE) {
		Y_SPRINTF("[UI] Incoming message: %d", cling.ui.level_1_index);

		cling.ui.level_1_index = 0;

		len = NOTIFIC_get_app_name(cling.ui.level_1_index, (char *)string);
		
		if ((len > 14) && cling.ui.b_detail_page) {
			_display_detail_2_row(string, 0);
			b_full_screen = TRUE;
		} else {
			string[16]= 0;
			FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, TRUE);
		}
		
		len = 0;
		b_more = TRUE;
		b_center = FALSE;
	} else if (mode == UI_MIDDLE_MODE_APP_NOTIF) {
		len = NOTIFIC_get_app_name(cling.ui.level_1_index, (char *)string);
		Y_SPRINTF("[UI] app index: %d, %d, %s", cling.ui.level_1_index, len, (char *)string);

		if ((len > 14) && cling.ui.b_detail_page) {
			_display_detail_2_row(string, 0);
			b_full_screen = TRUE;
		} else {
			string[16]= 0;
			FONT_load_characters(cling.ui.p_oled_up+128+128, (char *)string, 16, TRUE);
		}
		
		len = 0;
		b_more = TRUE;
		b_center = FALSE;
	} else if (mode == UI_MIDDLE_MODE_DETAIL_NOTIF) {
		NOTIFIC_get_app_message_detail(cling.ui.level_1_index, (char *)string);
		_display_detail_2_row(string, cling.ui.level_2_index);
		Y_SPRINTF("[UI] message detail: %d %d %s", cling.ui.level_1_index, cling.ui.level_2_index, (char *)string);
		b_more = TRUE;
		len = 0;
		b_center = FALSE;
	} else if (mode == UI_MIDDLE_MODE_PHONE_FINDER) {
		string[0] = ICON32_PHONE_FINDER_1;	
		string[1] = ' ';
		string[2] = ICON32_PHONE_FINDER_2;
		string[3] = ' ';
		string[4] = ICON32_PHONE_FINDER_3;
		len = 5;
	} else if (mode == UI_MIDDLE_MODE_REMINDER) {
		
		if (cling.reminder.total>0) {
			
			if (cling.reminder.ui_alarm_on) {
				len = sprintf((char *)string, "%d:%02d", cling.reminder.ui_hh, cling.reminder.ui_mm);
			} else {
				
				if (cling.reminder.ui_hh == 0xff || cling.reminder.ui_mm == 0xff || cling.reminder.ui_hh >= 24 || cling.reminder.ui_mm >= 60) {
					len = sprintf((char *)string, "0:00");
				} else {
					len = sprintf((char *)string, "%d:%02d", cling.reminder.ui_hh, cling.reminder.ui_mm);
					
					Y_SPRINTF("[UI] ui_hh: %d, ui_mm: %d", cling.reminder.ui_hh, cling.reminder.ui_mm);
				}
			}
		} else {
//			len = sprintf((char *)string, "0:00");
  		  len = 0;
  		  string[len++] = ICON_MIDDLE_NO_SKIN_TOUCH;
  		  string[len] = 0;
		}
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_HEART_RATE) {
		if (TOUCH_is_skin_touched()) {
			N_SPRINTF("[UI] Heart rate - valid");

			len = 0;
			//cling.hr.heart_rate_ready = 0;
			if (cling.hr.heart_rate_ready) {
				len = sprintf((char *)string, "%d", PPG_minute_hr_calibrate());
			} else {
				
				if (cling.ui.heart_rate_wave_index > 5) {
					cling.ui.heart_rate_wave_index = 0;
				}
				switch (cling.ui.heart_rate_wave_index) {
					case 0:
						len = sprintf((char *)string, "-,,,,,");
						break;
					case 1:
						len = sprintf((char *)string, ",-,,,,");
						break;
					case 2:
						len = sprintf((char *)string, ",,-,,,");
						break;
					case 3:
						len = sprintf((char *)string, ",,,-,,");
						break;
					case 4:
						len = sprintf((char *)string, ",,,,-,");
						break;
					case 5:
						len = sprintf((char *)string, ",,,,,-");
						break;
					default:
						break;
				}
				
				_display_dynamic(cling.ui.p_oled_up+128+128, len, string);
				cling.ui.heart_rate_wave_index ++;
				
				return b_full_screen;
			}
		} else {
			N_SPRINTF("[UI] Heart rate - not valid");
			
			len = 0;
			string[len++] = ICON_MIDDLE_NO_SKIN_TOUCH;
			string[len] = 0;
		}
	} else if (mode == UI_MIDDLE_MODE_SKIN_TEMP) {
		if (TOUCH_is_skin_touched()) {
  		integer = cling.therm.current_temperature/10;
			fractional = cling.therm.current_temperature - integer * 10;
			len = sprintf((char *)string, "%d.%d", integer, fractional);
			string[len++] = ICON_MIDDLE_CELCIUS_IDX;
		} else {
			N_SPRINTF("[UI] skin temp - not valid");
			len = 0;
			string[len++] = ICON_MIDDLE_NO_SKIN_TOUCH;
			string[len] = 0;
		}
	} else if (mode == UI_MIDDLE_MODE_WEATHER) {

		if (WEATHER_get_weather(0, &weather)) {
			
			if (cling.ui.level_1_index & 0x01) {
				len = 0;
				string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
				len += sprintf((char *)string+len, "%d", weather.low_temperature);
				string[len++] = ICON_MIDDLE_CELCIUS_IDX;
			} else {
				len = 0;
				string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
				len += sprintf((char *)string+len, "%d", weather.high_temperature);
				string[len++] = ICON_MIDDLE_CELCIUS_IDX;
			}
		} else {
			if (cling.ui.level_1_index & 0x01) {
				len = 0;
				string[len++] = ICON_MIDDLE_LOW_TEMP_IDX;
				len += sprintf((char *)string+len, "15");
				string[len++] = ICON_MIDDLE_CELCIUS_IDX;
			} else {
				len = 0;
				string[len++] = ICON_MIDDLE_HIGH_TEMP_IDX;
				len += sprintf((char *)string+len, "22");
				string[len++] = ICON_MIDDLE_CELCIUS_IDX;
			}
		}
		b_more = TRUE;
	} else if (mode == UI_MIDDLE_MODE_SOS) {

		string[0] = 'S';
		string[1] = 'O';
		string[2] = 'S';
		string[3] = ' ';
		string[4] = ICON32_PHONE_FINDER_1;
		len = 5;
		b_center = TRUE;
	} else if (mode == UI_MIDDLE_MODE_BLE_CODE) {
		SYSTEM_get_ble_code(string);
		len = 4;
		b_center = TRUE;
	} else if (mode == UI_MIDDLE_MODE_LINKING) {
		
		string[0] = ICON32_PHONE_FINDER_1;
		string[1] = ' ';
		string[2] = ICON32_PHONE_FINDER_2;
		string[3] = ' ';
		string[4] = ICON32_PHONE_FINDER_4;
		len = 5;
		b_center = TRUE;
	}

	if (len > 0) {
		offset = _render_middle_section_large(len, string, margin);
	}
	
	// Shift all the display to the middle
	if (b_center) {
		p0 = cling.ui.p_oled_up+128;
		p1 = p0+128;
		p2 = p1+128;
		ptr = (128 - offset)>>1;
		
		if (ptr > 0) {
			p0 += 127; p4 = p0 - ptr;
			p1 += 127; p5 = p1 - ptr;
			p2 += 127; p6 = p2 - ptr;
			for (i = 0; i < 128-ptr; i++) {
				*p0-- = *p4--;
				*p1-- = *p5--;
				*p2-- = *p6--;
			}
			for (; i < 128; i++) {
				*p0-- = 0;
				*p1-- = 0;
				*p2-- = 0;
			}
		}
	}	
	
	if (b_more & cling.ui.b_detail_page) {

		if ((mode == UI_MIDDLE_MODE_APP_NOTIF) ||
				(mode == UI_MIDDLE_MODE_DETAIL_NOTIF) ||
				(mode == UI_MIDDLE_MODE_MESSAGE)
		) {
			_render_icon_more(115);
		} else {
			_render_icon_more(110);
		}
	}
	
	// Add progress bar at the bottome
	if (b_progress_bar) {
		p0 = cling.ui.p_oled_up+128+128+128+14;
		for (i = 0; i < bar_len; i++) {
			*p0++ = 0xf0;
		}
		for (; i < 100; i++) {
			*p0++ = 0x80;
		}
	}
	
	return b_full_screen;
}

static I8U _render_top_sec(I8U *string, I8U len, I8U offset)
{
	I8U i, j, margin = 1;
	const I8U *pin;
	I8U *p0;
	
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset; // Add extra 10 pixels to make it center
		if (string[i] == ' ') {
			offset += 4;
		} else {
			pin = asset_content+asset_pos[string[i]];
			for (j = 0; j < asset_len[string[i]]; j++) {
					*p0++ = (*pin++);
			}
			if (i != (len-1))
				offset += asset_len[string[i]] + margin;
			else
				offset += asset_len[string[i]];
		}
	}
	
	return offset;
}

static void _render_firmware_ver()
{
	I8U string[64];
	I16U len;
	I16U major;
	I16U minor;
	
	major = cling.system.mcu_reg[REGISTER_MCU_REVH]>>4;
	minor = cling.system.mcu_reg[REGISTER_MCU_REVH]&0x0f;
	minor <<= 8;
	minor |= cling.system.mcu_reg[REGISTER_MCU_REVL];
	
	len = sprintf((char *)string, "VER:%d.%d(%d)", major, minor,cling.whoami.touch_ver[2]);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
}

static void _render_2_indicator(I8U len1, const I8U *in1, I8U offset1, I8U len2, const I8U *in2, I8U offset2)
{
	I8U *p0, *p1, *pin;
	I8U j;

	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset1+98;
	p1 = p0 + 128;
	pin = (I8U *)in1;
	for (j = 0; j < len1; j++) {
		*p0++ = (*pin++);
		*p1++ = (*pin++);
	}
	
	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset2+128+128+128+98;
	pin = (I8U *)in2;
	for (j = 0; j < len2; j++) {
			*p0++ = (*pin++);
	}
}

static void _render_indicator(I8U len1, const I8U *in1, I8U offset)
{
	I8U *p0, *pin, *p1;
	I8U j;

	// Render the left side (offset set to 60 for steps comment)
	p0 = cling.ui.p_oled_up+offset;
	p1 = p0 + 128;
	pin = (I8U *)in1;
	for (j = 0; j < len1; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
	}
}


static void _left_icon_render(I8U mode)
{
	I16U i;
	I16U offset = 0; // Pixel offet at top row
	I8U string[64];
	I8U len=0;
	WEATHER_CTX weather;

	// Render the left side 
	if (mode == UI_TOP_MODE_NONE) {
	} else if (mode == UI_TOP_MODE_STEPS) {
			// Render the left side
		_render_one_icon(ICON_TOP_STEPS_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_STEPS);
	} else if (mode == 	UI_TOP_MODE_DISTANCE) {
		_render_one_icon(ICON_TOP_DISTANCE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_DISTANCE);
	} else if (mode == UI_TOP_MODE_CALORIES) {
		_render_one_icon(ICON_TOP_CALORIES_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_CALORIES);
	} else if (mode == UI_TOP_MODE_SLEEP) {
		_render_one_icon(ICON_TOP_SLEEP_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_SLEEP);
	} else if (mode == UI_TOP_MODE_UV_IDX) {
		_render_one_icon(ICON_TOP_UV_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_UV);
	} else if (mode == UI_TOP_MODE_INCOMING_CALL) {
		_render_one_icon(ICON_TOP_CALLS_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_CALLS);
	} else if (mode == UI_TOP_MODE_2DIGITS_INDEX) {

		len = sprintf((char *)string, "%02d", cling.ui.level_1_index);
		FONT_load_characters(cling.ui.p_oled_up, (char *)string, 16, FALSE);

	} else if (mode == UI_TOP_MODE_MESSAGE) {
		_render_one_icon(ICON_TOP_MESSAGE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_MESSAGE);

	} else if (mode == UI_TOP_MODE_REMINDER) {
		if (cling.reminder.state != REMINDER_STATE_IDLE) {
				if (cling.ui.clock_sec_blinking) {
					cling.ui.clock_sec_blinking = FALSE;
					_render_one_icon(ICON_TOP_REMINDER_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_REMINDER);
				} else {
					cling.ui.clock_sec_blinking = TRUE;
					// Empty icon to have blinking effects
				}
		} else {
		
				// Render top row REMINDER icon
				_render_one_icon(ICON_TOP_REMINDER_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_REMINDER);
		}
	} else if (mode == UI_TOP_MODE_HEART_RATE) {
		if (TOUCH_is_skin_touched()) {
				if (cling.ui.clock_sec_blinking) {
					cling.ui.clock_sec_blinking = FALSE;
					_render_one_icon(ICON_TOP_HEART_RATE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_HEART_RATE);
				} else {
					cling.ui.clock_sec_blinking = TRUE;
					// Empty icon to have blinking effects
				}
		} else {
			// Render top row heart rate icon
			_render_one_icon(ICON_TOP_HEART_RATE_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_HEART_RATE);
		}
	} else if (mode == UI_TOP_MODE_SKIN_TEMP) {
		N_SPRINTF("[UI] display skin temp (2)");
		_render_one_icon(ICON_TOP_SKIN_TEMP_LEN, cling.ui.p_oled_up+offset, asset_content+ICON_TOP_SKIN_TEMP);
	} else if (mode == UI_TOP_MODE_WEATHER) {
		WEATHER_get_weather(0, &weather);
		_render_one_icon(asset_len[ICON_WEATHER_IDX+weather.type], cling.ui.p_oled_up+offset, asset_content+asset_pos[ICON_WEATHER_IDX+weather.type]);
	} else if (mode == UI_TOP_MODE_RETURN) {
		_render_one_icon(ICON_TOP_RETURN_LEN, cling.ui.p_oled_up, asset_content+ICON_TOP_RETURN);
	} else if (mode == UI_TOP_MODE_OK) {
		_render_one_icon(ICON_TOP_OK_LEN, cling.ui.p_oled_up+111, asset_content+ICON_TOP_OK);
	} else if (mode == UI_TOP_MODE_WORKOUT_START) {
		_render_2_indicator(ICON_TOP_WORKOUT_START_ICON_LEN, asset_content+ICON_TOP_WORKOUT_START_ICON, 5,
												ICON_TOP_WORKOUT_START_TEXT_LEN, asset_content+ICON_TOP_WORKOUT_START_TEXT, 0);
	} else if (mode == UI_TOP_MODE_WORKOUT_STOP) {
		_render_2_indicator(ICON_TOP_WORKOUT_STOP_ICON_LEN, asset_content+ICON_TOP_WORKOUT_STOP_ICON, 5,
												ICON_TOP_WORKOUT_STOP_TEXT_LEN, asset_content+ICON_TOP_WORKOUT_STOP_TEXT, 0);
	}

	
	if (len > 0) {
		for (i = 0; i < len; i++) {
			// Render the left side
			_render_one_icon(asset_len[string[i]], cling.ui.p_oled_up+offset, asset_content+asset_pos[string[i]]);
			
			offset += asset_len[string[i]] + 1;
		}
	}

}

static void _left_system_render(I8U *in, BOOLEAN b_charging, BOOLEAN b_ble_conn)
{	
	I8U *p0, *pin;
	I8U j;
	I8U curr_batt_level = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	I8U p_v = 0x1c;

	if (b_ble_conn) {
		p0 = in + ICON_BOTTOM_IND_CHARGING_LEN + 2;
		pin = (I8U *)(asset_content+ICON_TOP_SMALL_BLE);
		for (j = 0; j < ICON_TOP_SMALL_BLE_LEN; j++) {
				*p0++ = (*pin++);
		}
	}
	
	// Render the right side (offset set to 60 for steps comment)
	p0 = in;
	
	if (b_charging) {
		pin = (I8U *)(asset_content+ICON_BOTTOM_IND_CHARGING);
		for (j = 0; j < ICON_BOTTOM_IND_CHARGING_LEN; j++) {
				*p0++ = (*pin++);
		}
	} else {
		pin = (I8U *)(asset_content+ICON_BOTTOM_IND_NO_CHARGING);
		for (j = 0; j < ICON_BOTTOM_IND_NO_CHARGING_LEN; j++) {
				*p0++ = (*pin++);
		}
	}
	
	// Filling up the percentage
	curr_batt_level /= 11;
	
	if (curr_batt_level == 0)
		return;
	
	if (curr_batt_level > 9)
		curr_batt_level = 9;
	
	// Note: the battery button icon is 9 pixels of length
	p0 = in+2;
	for (j = 0; j < curr_batt_level; j++) {
		*p0++ |= p_v;
	}
	
	N_SPRINTF("[UI] left rendering: %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
}

static void render_ble_batt_rotation(BOOLEAN b_charging, BOOLEAN b_ble_conn, BOOLEAN b_90_degree)
{
	I8U data_buf[128];
	
	memset(data_buf, 0, 128);
	
	_left_system_render(data_buf, b_charging, b_ble_conn);
	
	if (b_90_degree)
		_rotate_90_degree(data_buf, cling.ui.p_oled_up+120);
	else
		_rotate_270_degree(data_buf, cling.ui.p_oled_up+384);

}


static void _render_calendar(I8U *buf, SYSTIME_CTX time)
{
	I8U string[64];
	I8U len, offset=93;
	char *week[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

	if (cling.ui.fonts_type) {
		string[0] = ICON_DOW_IDX+time.dow;
		
		len = 1;
		
		len += sprintf((char *)(string+1), " %d",time.day);
		
		_render_top_sec(string, len, offset);

		len = sprintf((char *)string, " %d", time.day);
		FONT_load_characters(buf+(128-len*6), (char *)string, 8, FALSE);
	} else {
		len = sprintf((char *)string, "%s %d", week[time.dow], time.day);
		FONT_load_characters(buf+(128-len*6), (char *)string, 8, FALSE);
	}
}

static void _render_calendar_rotation(SYSTIME_CTX time, BOOLEAN b_90_degree)
{
	I8U data_buf[128];
	I8U string[32];

	memset(data_buf, 0, 128);
	
	sprintf((char *)string, "%d/%02d",time.month, time.day);
	FONT_load_characters(data_buf, (char *)string, 8, FALSE);
	if (b_90_degree)
		_rotate_90_degree(data_buf, cling.ui.p_oled_up);
	else
		_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+120);
}

static void _render_dow_rotation(SYSTIME_CTX time, BOOLEAN b_90_degree)
{
	I8U data_buf[128];
	I8U string[32];
	I8U j;
	const I8U *pin;
	I8U *p0;
	char *week_en[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

	memset(data_buf, 0, 128);
	
	if (cling.ui.fonts_type) {
		p0 = data_buf; // Add extra 10 pixels to make it center
		pin = asset_content+asset_pos[ICON_DOW_IDX+time.dow];
		for (j = 0; j < asset_len[ICON_DOW_IDX+time.dow]; j++) {
				*p0++ = (*pin++);
		}

	} else {
		sprintf((char *)string, "%s", week_en[time.dow]);
		FONT_load_characters(data_buf, (char *)string, 8, FALSE);
	}
	
	if (b_90_degree)
		_rotate_90_degree(data_buf, cling.ui.p_oled_up+8);
	else // 270 degree
		_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+112);
}

static void _render_battery_perc_rotation(BOOLEAN b_90_degree)
{
	I8U data_buf[128];
	I8U string[32];
				
	memset(data_buf, 0, 128);
	
	sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);	
	FONT_load_characters(data_buf, (char *)string, 8, FALSE);
	if (b_90_degree)
		_rotate_90_degree(data_buf, cling.ui.p_oled_up);
	else // 270 degree
		_rotate_270_degree(data_buf, cling.ui.p_oled_up+384+120);
}

static void _render_clock(SYSTIME_CTX time)
{
	I8U string[64];
	I8U len;

	len = sprintf((char *)string, "%d:%02d",time.hour, time.minute);
	FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
}

static void _right_row_render(I8U mode)
{	
	I8U string[64];
	I8U len;
	SYSTIME_CTX delta;

	// Render the right side
	if (mode == UI_BOTTOM_MODE_NONE) {
	} else if (mode == UI_BOTTOM_MODE_2DIGITS_INDEX) {

		if (cling.reminder.ui_alarm_on)
			return;
		
		len = sprintf((char *)string, "%02d", cling.ui.level_1_index);
		//FONT_load_characters(cling.ui.p_oled_up+(120-len*6), (char *)string, 8, FALSE);
		FONT_load_characters(cling.ui.p_oled_up+(128-len*8), (char *)string, 16, FALSE);

	} else if (mode == UI_BOTTOM_MODE_CLOCK) {
		
		_render_clock(cling.time.local);
		
	} else if (mode == UI_BOTTOM_MODE_CALENDAR) {
		if (cling.ui.clock_orientation == 1) {
			_render_calendar(cling.ui.p_oled_up, cling.time.local);
		} else if (cling.ui.clock_orientation == 2) {
			_render_calendar_rotation(cling.time.local, FALSE);
			_render_dow_rotation(cling.time.local, FALSE);
		} else {
			_render_calendar_rotation(cling.time.local, TRUE);
			_render_dow_rotation(cling.time.local, TRUE);
		}
	} else if (mode == UI_BOTTOM_MODE_DELTA_DATE_BACKWARD) {
		
		RTC_get_delta_clock_backward(&delta, cling.ui.level_1_index);
		_render_calendar(cling.ui.p_oled_up, delta);
		
	} else if (mode == UI_BOTTOM_MODE_MAX) {
		_render_indicator(ICON_BOTTOM_IND_MAX_LEN, asset_content+ICON_BOTTOM_IND_MAX, 123);

	} else if (mode == UI_BOTTOM_MODE_FIRMWARE_VER) {
		_render_firmware_ver();
	} else if (mode == UI_BOTTOM_MODE_CHARGING_PERCENTAGE) {
		if (cling.ui.clock_orientation == 1) {
			len = sprintf((char *)string, "%d %%", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
			FONT_load_characters(cling.ui.p_oled_up+(128-len*6), (char *)string, 8, FALSE);
		} else if (cling.ui.clock_orientation == 2) {
			_render_battery_perc_rotation(FALSE);
		} else {
			_render_battery_perc_rotation(TRUE);
		}
	} else if (mode == UI_BOTTOM_MODE_OK) {
		_render_one_icon(ICON_TOP_OK_LEN, cling.ui.p_oled_up+111, asset_content+ICON_TOP_OK);
	}
}

static void _render_screen()
{
		OLED_full_scree_show(cling.ui.p_oled_up);

		OLED_set_display(1);
}

static I8U _render_carousel_icon_single(I16U x, I8U char_len, I8U index)
{
	I8U j;
	I8U *p0, *p1, *p2;
	const I8U *pin;
	
	if (!index)
		return 0;

	// We might add some margin here
	p0 = cling.ui.p_oled_up+128+x;
	p1 = p0 + 128;
	p2 = p1 + 128;

	pin = asset_content+asset_pos[512+index];
	for (j = 0; j < char_len; j++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
	}
	
	return char_len;
}

static void _core_display_carousel(I8U left, I8U top, I8U right, BOOLEAN b_render)
{
	I8U start_offset;
	I8U char_len;
	
	// Clean up top row
	memset(cling.ui.p_oled_up, 0, 512);
	
	// Render the left icon
	char_len = asset_len[512+left];
	start_offset = 0;
	_render_carousel_icon_single(start_offset, char_len, left);
	
	char_len = asset_len[512+top];
	start_offset = 42-char_len;
	start_offset >>= 1;
	start_offset += 42;
	_render_carousel_icon_single(start_offset, char_len, top);
	
	char_len = asset_len[512+right];
	start_offset = 128-char_len;
	_render_carousel_icon_single(start_offset, char_len, right);
	
	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}

static void _core_home_display_horizontal(I8U middle, I8U bottom, BOOLEAN b_render)
{
	BOOLEAN b_charging = FALSE;
	BOOLEAN b_conn = FALSE;
	I8U i;
	I8U *p0, *p1, *p2, *p3;
	
	// Main info
	_middle_row_render(middle, TRUE);

	// Info on the right
	if (BATT_is_charging())
		b_charging = TRUE;
	if (BTLE_is_connected())
		b_conn = TRUE;

	// Charging icon & BLE connection
	if (middle == UI_MIDDLE_MODE_CLOCK) {
		if (cling.ui.clock_orientation == 1) {
			_left_system_render(cling.ui.p_oled_up, b_charging, b_conn);
		} else if (cling.ui.clock_orientation == 2) {
			render_ble_batt_rotation(b_charging, b_conn, FALSE);
		} else {
			render_ble_batt_rotation(b_charging, b_conn, TRUE);
		}
	} else {
		_left_system_render(cling.ui.p_oled_up, b_charging, b_conn);
	}

	// Info on the left
	_right_row_render(bottom);
	
	// For clock page, we clean up a bit for the 'more' icon
	
	if ((middle == UI_MIDDLE_MODE_CLOCK) && cling.ui.b_detail_page) {
		p0 = cling.ui.p_oled_up+110;
		p1 = p0 + 128;
		p2 = p1 + 128;
		p3 = p2 + 128;
		for (i = 110; i < 128; i ++) {
			*p0++ = 0;
			*p1++ = 0;
			*p2++ = 0;
			*p3++ = 0;
		}
		N_SPRINTF("[UI] clean up space for detail icon");
		_render_icon_more(110);
	}
	
	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}

static void _core_display_horizontal(I8U top, I8U middle, I8U bottom, BOOLEAN b_render)
{	
	// Main info
	if (!_middle_row_render(middle, TRUE)) {
		// Info on the left
		_left_icon_render(top);
		
		_right_row_render(bottom);
	}
	
	if (b_render) {
		// Finally, we render the frame
		_render_screen();
	}
}

static void _display_unauthorized_home()
{
	_core_home_display_horizontal(UI_MIDDLE_MODE_BLE_CODE, UI_BOTTOM_MODE_FIRMWARE_VER, TRUE);
}

static void _display_authenticating()
{
	_core_home_display_horizontal(UI_MIDDLE_MODE_LINKING, UI_BOTTOM_MODE_NONE, TRUE);
}

static void _display_tracking_stats(UI_ANIMATION_CTX *u, BOOLEAN b_render, I8U top, I8U middle, I8U bottom)
{
		N_SPRINTF("[UI] tracking frame: %d", u->level_1_index);
	if (u->level_1_index == 0) {
		_core_display_horizontal(top, middle, bottom, b_render);
	} else  {
		_core_display_horizontal(top, middle, UI_BOTTOM_MODE_DELTA_DATE_BACKWARD, b_render);
	}
}

static void _display_frame_tracker(I8U index, BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (index) {
		case UI_DISPLAY_TRACKER_STEP:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_STEPS, UI_MIDDLE_MODE_STEPS, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_DISTANCE:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_DISTANCE, UI_MIDDLE_MODE_DISTANCE, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_CALORIES:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_CALORIES, UI_MIDDLE_MODE_CALORIES, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_SLEEP:
			_display_tracking_stats(u, b_render, UI_TOP_MODE_SLEEP, UI_MIDDLE_MODE_SLEEP, UI_BOTTOM_MODE_CLOCK);
			break;
		case UI_DISPLAY_TRACKER_UV_IDX:
			N_SPRINTF("[UI] uv index (1)");
			_display_tracking_stats(u, b_render, UI_TOP_MODE_UV_IDX, UI_MIDDLE_MODE_UV_IDX, UI_BOTTOM_MODE_CLOCK);
			break;
		default:
			break;
	}
}

static void _display_weather(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
		_core_display_horizontal(UI_TOP_MODE_WEATHER, UI_MIDDLE_MODE_WEATHER, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_idle_alert()
{
	I8U string1[32];

	memset(cling.ui.p_oled_up, 0, 512);
	
	if (cling.ui.fonts_type == FONTS_TYPE_SIMPLE_CHINESE){	
		sprintf((char *)string1, "该起来动动了");
	} 
	else if (cling.ui.fonts_type == FONTS_TYPE_TRADITIONAL_CHINESE){
		sprintf((char *)string1, "該起來動動了");
	}
	else {
		sprintf((char *)string1, "time for a move");
	}

	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string1, 16, TRUE);

	_render_screen();
}

static void _display_frame_workout(I8U index, BOOLEAN b_render)
{
	I8U string1[32];
	I8U string2[32];
	I8U len2=0, workout_idx=0;
	char *workout_en[] = {"Walk", "Run", "Row", "Elliptical", "Stairs", "Cycle", "Aerobic", "Piloxing", "Others", "Outdoor", "Indoor"};
	char *workout_s_cn[] = {"徒步", "跑步", "划船", "椭圆机 ", "爬楼梯 ", "单车", "有氧操 ", "Piloxing", "其它", "户外", "室内"};
	char *workout_t_cn[] = {"徒步", "跑步", "划船", "橢圓機 ", "爬樓梯 ", "單車", "有氧操 ", "Piloxing", "其它", "戶外", "室內"};
	char *workout_indicator[] = {
		"-,,,,,,,,",
		",-,,,,,,,", 
		",,-,,,,,,", 
		",,,-,,,,,", 
		",,,,-,,,,", 
		",,,,,-,,,", 
		",,,,,,-,,", 
		",,,,,,,-,", 
		",,,,,,,,-", 
		"-,",
		",-"};
	
	memset(cling.ui.p_oled_up, 0, 512);
	
	if (index < UI_DISPLAY_WORKOUT_WALKING) {
		memset(string1, 0, 32);
		memset(string2, 0, 32);
	} else if (index > UI_DISPLAY_WORKOUT_OUTDOOR) {
		memset(string1, 0, 32);
		memset(string2, 0, 32);
	} else {
		workout_idx = index - UI_DISPLAY_WORKOUT_WALKING;
	}
	
	if (cling.ui.fonts_type == FONTS_TYPE_SIMPLE_CHINESE){	
		sprintf((char *)string1, "%s", workout_s_cn[workout_idx]);
	}
	else if (cling.ui.fonts_type == FONTS_TYPE_TRADITIONAL_CHINESE){
		sprintf((char *)string1, "%s", workout_t_cn[workout_idx]);
	}
	else{
		sprintf((char *)string1, "%s", workout_en[workout_idx]);
	}
	len2 = sprintf((char *)string2, "%s", workout_indicator[workout_idx]);
	
	FONT_load_characters(cling.ui.p_oled_up+128, (char *)string1, 16, TRUE);
	
	_display_dynamic(cling.ui.p_oled_up+128*3, len2, string2);
	
	_left_icon_render(UI_TOP_MODE_RETURN);
	_left_icon_render(UI_TOP_MODE_OK);
	
	if (b_render) {

		// Finally, we render the frame
		_render_screen();
	}
}

static void _display_stopwatch_core(I8U *string1, I8U len1, I8U *string2, I8U len2, I8U mode, BOOLEAN b_center)
{
	I8U *p0, *pin, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U i, j, ptr, offset1, char_len;

	// Text line
	p0 = cling.ui.p_oled_up;
	p1 = p0 + 128;
	p2 = p1 + 128;
	offset1 = 0;
	for (i = 0; i < len1; i++) {
		
		if (string1[i] == '1') {
			p0 += 2;
			p1 += 2;
			p2 += 2;
			offset1 += 2;
		}
		
		pin = (I8U *)(asset_content+asset_pos[512+string1[i]]);
		char_len = asset_len[512+string1[i]];
		for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
		}
		offset1 += char_len;
		p0 ++;
		p1 ++;
		p2 ++;
		if (i < len1)
			offset1 ++;
		
		if (string1[i] == '1') {
			p0 += 3;
			p1 += 3;
			p2 += 3;
			offset1 += 3;
		}
		
	}

	p0 = cling.ui.p_oled_up;
	p1 = p0+128;
	p2 = p1+128;
	if (b_center) {
		
		ptr = (128 - offset1)>>1;
		
		if (ptr > 0) {
			p0 += 127; p4 = p0 - ptr;
			p1 += 127; p5 = p1 - ptr;
			p2 += 127; p6 = p2 - ptr;
			for (i = 0; i < 128-ptr; i++) {
				*p0-- = *p4--;
				*p1-- = *p5--;
				*p2-- = *p6--;
			}
			for (; i < 128; i++) {
				*p0-- = 0;
				*p1-- = 0;
				*p2-- = 0;
			}
		}
	} else if (b_center > 1) {
		ptr = 128 - offset1;
		
		if (ptr > 0) {
			p0 += 127; p4 = p0 - ptr;
			p1 += 127; p5 = p1 - ptr;
			p2 += 127; p6 = p2 - ptr;
			for (i = 0; i < offset1; i++) {
				*p0-- = *p4--;
				*p1-- = *p5--;
				*p2-- = *p6--;
			}
			for (; i < 128; i++) {
				*p0-- = 0;
				*p1-- = 0;
				*p2-- = 0;
			}
		}
	}
	
	if (len2 > 0) {
		// indicator
		p2 = cling.ui.p_oled_up+128+128+128+60;
		for (i = 0; i < len2; i++) {
			pin = (I8U *)(asset_content+asset_pos[string2[i]]);
			char_len = asset_len[string2[i]];
			for (j = 0; j < char_len; j++) {
					*p2++ = (*pin++);
			}
			p2 += 5;			
		}
	}
	
	_left_icon_render(mode);
}

static void _display_stopwatch_start(I8U *string1, I8U len1, I8U mode)
{
	I8U *p0, *pin, *p1, *p2;
	I8U *p4, *p5, *p6;
	I8U i, j, ptr, offset1, char_len;

	// Text line
	p0 = cling.ui.p_oled_up;
	p1 = p0 + 128;
	p2 = p1 + 128;
	offset1 = 0;
	for (i = 0; i < len1; i++) {
		
		pin = (I8U *)(asset_content+asset_pos[512+string1[i]]);
		char_len = asset_len[512+string1[i]];
		for (j = 0; j < char_len; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
				*p2++ = (*pin++);
		}
		offset1 += char_len;
		p0 ++;
		p1 ++;
		p2 ++;
		if (i < len1)
			offset1 ++;
	}

	p0 = cling.ui.p_oled_up;
	p1 = p0+128;
	p2 = p1+128;
	
	ptr = (128 - offset1)>>1;
	
	if (ptr > 0) {
		p0 += 127; p4 = p0 - ptr;
		p1 += 127; p5 = p1 - ptr;
		p2 += 127; p6 = p2 - ptr;
		for (i = 0; i < 128-ptr; i++) {
			*p0-- = *p4--;
			*p1-- = *p5--;
			*p2-- = *p6--;
		}
		for (; i < 128; i++) {
			*p0-- = 0;
			*p1-- = 0;
			*p2-- = 0;
		}
	}

	
	_left_icon_render(mode);
}

static void _display_frame_setting(I8U index, BOOLEAN b_render)
{
	_core_display_horizontal(UI_TOP_MODE_RETURN, UI_MIDDLE_MODE_BLE_CODE, UI_BOTTOM_MODE_FIRMWARE_VER, b_render);
}


static void _display_frame_stopwatch(I8U index, BOOLEAN b_render)
{
	I8U string1[32];
	I8U string2[32];
	I8U len1=0, len2 = 0;
	BOOLEAN b_center = TRUE;
	I32U t_diff, hour, minute, second;
	
	memset(cling.ui.p_oled_up, 0, 512);
	
	switch (index) {
		case UI_DISPLAY_STOPWATCH_START:
		{
			len1 = sprintf((char *)string1, "00.00");
			_display_stopwatch_start(string1, len1, UI_TOP_MODE_WORKOUT_START);
			_left_icon_render(UI_TOP_MODE_RETURN);
			//_render_one_icon(ICON_TOP_RETURN_LEN, cling.ui.p_oled_up, asset_content+ICON_TOP_RETURN);
			break;
		} 
		case UI_DISPLAY_STOPWATCH_STOP:
		{
			t_diff = CLK_get_system_time();
			t_diff -= cling.activity.workout_time_stamp_start;
			t_diff /= 1000;
			hour = t_diff / 3600;
			t_diff -= hour*3600;
			minute = t_diff / 60;
			t_diff -= minute * 60;
			second = t_diff;
			len1 = sprintf((char *)string1, "%d:%02d.%02d", hour, minute, second);
			len2 = sprintf((char *)string2, "-,,");
			b_center = FALSE;
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_WORKOUT_STOP, b_center);
			
			N_SPRINTF("[UI] stopwatch stop: %d:%d.%d (%d,%d)", hour, minute, second, CLK_get_system_time(), cling.activity.workout_time_stamp_start);
			break;
		} 
		case UI_DISPLAY_STOPWATCH_RESULT:
		{
			t_diff = cling.activity.workout_time_stamp_stop - cling.activity.workout_time_stamp_start;
			t_diff /= 1000;
			hour = t_diff / 3600;
			t_diff -= hour*3600;
			minute = t_diff / 60;
			t_diff -= minute * 60;
			second = t_diff;
			len1 = sprintf((char *)string1, "%d:%d.%d", hour, minute, second);
			len2 = 0;
			b_center = 2;
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_RETURN, b_center);
			break;
		}
		case UI_DISPLAY_STOPWATCH_HEARTRATE:
		{
			if (TOUCH_is_skin_touched()) {

				len1 = sprintf((char *)string1, "%d", PPG_minute_hr_calibrate());

			} else {
				N_SPRINTF("[UI] Heart rate - not valid");
				
				len1 = 0;
				string1[len1++] = ICON_MIDDLE_NO_SKIN_TOUCH;
				string1[len1] = 0;
			}
		
			len2 = sprintf((char *)string2, ",-,");
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_HEART_RATE, b_center);
			break;
		}
		case UI_DISPLAY_STOPWATCH_CALORIES:
		{
			t_diff = cling.activity.day.calories - cling.activity.workout_Calories_start;
			t_diff += cling.activity.workout_Calories_acc;
			
			len1 = sprintf((char *)string1, "%d", (t_diff>> 4));
			len2 = sprintf((char *)string2, ",,-");
			_display_stopwatch_core(string1, len1, string2, len2, UI_TOP_MODE_CALORIES, b_center);
			break;
		}
		default:
			break;
	}
	
	//_left_icon_render(UI_TOP_MODE_RETURN);
	
	if (b_render) {

		// Finally, we render the frame
		_render_screen();
	}
}

static void _display_frame_smart(I8U index, BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (index) {
		case UI_DISPLAY_SMART_WEATHER:
			_display_weather(u, b_render);
			break;
		case UI_DISPLAY_SMART_MESSAGE:
			_core_display_horizontal(UI_TOP_MODE_RETURN, UI_MIDDLE_MODE_MESSAGE, UI_BOTTOM_MODE_CLOCK, b_render);
			break;
		case UI_DISPLAY_SMART_APP_NOTIF:
			_core_display_horizontal(UI_TOP_MODE_RETURN, UI_MIDDLE_MODE_APP_NOTIF, UI_BOTTOM_MODE_2DIGITS_INDEX, b_render);
			break;
		case UI_DISPLAY_SMART_DETAIL_NOTIF:
			_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_DETAIL_NOTIF, UI_BOTTOM_MODE_NONE, b_render);
			break;
		case UI_DISPLAY_SMART_REMINDER:
			_core_display_horizontal(UI_TOP_MODE_REMINDER, UI_MIDDLE_MODE_REMINDER, UI_BOTTOM_MODE_2DIGITS_INDEX, b_render);
			break;
		case UI_DISPLAY_SMART_PHONE_FINDER:
			_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_PHONE_FINDER, UI_BOTTOM_MODE_NONE, b_render);
			break;
		case UI_DISPLAY_SMART_INCOMING_CALL:
			_core_display_horizontal(UI_TOP_MODE_INCOMING_CALL, UI_MIDDLE_MODE_INCOMING_CALL, UI_BOTTOM_MODE_OK, b_render);
			break;
		case UI_DISPLAY_SMART_INCOMING_MESSAGE:
			_core_display_horizontal(UI_TOP_MODE_RETURN, UI_MIDDLE_MODE_INCOMING_MESSAGE, UI_BOTTOM_MODE_CLOCK, b_render);
			break;
		default:
			break;
	}
}

static void _display_heart_rate(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
	_core_display_horizontal(UI_TOP_MODE_HEART_RATE, UI_MIDDLE_MODE_HEART_RATE, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_skin_temperature(UI_ANIMATION_CTX *u, BOOLEAN b_render)
{
	N_SPRINTF("[UI] display skin temp (1)");
	// Current skin temperature
	_core_display_horizontal(UI_TOP_MODE_SKIN_TEMP, UI_MIDDLE_MODE_SKIN_TEMP, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_frame_vital(I8U index, BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	switch (index) {
		case UI_DISPLAY_VITAL_SKIN_TEMP:
			N_SPRINTF("[UI] vital: skin temp");
			_display_skin_temperature(u, b_render);
			break;
		case UI_DISPLAY_VITAL_HEART_RATE:
			N_SPRINTF("[UI] vital: heart rate");
			_display_heart_rate(u, b_render);
			break;
		default:
			break;
	}
}

static void _display_frame_carousel(I8U index, BOOLEAN b_render)
{	
	switch (index) {
		case UI_DISPLAY_CAROUSEL_1:
			_core_display_carousel(ICON32_WORKOUT_IDX, ICON32_STOPWATCH_IDX, ICON32_MESSAGE_IDX, b_render);
			break;
		case UI_DISPLAY_CAROUSEL_2:
			_core_display_carousel(ICON32_WEATHER_IDX, ICON32_ALARM_IDX, ICON32_SETTING_IDX, b_render);
			break;
		default:
			break;
	}
}

static void _display_frame_ota(I8U index, BOOLEAN b_render)
{
		_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_OTA, UI_BOTTOM_MODE_NONE, b_render);
}

static void _display_sos(BOOLEAN b_render)
{
	_core_display_horizontal(UI_TOP_MODE_NONE, UI_MIDDLE_MODE_SOS, UI_BOTTOM_MODE_CLOCK, b_render);
}

static void _display_frame_home(BOOLEAN b_render)
{
		if (BATT_is_charging()) {
			_core_home_display_horizontal(UI_MIDDLE_MODE_CLOCK, UI_BOTTOM_MODE_CHARGING_PERCENTAGE, b_render);
		} else {
			_core_home_display_horizontal(UI_MIDDLE_MODE_CLOCK, UI_BOTTOM_MODE_CALENDAR, b_render);
		}
}

static void _display_frame_appear(I8U index, BOOLEAN b_render)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	N_SPRINTF("[UI] frame appear: %d", index);

	if (index == UI_DISPLAY_HOME) {
		_display_frame_home(b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_TRACKER) && (index <= UI_DISPLAY_TRACKER_END)) {
		_display_frame_tracker(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_SMART) && (index <= UI_DISPLAY_SMART_END)) {
		_display_frame_smart(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_VITAL) && (index <= UI_DISPLAY_VITAL_END)) {
		_display_frame_vital(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_WORKOUT) && (index <= UI_DISPLAY_WORKOUT_END)) {
		_display_frame_workout(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_STOPWATCH) && (index <= UI_DISPLAY_STOPWATCH_END)) {
		_display_frame_stopwatch(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_SETTING) && (index <= UI_DISPLAY_SETTING_END)) {
		_display_frame_setting(index, b_render);
		u->frame_cached_index = index;
	} else if ((index >= UI_DISPLAY_CAROUSEL) && (index <= UI_DISPLAY_CAROUSEL_END)) {
		_display_frame_carousel(index, b_render);
	} else if (index == UI_DISPLAY_OTA) {
		_display_frame_ota(index, b_render);
	} else {
	}
}

static void _render_logo()
{
	I16U i;
	I16U strt_offset;
	I8U *p0, *p1, *p2, *p3;
	const I8U *pin;

	memset(cling.ui.p_oled_up, 0, 512);
	strt_offset = (128-ICON32_LOGO_LEN)>>1;
	p0 = cling.ui.p_oled_up+strt_offset;
	p1 = p0+128;
	p2 = p1+128;
	p3 = p2+128;
	pin = asset_content+ICON32_LOGO;
	for (i = 0; i < ICON32_LOGO_LEN; i++) {
			*p0++ = (*pin++);
			*p1++ = (*pin++);
			*p2++ = (*pin++);
			*p3++ = (*pin++);
	}

	// Finally, we render the frame
	_render_screen();
}

static void _display_charging(I16U perc)
{
	I16U i, j;
	I16U fill_len;
	I8U *p0, *p1;
	I16U offset = 0; // Pixel offet at top row
	I8U string[64];
	I8U len=0;
		const I8U *pin;

	// Render the icon the first
	memset(cling.ui.p_oled_up, 0, 512);
	if (BATT_is_charging()) {
		string[len++] = ICON_MIDDLE_CHARGING_BATT_IDX;
		string[len++] = ICON_MIDDLE_CHARGING_ICON_IDX;
	} else {
		string[len++] = ICON_MIDDLE_CHARGING_BATT_IDX;
	}
	
	// Rendering the icon
	offset = 15;
	for (i = 0; i < len; i++) {
		p0 = cling.ui.p_oled_up+offset+128;
		p1 = p0+128;
		pin = asset_content+asset_pos[256+string[i]];
		for (j = 0; j < asset_len[256+string[i]]; j++) {
				*p0++ = (*pin++);
				*p1++ = (*pin++);
		}
		
		if (i != (len-1))
			offset += asset_len[256+string[i]] + 5;
		else
			offset += asset_len[256+string[i]];
	}
	
	// Shift up by 6 pixels
	//_frame_buffer_vertical_adj(-6);
	
	// Filling up the percentage
	//perc = cling.system.mcu_reg[REGISTER_MCU_BATTERY];
	if (perc >= 100) {
		perc = 100;
		fill_len = 43;
	} else {
		fill_len = perc*43/100;
	}
	
	// Filling the percentage
	offset = 15;
	p0 = cling.ui.p_oled_up+2+128+offset;
	p1 = p0+128;
	for (i = 0; i < fill_len; i++) {
		*p0++ |= 0xfc;
		*p1++ |= 0x3f;
	}
	
	// show the percentage
	len = sprintf((char *)string, "%d%%", perc);
	FONT_load_characters(cling.ui.p_oled_up+128+90, (char *)string, 16, FALSE);
	
	// Finally, we render the frame
	_render_screen();
	
}

void UI_switch_state(I8U state, I32U interval)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	if (u->state == UI_STATE_SOS)
		return;
	
	u->state_init = TRUE;
	u->state = state;
	u->frame_interval = interval;
	u->display_to_base = CLK_get_system_time();
	
	N_SPRINTF("[UI] state switching: %d, %d", state, u->display_to_base);
}

BOOLEAN UI_is_idle()
{
	return (cling.ui.state == UI_STATE_IDLE);
}



#define OVERALL_PANNING_FRAMES 10

const I8U panning_table[] = {
	16, 16, 16, 16, 16, 16, 16, 16, 0, 0, 10
};

static void _frame_buffer_move_forward(I8U offset)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

		p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memcpy(p0, p0+offset, len); p0 += 128;
	}
	
		p0 = cling.ui.p_oled_up+len;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

static void _frame_buffer_move_backward(I8U offset)
{
	I8U *p0;
	I8U len = 128 - offset;
	int i;

	p0 = cling.ui.p_oled_up;

	for (i = 0; i < 4; i++) {
		memcpy(p0+offset, p0, len); p0 += 128;
	}
	
	p0 = cling.ui.p_oled_up;
	
	for (i = 0; i < 4; i++) {
		memset(p0, 0, offset); p0 += 128;
	}
}

const I8U iris_tab[4][12] = {
 {128,0,0,
	48,32,48,
	48,32,48,
	128,0,0},
 {128,0,0,
	32,64,32,
	32,64,32,
	128,0,0},
 {16,96,16,
	16,96,16,
	16,96,16,
	16,96,16},
 {8,112,8,
	8,112,8,
	8,112,8,
	8,112,8},
};

static void _iris_frame_core(I8U offset)
{
	I8U *p0;
	I8U i;
	I8U len, index;
	const I8U *p_tab;
	
	p0 = cling.ui.p_oled_up;

	if (offset == 1) {
		memset(p0, 0, 512);
		return;
	} else if (offset == 5) {
		return;
	}
	
	p_tab = iris_tab[offset-2];
	for (i = 0; i < 4; i++) {
		len = *p_tab++; // reset  
		index = len;
		memset(p0,0,len);
		
		index += *p_tab++;  // Keep value
		
		len = *p_tab++; // reset
		memset(p0+index,0,len); //
		p0 += 128;
	}
	
}

static BOOLEAN _frame_panning(I8U dir, BOOLEAN b_out)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U shift_pos = 0;
	I8U i;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= panning_table[OVERALL_PANNING_FRAMES]) {
		u->animation_index = 0;
		return TRUE;
	}

	// Shift screen
	if (b_out) {
		// Panning out current frame
		_display_frame_appear(u->frame_prev_idx, FALSE);
		
		for (i = 0; i < u->animation_index; i++) {
			shift_pos += panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_forward(shift_pos);
		} else {
			_frame_buffer_move_backward(shift_pos);
		}
	} else {
		// Panning in next frame
		_display_frame_appear(u->frame_next_idx, FALSE);
		shift_pos = 128;
		for (i = 0; i < u->animation_index; i++) {
			shift_pos -= panning_table[i];
		}
		
		if (dir == TRANSITION_DIR_LEFT) {
			_frame_buffer_move_backward(shift_pos);
		} else {
			_frame_buffer_move_forward(shift_pos);
		}
	}
	
	// update frame buffer and render it
	_render_screen();
	
	return FALSE;
}

static BOOLEAN _frame_iris()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= 6) {
		u->animation_index = 0;
		return TRUE;
	}

	// Iris  in next frame
	_display_frame_appear(u->frame_next_idx, FALSE);
	
	//
	_iris_frame_core(u->animation_index);
		
	
	// update frame buffer and render it
	_render_screen();
	
	return FALSE;
}

static void _frame_buffer_move_upward(I8U offset)
{
	I8U *p0, *p1;
	I8U i;
	I8U len = 4 - offset;

	p0 = cling.ui.p_oled_up;
	p1 = p0+128*offset; // Get offset of sourcing buffer

	for (i = 0; i < len; i++) {
		memcpy(p0, p1, 128);
		p0 += 128;
		p1 += 128;
	}
	for (; i < 4; i++) {
		memset(p0, 0, 128);
		p0 += 128;
	}
}

static void _frame_buffer_move_downward(I8U offset)
{
	I8U *p0, *p1;
	I8U i;
	I8U len = 4 - offset;

	p0 = cling.ui.p_oled_up+128*3;
	p1 = p0-128*offset; // Get offset of sourcing buffer

	for (i = 0; i < len; i++) {
		memcpy(p0, p1, 128);
		p0 -= 128;
		p1 -= 128;
	}
	for (; i < 4; i++) {
		memset(p0, 0, 128);
		p0 -= 128;
	}
}

static BOOLEAN _frame_tilting(I8U dir, BOOLEAN b_out)
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I8U shift_pos = 0;
	
	// Update animation index
	u->animation_index ++;
	
	if (u->animation_index >= 4) {
		u->animation_index = 0;
		return TRUE;
	}

	// Shift screen
	if (b_out) {
		// Panning out current frame
		_display_frame_appear(u->frame_prev_idx, FALSE);
		
		shift_pos = u->animation_index;
		
		if (dir == TRANSITION_DIR_UP) {
			_frame_buffer_move_upward(shift_pos);
		} else {
			_frame_buffer_move_downward(shift_pos);
		}
	} else {
		// Panning in next frame
		_display_frame_appear(u->frame_next_idx, FALSE);
		
		shift_pos = 4 - u->animation_index;
		
		if (dir == TRANSITION_DIR_UP) {
			_frame_buffer_move_downward(shift_pos);
		} else {
			_frame_buffer_move_upward(shift_pos);
		}
	}
	
	// update frame buffer and render it
	_render_screen();
	
	return FALSE;
}

static void _frame_animating()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	N_SPRINTF("[UI] animating: %d, %d, %d, %d, %d", u->animation_mode, u->animation_index, u->frame_prev_idx,
	u->frame_index, u->frame_next_idx);
	
	switch (u->animation_mode) {
		case ANIMATION_PANNING_OUT:
			if (_frame_panning(u->direction, TRUE)) {
				u->animation_mode = ANIMATION_PANNING_IN;
			}
			break;
		case ANIMATION_PANNING_IN:
			//if (_frame_panning(u->direction, FALSE)) {
			_frame_panning(u->direction, FALSE);
			{
				UI_switch_state(UI_STATE_APPEAR, 0);
			}
			break;
		case ANIMATION_TILTING_OUT:
			if (_frame_tilting(u->direction, TRUE)) {
				u->animation_mode = ANIMATION_TILTING_IN;
			}
			break;
		case ANIMATION_TILTING_IN:
			//if (_frame_tilting(u->direction, FALSE)) {
			_frame_tilting(u->direction, FALSE);
			{
				u->state = UI_STATE_APPEAR;
			}
			break;
		case ANIMATION_IRIS:
			if (_frame_iris()) {
				UI_switch_state(UI_STATE_APPEAR, 0);
			}
			break;
		default:
			break;
	}
}

static void _restore_ui_index()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	
	if ((u->frame_index >= UI_DISPLAY_WORKOUT) && (u->frame_index <= UI_DISPLAY_WORKOUT_END)) {
		u->frame_index = UI_DISPLAY_HOME;
		return;
	}
	
	if ((u->frame_index >= UI_DISPLAY_CAROUSEL) && (u->frame_index <= UI_DISPLAY_CAROUSEL_END)) {
		u->frame_index = UI_DISPLAY_HOME;
		return;
	}
	
	if (u->frame_cached_index == UI_DISPLAY_STOPWATCH_START) {
		u->frame_index = UI_DISPLAY_HOME;
		return;
	}
	
	if (u->frame_cached_index == UI_DISPLAY_SMART_PHONE_FINDER) {
		u->frame_index = UI_DISPLAY_HOME;
		return;
	}
	
	if ((u->frame_cached_index == UI_DISPLAY_SMART_INCOMING_MESSAGE) ||
			(u->frame_cached_index == UI_DISPLAY_SMART_MESSAGE) ||
			(u->frame_cached_index == UI_DISPLAY_SMART_APP_NOTIF) ||
			(u->frame_cached_index == UI_DISPLAY_SMART_DETAIL_NOTIF))
	{
		// Make sure it is activity workout alive
		if (!cling.activity.b_workout_active)
			cling.activity.workout_type = WORKOUT_NONE;
		
		if (cling.activity.workout_type > WORKOUT_NONE) {
			u->frame_cached_index = UI_DISPLAY_STOPWATCH_STOP;
		} else {
			u->frame_cached_index = UI_DISPLAY_SMART_MESSAGE;
		}
	}

	// If it is charging
	
	if (BATT_is_charging()) {
		if ((u->frame_index < UI_DISPLAY_STOPWATCH_STOP) || (u->frame_index > UI_DISPLAY_STOPWATCH_CALORIES)) {
			u->frame_index = UI_DISPLAY_HOME;
			return;
		}
	}
	
	// Go back to previous UI page
	u->frame_index = UI_DISPLAY_PREVIOUS;
	
}

static BOOLEAN _ui_frame_blinking()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	if ((u->frame_index >= UI_DISPLAY_SMART) && (u->frame_index <= UI_DISPLAY_SMART_END))
		return FALSE;
	else 
		return TRUE;
}

#if 0
static I8U uistate;
static I8U ui_frame_idx;
static BOOLEAN uiactive;
#endif
extern I32U sim_started;

void UI_state_machine()
{
	UI_ANIMATION_CTX *u = &cling.ui;
	I32U t_curr, t_diff, t_threshold;
	
	
	t_curr = CLK_get_system_time();
	#if 0
	if (uiactive != u->display_active) {
		Y_SPRINTF("[UI] new active state: %d @ %d", u->state, CLK_get_system_time());
		uiactive = u->display_active;
	}
	#endif
	if (!u->display_active)
		return;
	#if 0
	if (uistate != u->state) {
		Y_SPRINTF("[UI] new state: %d, %d @ %d", uistate, u->state, CLK_get_system_time());
		uistate = u->state;
	}
	
	if (ui_frame_idx != u->frame_index) {
		Y_SPRINTF("[UI] new index: %d (%d)", u->frame_index, ui_frame_idx);
		ui_frame_idx = u->frame_index;
	}
	#endif
	switch (u->state) {
		case UI_STATE_IDLE:
			break;
		case UI_STATE_CLING_START:
			if (u->state_init) {
				u->state_init = FALSE;
				_render_logo();
			} else if (t_curr > u->display_to_base + u->frame_interval) {
				UI_switch_state(UI_STATE_HOME, 1000);
			}
			break;
		case UI_STATE_CLOCK_GLANCE:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				if (BATT_is_charging() || BATT_is_low_battery()) {
					Y_SPRINTF("[UI] LOW BATTERY(or charging): %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
					UI_switch_state(UI_STATE_LOW_POWER, 1000);
				} else if (!LINK_is_authorized()) {
					u->prefer_state = UI_STATE_AUTHORIZATION;
					
					UI_switch_state(UI_STATE_AUTHORIZATION, 1000);
				} else {
					_display_frame_home(TRUE);
				}
				N_SPRINTF("[UI] clock glance");
			} else if ((t_curr > u->display_to_base + 1000) || TOUCH_new_gesture()) {
				N_SPRINTF("[UI] switch to sensing mode, %d, %d", t_curr, u->display_to_base);
				UI_switch_state(UI_STATE_TOUCH_SENSING, 2000);
				u->touch_time_stamp = t_curr;
			}
			break;
		}
		case UI_STATE_HOME:
			if (u->state_init) {
				u->state_init = FALSE;
				if (LINK_is_authorized()) {

					if (BATT_is_low_battery())
					{
						Y_SPRINTF("[UI] LOW BATTERY: %d", cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
						u->state = UI_STATE_LOW_POWER;
					} else {
						UI_switch_state(UI_STATE_APPEAR, 1000);
					}
					// Let's give UI screen about 4 more seconds once we have clock shown up
					// Just to be nice!
					u->touch_time_stamp = t_curr;
				} else {
					u->prefer_state = UI_STATE_AUTHORIZATION;
					
					UI_switch_state(UI_STATE_AUTHORIZATION, 1000);
				}
			}
			break;
		case UI_STATE_AUTHORIZATION:
			if (u->state_init) {
				u->state_init = FALSE;
				_display_unauthorized_home();
				
				// Link re-initialization
				N_SPRINTF("[UI] Link status re-initialization");
			} else if (t_curr > u->display_to_base + u->frame_interval) {
				u->display_to_base = t_curr;
				N_SPRINTF("[UI] display to at authorization: %d", u->display_to_base);
				u->frame_interval = 1000;
				if (LINK_is_authorizing()) {
					t_diff = t_curr - cling.link.link_ts;
					if (t_diff < 60000) {
						_display_authenticating();
					} else {
						cling.link.b_authorizing = FALSE;
					}
				} else {
					if (OTA_if_enabled()) {
						u->frame_index = UI_DISPLAY_OTA;
						UI_switch_state(UI_STATE_APPEAR, 1000);
					} else {
						UI_switch_state(UI_STATE_HOME, 0);
					}
				}
			}
			break;
		case UI_STATE_REMINDER:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				u->frame_index = UI_DISPLAY_SMART_REMINDER;
				_display_frame_smart(u->frame_index, TRUE);
			} else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					if (LINK_is_authorized()) {
						u->frame_index = UI_DISPLAY_PREVIOUS;
						UI_switch_state(UI_STATE_TOUCH_SENSING, 4000);
					} else {
						UI_switch_state(UI_STATE_HOME, 4000);
					}
				}
			}
			break;
		}
		case UI_STATE_NOTIFIC:
		{
#ifndef _CLING_PC_SIMULATION_
			if (u->state_init) {
				u->state_init = FALSE;
#ifdef _ENABLE_ANCS_
				if (cling.notific.cat_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL)
					u->frame_index = UI_DISPLAY_SMART_INCOMING_CALL;
				else
					u->frame_index = UI_DISPLAY_SMART_INCOMING_MESSAGE;
#endif								
				// Go display incoming message/call
				_display_frame_smart(u->frame_index, TRUE);
			} else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					if (LINK_is_authorized()) {
						UI_switch_state(UI_STATE_TOUCH_SENSING, 4000);
					} else {
						UI_switch_state(UI_STATE_HOME, 4000);
					}
				}
			}
#endif
			break;
		}
		case UI_STATE_LOW_POWER:
		{
			if (u->state_init) {
				Y_SPRINTF("[UI] low power (or charging) shown");
				u->state_init = FALSE;
				_display_charging(cling.system.mcu_reg[REGISTER_MCU_BATTERY]);
				u->touch_time_stamp = t_curr;
			}
			else {
				if (t_curr > (u->touch_time_stamp+2000)) {
					UI_switch_state(UI_STATE_APPEAR, 1000);
				}
			}
			break;
		}
		case UI_STATE_IDLE_ALERT:
		{
			if (u->state_init) {
				Y_SPRINTF("[UI] idle alert shown");
				u->state_init = FALSE;
				_display_idle_alert();
				u->touch_time_stamp = t_curr;
			}
			else {
				if (t_curr > (u->touch_time_stamp+4000)) {
					u->state = UI_STATE_DARK;
					Y_SPRINTF("[UI] Idle alert is ended");
				}
			}
			break;
		}
		case UI_STATE_CHARGING:
		{
			if (u->state_init) {
				u->state_init = FALSE;
				UI_switch_state(UI_STATE_HOME, 0);
			}
			else {
				if (t_curr > u->display_to_base + u->frame_interval) {
					
					//if (LINK_is_authorized())
						//UI_switch_state(u->prefer_state, 4000);
					//else
						UI_switch_state(UI_STATE_HOME, 4000);
				}
			}
			break;
		}
		case UI_STATE_TOUCH_SENSING:
			// Don't do anything if system is updating firmware
			if (OTA_if_enabled()) {
				
				// Skip all the gestures
				u->frame_index = UI_DISPLAY_OTA;
				UI_switch_state(UI_STATE_APPEAR, 1000);
#if 0
				if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] > 90) 
#endif
				{
					u->touch_time_stamp = t_curr;
				}
#ifndef USING_VIRTUAL_ACTIVITY_SIM
			} else if (!LINK_is_authorized()) {
				// Go to home page if device is unauthorized.
				UI_switch_state(UI_STATE_HOME, 4000);
#endif
			} else if (LINK_is_authorizing()) {
					u->prefer_state = UI_STATE_AUTHORIZATION;
					
					UI_switch_state(UI_STATE_AUTHORIZATION, 1000);
			} else if (_ui_touch_sensing())  {
				N_SPRINTF("[UI] new gesture --- %d", t_curr);
				u->touch_time_stamp = t_curr;
			} else if (_ui_frame_blinking()) {
				// Otherwise, blinking every 800 ms.
				if (t_curr > u->display_to_base + u->frame_interval) {
					N_SPRINTF("[UI] Go blinking the icon");
					UI_switch_state(UI_STATE_APPEAR, 400);
				}
			}
			
#ifndef USING_VIRTUAL_ACTIVITY_SIM
			
			// 4 seconds screen dark expiration
			// for heart rate measuring, double the Screen ON time.
			if (u->true_display)
				t_threshold = cling.user_data.screen_on_general; // in second
			else
				t_threshold = 2;
			t_threshold *= 1000; // second -> milli-second
			if (u->frame_index == UI_DISPLAY_VITAL_HEART_RATE) {
				if (TOUCH_is_skin_touched()) {
					t_threshold = cling.user_data.screen_on_heart_rate; // in second
					t_threshold *= 1000; // second -> milli-second
				}
			} else if (u->frame_index == UI_DISPLAY_TRACKER_UV_IDX) {
				t_threshold += 5000;
			}
			
			// If we don't see any gesture in 4 seconds, dark out screen
			if (t_curr > (u->touch_time_stamp+t_threshold)) {
				N_SPRINTF("[UI] gesture monitor time out - %d", t_threshold);
				if (BATT_is_charging()) {
					if (t_curr > (u->touch_time_stamp+10000)) {
						u->state = UI_STATE_DARK;
						TRACKING_enter_low_power_mode();
					}
				} else {
					u->state = UI_STATE_DARK;
				}
			}
#endif
			break;
		case UI_STATE_ANIMATING:
			// perform frame animation before a final apperance
			_frame_animating();
			break;
		case UI_STATE_APPEAR:
			if (u->state_init) {
				N_SPRINTF("[UI] time: %d, index: %d", t_curr, u->frame_index);
				u->state_init = FALSE;
				if (u->frame_index == UI_DISPLAY_PREVIOUS) {
					u->frame_index = u->frame_cached_index;
					Y_SPRINTF("[UI] load cached index: %d", u->frame_index);
				}
				// Display this frame.
				_display_frame_appear(u->frame_index, TRUE);
			} else {
				UI_switch_state(UI_STATE_TOUCH_SENSING, 500);
			}
			break;
		case UI_STATE_SOS:
		{
			if (u->state_init) {
				Y_SPRINTF("[UI] SOS is pressed");
				u->state_init = FALSE;
				_display_sos(TRUE);
				u->touch_time_stamp = t_curr;
				u->frame_index = UI_DISPLAY_HOME;
				u->frame_cached_index = UI_DISPLAY_HOME;
			}
			else {
				if (t_curr > (u->touch_time_stamp+10000)) {
					u->state = UI_STATE_DARK;
					Y_SPRINTF("[UI] SOS is ended");
				}
			}
			break;
		}
		case UI_STATE_DARK:
		{
			if (t_curr > (u->display_to_base+u->frame_interval)) {
				Y_SPRINTF("[UI] screen go dark");
				// Turn off OLED panel
				OLED_set_panel_off();
				u->state = UI_STATE_IDLE;
				u->level_1_index = 0;
				u->true_display = FALSE;
				u->b_detail_page = FALSE;
				
				// Reset UV index
				cling.uv.max_UI_uv = 0;
				
				// Reset alarm clock flag
				cling.reminder.ui_alarm_on = FALSE;

				// Reset workout type
				UI_reset_workout_mode();
				
				// Remember the last screen so that 
				/*
				if (u->frame_cached_index == UI_DISPLAY_VITAL_HEART_RATE)
				{
					u->frame_index = UI_DISPLAY_CAROUSEL_2;
				} else 
				*/
				_restore_ui_index();
				
				if (cling.lps.b_low_power_mode) {
					TOUCH_power_set(TOUCH_POWER_DEEP_SLEEP);
				} else {
					TOUCH_power_set(TOUCH_POWER_HIGH_20MS);
				}
			}
			break;
		}
		default:
			break;
	}
}

void UI_turn_on_display(UI_ANIMATION_STATE state, I32U time_offset)
{
	// Turn on OLED panel
	OLED_set_panel_on();
	
	// touch time update
	cling.ui.touch_time_stamp = CLK_get_system_time()-time_offset;
	cling.ui.true_display = TRUE;

	if (state != UI_STATE_IDLE) {
		UI_switch_state(state, 0);
	} else {
		if (UI_is_idle()) {
			// UI initial state, A glance of current time
			UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
		}
	}
}

void UI_reset_workout_mode()
{
	if (cling.ui.frame_index < UI_DISPLAY_STOPWATCH_STOP) {
		//
		if (cling.ui.frame_index < UI_DISPLAY_SMART) {
			cling.activity.workout_type = WORKOUT_NONE;
			cling.activity.workout_place = WORKOUT_PLACE_NONE;
			cling.activity.b_workout_active = FALSE;
		} else if (cling.ui.frame_index > UI_DISPLAY_SMART_END) {
			cling.activity.workout_type = WORKOUT_NONE;
			cling.activity.workout_place = WORKOUT_PLACE_NONE;
			cling.activity.b_workout_active = FALSE;
		}
	}
	
	if (cling.ui.frame_index > UI_DISPLAY_STOPWATCH_CALORIES) {
		cling.activity.workout_type = WORKOUT_NONE;
		cling.activity.workout_place = WORKOUT_PLACE_NONE;
		cling.activity.b_workout_active = FALSE;
	}
}


