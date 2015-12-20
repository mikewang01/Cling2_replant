/***************************************************************************//**
 * @file RTC_MSP430.c
 * @brief Driver implementation for the MSP430 real-time clock.
 *
 *
 ******************************************************************************/


#include "standards.h"
#include "main.h"


#define APP_TIMER_MAX_TIMERS                 4                                          /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE              4                                          /**< Size of timer operation queues. */

#ifndef _CLING_PC_SIMULATION_
APP_TIMER_DEF(m_rtc_timer_id); /**< 1 sec based timer. >**/
APP_TIMER_DEF(m_operation_timer_id); /**< 1 sec based timer. >**/
#endif

const I8U DAYS_IN_EACH_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void OPERATION_timer_handler( void * p_context )
{
    N_SPRINTF("[RTC] OPERATION 50ms timer handler func");

    // Timer update
    cling.hr.sample_ready = TRUE;
}

void RTC_timer_handler( void * p_context )
{
	I32U tick_now;
	I32U tick_diff;
	I32U tick_in_s;

#ifndef _CLING_PC_SIMULATION_	
	if (cling.time.tick_count) {		
		app_timer_cnt_get(&tick_now);
		app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
		tick_in_s = (tick_diff>>15);
		tick_diff = (tick_in_s << 15);
		cling.time.tick_count += tick_diff;
		cling.time.tick_count &= MAX_RTC_CNT;
	} else {	
		app_timer_cnt_get(&cling.time.tick_count);
		tick_in_s = (cling.time.tick_count>>15);
	}
#else
	tick_in_s = 1; // 1 second timer
#endif
	
	// update battery measuring timer
	cling.batt.level_update_timebase += tick_in_s;
	BATT_update_charging_time(tick_in_s);

	// update radio duty cycling
	cling.time.system_clock_in_sec += tick_in_s;
	cling.time.time_since_1970 += tick_in_s;
	if (cling.batt.state_switching_duration < 128)
		cling.batt.state_switching_duration += tick_in_s;
	if (cling.batt.shut_down_time < BATTERY_SYSTEM_UNAUTH_POWER_DOWN) {
		cling.batt.shut_down_time += tick_in_s;
	}
	
	// Accumulate skin touch time
	if (TOUCH_is_skin_touched()) {
		if (cling.touch.skin_touch_time_per_minute < 60) {
			cling.touch.skin_touch_time_per_minute += tick_in_s;
			N_SPRINTF("[RTC] skint touch time per min: %d", cling.touch.skin_touch_time_per_minute);
		}
	}	
#ifdef USING_VIRTUAL_ACTIVITY_SIM
	 if (OTA_if_enabled()) {
		 cling.ota.percent ++;
		 if (cling.ota.percent >= 100) {
			 cling.ota.percent = 100;
		 }
	 }
#endif

	// Indicates a second-based RTC interrupt
	RTC_get_local_clock(cling.time.time_since_1970, &cling.time.local);

	// Check if we have minute passed by, or 
	if (cling.time.local.minute != cling.time.local_minute) {
		cling.time.local_minute_updated = TRUE;
		cling.time.local_minute = cling.time.local.minute;
		if (
			   (cling.user_data.idle_time_in_minutes>0) && 
				 (cling.time.local.hour>=cling.user_data.idle_time_start) && 
				 (cling.time.local.hour< cling.user_data.idle_time_end)
			 )
    {
			cling.user_data.idle_minutes_countdown --;
		}
		N_SPRINTF("[RTC] min updated (%d)", cling.activity.day.walking);
	}	
	
	if (cling.time.local.day != cling.time.local_day) {
		cling.time.local_day_updated = TRUE;
		cling.time.local_day = cling.time.local.day;
		
		// Reset reminder
		cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;

		Y_SPRINTF("[RTC] local day updated");
	}

	// Testing, assuming user sleeps around 22:00 at night
	if (cling.time.local_hour != cling.time.local.hour) {
		if (cling.time.local.hour == 12) {
			cling.time.local_noon_updated = TRUE;
		}
		cling.time.local_hour = cling.time.local.hour;
	}
}

// RTC is set to Calendar mode
EN_STATUSCODE RTC_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	uint32_t err_code;
	
	// Initialize timer module.
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

	err_code = app_timer_create(&m_rtc_timer_id,
															APP_TIMER_MODE_REPEATED,
															RTC_timer_handler);

	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_create(&m_operation_timer_id,
															APP_TIMER_MODE_REPEATED,
															OPERATION_timer_handler);

	APP_ERROR_CHECK(err_code);
#endif
  return STATUSCODE_SUCCESS;
}

EN_STATUSCODE RTC_Start(void)
{
#ifndef _CLING_PC_SIMULATION_

    app_timer_start(m_rtc_timer_id, SYSCLK_INTERVAL_2000MS, NULL);

    RTC_start_operation_clk();

    N_SPRINTF("[RTS] reset tick count, start 20 ms sysclock");
#endif
    return STATUSCODE_SUCCESS;
}

//
// We might not need RTC stop as RTC runs all the time
//
void RTC_stop_operation_clk(void)
{
#ifndef _CLING_PC_SIMULATION_
    I32U t_curr = CLK_get_system_time();

    N_SPRINTF("[RTC] status: %d, %d, %d", cling.time.sysclk_interval, cling.time.sysclk_config_timestamp, t_curr);

    if (t_curr > (cling.time.operation_clk_start_in_ms + OPERATION_CLK_EXPIRATION)) {

        if (cling.time.operation_clk_enabled) {
            Y_SPRINTF("[SYSCLK] OPERATION clk stop, %d @ %d ", t_curr, cling.time.operation_clk_start_in_ms);
            cling.time.operation_clk_enabled = FALSE;
            app_timer_stop(m_operation_timer_id);
        }
    }
#endif
}

void RTC_start_operation_clk()
{
    I32U err_code;
#ifndef _CLING_PC_SIMULATION_

    cling.time.operation_clk_start_in_ms = CLK_get_system_time();

    if (!cling.time.operation_clk_enabled) {
        Y_SPRINTF("[SYSCLK] OPERATION clk start, %d ", cling.time.operation_clk_start_in_ms);
        cling.time.operation_clk_enabled = TRUE;
        err_code = app_timer_start(m_operation_timer_id, SYSCLK_INTERVAL_20MS, NULL);
        APP_ERROR_CHECK(err_code);
    }
#endif
}

void RTC_system_shutdown_timer()
{
#ifndef _CLING_PC_SIMULATION_
    app_timer_stop(m_rtc_timer_id);
    app_timer_start(m_rtc_timer_id, SYSCLK_INTERVAL_6000MS, NULL);
#endif
}


void RTC_get_delta_clock_forward(SYSTIME_CTX *delta, I8U offset)
{
    I32U epoch = cling.time.time_since_1970 + offset * EPOCH_DAY_SECOND;
    I16S time_diff_in_minute = cling.time.time_zone;
    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch += time_diff_in_minute;

    RTC_get_regular_time(epoch, delta);
}

void RTC_get_delta_clock_backward(SYSTIME_CTX *delta, I8U offset)
{
    I32U epoch = cling.time.time_since_1970 - offset * EPOCH_DAY_SECOND;
    I16S time_diff_in_minute = cling.time.time_zone;
    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch += time_diff_in_minute;

    RTC_get_regular_time(epoch, delta);
}

void RTC_get_local_clock(I32U epoch_start, SYSTIME_CTX *local)
{
    I32U epoch;
    I32S time_diff_in_minute = cling.time.time_zone;

    time_diff_in_minute *= TIMEZONE_DIFF_UNIT_IN_SECONDS;
    epoch = epoch_start + time_diff_in_minute;

    RTC_get_regular_time(epoch, local);

}

I8U const month_leap_in_days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
I8U const month_normal_in_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void RTC_get_regular_time(I32U epoch, SYSTIME_CTX *t)
{
    I8U i;
    I32U s;
    I32U tick = epoch;
    I32U dayno = tick / EPOCH_DAY_SECOND;
    I32U dayclock = tick - dayno * EPOCH_DAY_SECOND;

    t->year = 1970;
    t->month = 1;
    t->day = 1;
    t->hour = dayclock / 3600;
    t->minute = (dayclock - t->hour * 3600) / 60;
    t->second = dayclock % 60;
    t->dow = (dayno + 3) % 7;

    // Give a life span of 200 years
    s = 31556926;
    for (i = 0; i < 200; i++) {
        // First figure out the year
        if ((t->year & 0x03) == 0) {
            s = 366;
        } else {
            s = 365;
        }
        if (dayno < s) {
            break;
        }
        t->year++;
        dayno -= s;
    }

    // get the month
    if ((t->year & 0x03) == 0) {
        for (i = 0; i < 12; i++) {
            if (dayno < month_leap_in_days[i]) {
                break;
            }
            t->month ++;
            dayno -= month_leap_in_days[i];
        }
    } else {
        for (i = 0; i < 12; i++) {
            if (dayno < month_normal_in_days[i]) {
                break;
            }
            t->month ++;
            dayno -= month_normal_in_days[i];
        }
    }

    // get the day
    t->day += dayno;

}

I32U RTC_get_epoch_day_start(I32U past_days)
{
    I32U current_epoch = cling.time.time_since_1970;
    I32U offset = cling.time.local.second;

    offset += cling.time.local.minute * 60;
    offset += cling.time.local.hour * 3600;

    offset += past_days * EPOCH_DAY_SECOND;

    current_epoch -= offset;

    return current_epoch;
}
/* @} */
