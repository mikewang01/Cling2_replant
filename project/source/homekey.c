/***************************************************************************//**
 * 
 * File: homekey.c
 *
 * Description: Home key basics - click, (double click, click and hold)
 *
 * Created on April/29/2015
 * 
 *****************************************************************************/

#include "main.h"
#include "homekey.h"

void HOMEKEY_click_init()
{
#ifndef _CLING_PC_SIMULATION_
	I8U b_pin;
	I8U stat = ON_CLICK;
	HOMEKEY_CLICK_STAT *k = &cling.key;

	b_pin = nrf_gpio_pin_read(GPIO_HOMEKEY);	

	// button released
	if (b_pin) stat = OFF_CLICK;

	k->stable_st = stat;
	k->temp_st = stat;
	k->ticks[stat] = CLK_get_system_time();
#endif
}

void HOMEKEY_check_on_hook_change()
{
#ifndef _CLING_PC_SIMULATION_
	HOMEKEY_CLICK_STAT *k = &cling.key;
	I32U t_curr;
	I8U b_pin;
	I8U stat = ON_CLICK;
	
	// Get a temporarily BUTTON status
	b_pin = nrf_gpio_pin_read(GPIO_HOMEKEY);	

	if (b_pin) {
		stat = OFF_CLICK;
	} 

	if (stat != k->temp_st) {

		t_curr = CLK_get_system_time();
		
		// update the click time stamp
		k->temp_st = stat;
		k->ticks[stat] = t_curr;
		Y_SPRINTF("[HOMEKEY] --- BUTTON Event at %d---(%d)", t_curr, stat);

		if (k->temp_st == ON_CLICK) {
		
			// Make sure OLED display panel is faced up.
			if (LINK_is_authorized()) {
					
				Y_SPRINTF("[TOUCH] ------------ TURN ON SCREEN --------");

				// Turn on OLED panel
				OLED_set_panel_on();
				
				cling.ui.true_display = TRUE;
				
				// Update touch event time
				cling.ui.touch_time_stamp = CLK_get_system_time();
				
				if (UI_is_idle()) {
					// UI initial state, a glance of current clock
					UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
					Y_SPRINTF("[KEY] set UI: button clicked (glance)");
				} else {
					// 
					UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
					Y_SPRINTF("[KEY] set UI: button clicked (sensing)");
				}
			} else {
				RTC_start_operation_clk();
			}
		} 
		return;
	} else {
		return;
	}
#else
	return;
#endif
}

/****************************************************************************
    Check button status for on-the-fly events.
****************************************************************************/
void HOMEKEY_click_check()
{
	HOMEKEY_CLICK_STAT *k = &cling.key;
	I32U t_curr=0;
	I32U offset = 0;
	I32U t_click_and_hold = CLICK_HOLD_TIME_LONG;
	I32U t_sos = CLICK_HOLD_TIME_SOS;

	// only update hook state machine when there is possible state switch 
	if (k->stable_st == k->temp_st) {
		
		// If it is press and hold, go return
		if (k->temp_st == ON_CLICK) {
			
			// Ignore SOS request if device is not authorized.
			if (!LINK_is_authorized())
				return;
			
			// Go ahead to check if SOS is pressed
			
			offset = CLK_get_system_time() - k->ticks[ON_CLICK];
			
			if (offset >= t_sos) {
				
				k->ticks[ON_CLICK] += 2000;
			
				// Increase the counter for "press + hold"
				k->click_sos_num ++;
				
				cling.touch.b_valid_gesture = TRUE;
				cling.touch.gesture = TOUCH_BUTTON_PRESS_SOS;
				Y_SPRINTF("[HOMEKEY] +++ button (SOS), %d, %d, %d", t_curr, offset, t_sos);
				
				// Send SOS message to the App
				CP_create_sos_msg();
			}
			
			return;
		}
	} 

	// Handle the debouncing of the click status change
	if (k->temp_st == ON_CLICK) {

  		// Check how long we have been in "ON-CLICK" status
		offset = CLK_get_system_time() - k->ticks[ON_CLICK];
		
		// Debouncing
		if (offset < HOMEKEY_CLICK_DEBOUNCE) 
			return;

		if (offset >= t_click_and_hold) { 
			// announce a stable state --> ON_CLICK
			k->stable_st = ON_CLICK;
			// clear possible hook flash flag
			k->half_click = 0;

			// Increase the counter for "press + hold"
			k->click_hold_num ++;

			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_PRESS_HOLD;
			Y_SPRINTF("[HOMEKEY] +++ button press and hold: %d, %d, %d, %d", offset, t_curr, k->ticks[ON_CLICK], CLK_get_system_time());
			
			// if unauthorized, we should turn off screen, or turn it on
			if (!cling.system.b_powered_up) {
				SYSTEM_restart_from_reset_vector();
			} else {
				cling.batt.shut_down_time = BATTERY_SYSTEM_UNAUTH_POWER_DOWN;
			}
		} else {
			// indicate a possible double click, no state update just yet!
			k->half_click = 1;

		} 
	} else {
		// Add debounce logic to prevent multiple button click
		offset = t_curr - k->ticks[OFF_CLICK];
		if (offset < HOMEKEY_CLICK_DEBOUNCE) 
			return;

		offset = k->ticks[OFF_CLICK] - k->ticks[ON_CLICK];

		// if there is half click flag, we are good to declare a single click event
		if(k->half_click && (offset < t_click_and_hold)) {
			k->half_click = 0;

			k->single_click_num ++;
			
			cling.touch.b_valid_gesture = TRUE;
			cling.touch.gesture = TOUCH_BUTTON_SINGLE;
			Y_SPRINTF("[HOMEKEY] +++ button single click");
		}
		else {
			// declare a new state --> OFF HOOK
			k->stable_st = OFF_CLICK;
			k->half_click = 0; 
		}
	}
}
#if 0
I32U time_key=0, sim_idx = 0, sim_started = 0;
const I8U sim_key_tab[30] = {	
	TOUCH_BUTTON_PRESS_HOLD,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_SWIPE_LEFT, // stop -> hr
	TOUCH_SWIPE_LEFT,
	TOUCH_SWIPE_LEFT,
	TOUCH_SWIPE_LEFT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_FINGER_RIGHT,
	TOUCH_SWIPE_RIGHT,
	TOUCH_SWIPE_RIGHT,
	TOUCH_SWIPE_RIGHT,
	TOUCH_SWIPE_RIGHT,
	TOUCH_SWIPE_RIGHT,
};

const I8U sim_time_gap[30] = {	
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,      // Should get to stop watch stop page
	3,
	2,
	2,
	2,
	2,
	2,
	2,
	3,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
};

void HOMEKEY_sim_kickoff()
{
	sim_started = 1;
	time_key = 0;
	cling.time.system_clock_in_sec = 0;
}

static void _sim_physical_touch()
{
		// Turn on OLED panel
		OLED_set_panel_on();
		
		cling.ui.true_display = TRUE;
		
		// Update touch event time
		cling.ui.touch_time_stamp = CLK_get_system_time();
		
		if (UI_is_idle()) {
			// UI initial state, a glance of current clock
			UI_switch_state(UI_STATE_CLOCK_GLANCE, 0);
			Y_SPRINTF("[KEY] set UI: sim click (glance)");
		} else {
			// 
			UI_switch_state(UI_STATE_TOUCH_SENSING, 0);
			Y_SPRINTF("[KEY] set UI: sim clicked (sensing)");
		}
}

void HOMEKEY_sim()
{
	I32U t_gap;
	#if 1
	if (!sim_started)
		return;
	#endif
	t_gap = time_key + sim_time_gap[sim_idx];
	
	if (cling.time.system_clock_in_sec > t_gap) {
		
		Y_SPRINTF("[HOMEKEY] index: %d, key: %d", sim_idx, cling.touch.gesture);
		_sim_physical_touch();
		
		time_key = cling.time.system_clock_in_sec;
		
		cling.touch.b_valid_gesture = TRUE;
		cling.touch.gesture = sim_key_tab[sim_idx++];
		
		if (sim_idx >= 18)
			sim_idx  = 0;
		
	}
}
#endif
