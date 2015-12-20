/***************************************************************************//**
 * File: sysclk.c
 *
 * Description: System clock manager
 *
 *
 * Created on Feb 08, 2014
 *
 * 
 *****************************************************************************/

#include "main.h"

#include "standards.h"

#ifdef _CLING_PC_SIMULATION_
I32U sim_tick_acc = 0;
void SYSCLK_update_tick(void)
{
	sim_tick_acc += 20;
	if (sim_tick_acc > 1000)
		sim_tick_acc -= 1000;
}

#endif

/* Uses the 16-bit TimerA0 counter and the lower 16 bits of the system
 * clock to give a "fine-grained" time. */
I32U SYSCLK_GetFineTime(void)
{
	I32U tick_diff=0;
#ifndef _CLING_PC_SIMULATION_
	I32U tick_now;
	double system_clock_in_ms;

	// Get the elapsed time
	app_timer_cnt_get(&tick_now);
	app_timer_cnt_diff_compute(tick_now, cling.time.tick_count, &tick_diff);
	system_clock_in_ms = cling.time.system_clock_in_sec*1000;
	system_clock_in_ms += (tick_diff/32.768);

	N_SPRINTF("[SYSCLK]: sec: %d, n: %d, o:%d", cling.time.system_clock_in_sec, tick_now, cling.time.tick_count);
	// 32758 ticks per second, about 32 tick per ms.
	return (I32U)system_clock_in_ms;
#else
	tick_diff = sim_tick_acc;
	return 0;
#endif
}

/** @} */
