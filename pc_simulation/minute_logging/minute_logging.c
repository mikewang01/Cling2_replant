// minute_logging.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "main.h"
#include "fs_root.h"
#include "RTC.h"

CLING_MAIN_CTX cling;

char *p_flash;
FILE *fLog;

#ifdef _SLEEP_SIMULATION_
extern int LIS3DH_is_awake;
#endif

static void _file_testing_rd()
{
	char file_name[256];
	char output_fname[256];
	char tmp[255];
	int ret;
	CLING_FILE f;
	FILE *fp;
	int f_total, i;

	f_total = ROOT_ListFiles();

	if (!f_total)
		return;

	for (i = 0; i < f_total; i++) {

		// Generate file name ...
		ROOT_GetFileName(i, (I8U *)file_name);

		if ((f.fc = FILE_fopen((I8U *)file_name, FILE_IO_READ)) == 0)
			break;

		sprintf(output_fname, "D:\\my_cplusplus\\cling_simulation\\minute_logging\\output\\%s", file_name);
		fp = fopen(output_fname, "w+b");

		while (FILE_feof(f.fc)) {
			// read content out of a file
			ret = FILE_fread(f.fc, (I8U *)tmp, 200);
			fwrite(tmp, 1, ret, fp);
			if (ret != 200)
				break;
		}

		// close the file
		FILE_fclose(f.fc);

		fclose(fp);
	}

	printf("overall file read: %d\r\n", i);

	sprintf(output_fname, "D:\\my_cplusplus\\cling_simulation\\minute_logging\\output\\flash.bin");
	fp = fopen(output_fname, "w+b");
	fwrite(p_flash, 1, FLASH_SIZE, fp);
	fclose(fp);
	return;
}

static void _cling_global_init()
{
	// Clean the cling global structure
	memset(&cling, 0, sizeof(CLING_MAIN_CTX));

	// Initialize the low power variables 
	cling.lps.b_low_power_mode = FALSE;

	// Version initialization
	cling.system.mcu_reg[REGISTER_MCU_HW_REV] = 0; // Board version is yet to be supported, initialized to 0.

	// HW INFO
	cling.system.mcu_reg[REGISTER_MCU_HWINFO] = 1;
	// BATTERY LEVEL
	cling.system.mcu_reg[REGISTER_MCU_BATTERY] = 50;
	// Software version number
	cling.system.mcu_reg[REGISTER_MCU_REVH] = BUILD_MAJOR_RELEASE_NUMBER;  // SW VER (HI)
	cling.system.mcu_reg[REGISTER_MCU_REVH] <<= 4;
	cling.system.mcu_reg[REGISTER_MCU_REVH] |= (BUILD_MINOR_RELEASE_NUMBER >> 8) & 0x0f;
	cling.system.mcu_reg[REGISTER_MCU_REVL] = (I8U)(BUILD_MINOR_RELEASE_NUMBER & 0xff);  // SW VER (LO)

	// Restoring the time zone info
	// Shanghai: UTC +8 hours (units in 15 minutes) -> 8*(60/15) = 32 minutes
	cling.time.time_zone = 32;
	cling.time.time_since_1970 = 1400666400;
	RTC_get_local_clock(&cling.time.local);

	// Get current local minute
	cling.time.local_day = cling.time.local.day;
	cling.time.local_minute = cling.time.local.minute;

	// Cling reminder state machine
	cling.reminder.state = REMINDER_STATE_CHECK_NEXT_REMINDER;

}

static int _interpret_streaming_packet(char *buf)
{
	CP_TX_STREAMING_CTX *s = &cling.gcp.streaming;
	int mode = s->pkt[2];
	int repeat_num = 0;
	int epoch;
	int i;
	int len = 0;

	// Copy 16 bytes to the buffer for data interpretion later
	if (mode == CP_MSG_TYPE_STREAMING_MULTI_MINUTES) {
		repeat_num = s->pkt[3];
		repeat_num <<= 8;
		repeat_num |= s->pkt[4];

		epoch = s->pkt[5];
		epoch <<= 8;
		epoch += s->pkt[6];
		epoch <<= 8;
		epoch += s->pkt[7];
		epoch <<= 8;
		epoch += s->pkt[8];

		if (epoch == 1402294500) {
			Y_SPRINTF("...");
		}

		for (i = 0; i < repeat_num; i++) {
			s->pkt[5] = (I8U)((epoch >> 24) & 0xff);
			s->pkt[6] = (I8U)((epoch >> 16) & 0xff);
			s->pkt[7] = (I8U)((epoch >> 8) & 0xff);
			s->pkt[8] = (I8U)(epoch & 0xff);
			memcpy(buf + len, s->pkt + 5, 16);
			epoch += 60;
			len += 16;
		}
	}
	else if (mode == CP_MSG_TYPE_STREAMING_MINUTE) {
		epoch = s->pkt[5];
		epoch <<= 8;
		epoch += s->pkt[6];
		epoch <<= 8;
		epoch += s->pkt[7];
		epoch <<= 8;
		epoch += s->pkt[8];

		if (epoch == 1402294500) {
			Y_SPRINTF("...");
		}

		memcpy(buf + len, s->pkt + 5, 16);
		len += 16;
	}
	else if (mode == CP_MSG_TYPE_STREAMING_SECOND)
	{
		return len;
	}
	else {
		return len;
	}

	return len;
}

static void _ack_streaming_packet()
{
	CP_TX_STREAMING_CTX *s = &cling.gcp.streaming;

	s->pending = FALSE;
	s->packet_need_ack = FALSE;
	s->flag_entry_read = TRUE;
}

static void _streaming_past_days(int days)
{
	char *streaming_data;
	char *pdata;
	int counts=0;
	int pos = 0;
	int i;
	I32U epoch_start, epoch_end, current_epoch;
	int steps, distance, calories, sleep, sleep_state, sleep_by_36hr;
	int test_flag = 0;
	int entry_count = 0;

	// First, enable streaming
	cling.system.mcu_reg[REGISTER_MCU_CTL] |= CTL_IM;
	cling.ble.packet_received_ts = CLK_get_system_time();

	BTLE_update_streaming_minute_entries();

	counts = cling.ble.streaming_minute_file_amount;
	counts <<= 8; // Each file has 256 entries
	counts += cling.ble.streaming_minute_scratch_amount;
	streaming_data = (char *)malloc(counts * 16);

	while (counts > 0)
	{
		if (!BTLE_streaming_authorized())
			break;
		
		_ack_streaming_packet();
		pos += _interpret_streaming_packet(streaming_data + pos);
		counts--;
	}

	// Get total activities for latest 30 days
	for (i = 0; i < days; i++) {
		epoch_start = RTC_get_epoch_day_start(i);
		epoch_end = epoch_start + EPOCH_DAY_SECOND;

		N_SPRINTF("[MAIN] day: %d, epoch start: %d, end: %d", i, epoch_start, epoch_end);

		counts = cling.ble.streaming_minute_file_amount;
		counts <<= 8; // Each file has 256 entries
		counts += cling.ble.streaming_minute_scratch_amount;
		pos = 0;
		pdata = streaming_data;
		steps = 0;
		distance = 0;
		calories = 0;
		sleep = 0;
		test_flag = 0;
		sleep_by_36hr = 0;
		entry_count = 0;
		while (counts>0)
		{
			// Get current epoch
			current_epoch = *pdata++;
			current_epoch <<= 8;
			current_epoch |= (I8U)(*pdata++);
			current_epoch <<= 8;
			current_epoch |= (I8U)(*pdata++);
			current_epoch <<= 8;
			current_epoch |= (I8U)(*pdata++);
			N_SPRINTF("epoch in flash: %d", current_epoch);

			// Make sure current epoch is within the specified day range
			if ((current_epoch >= epoch_start) && (current_epoch < epoch_end)/* && test_flag*/) {
				entry_count++;
#if 1
				if (current_epoch == 1400666400) {
					Y_SPRINTF("...");
				}
#endif
				pdata++; // Skin temp
				pdata++; // Skin temp
				steps += (I8U)(*pdata++); // walking
				steps += (I8U)(*pdata++); // running

				calories += (I8U)(*pdata++); // calories
				
				distance += (I8U)(*pdata++); // distance

				sleep_state = (I8U)(*pdata++); // sleep
#if 0
				Y_SPRINTF("SLEEP STATE: %d", sleep_state);
				if (sleep_state != 1) {
					Y_SPRINTF("...");
				}
#endif
				if ((sleep_state == SLP_STAT_LIGHT) ||
					(sleep_state == SLP_STAT_SOUND) ||
					(sleep_state == SLP_STAT_REM)
					)
				{
					sleep += 60;
				}
				if (i == 1) {
					N_SPRINTF("[MAIN] STREAMING ACC: %d, %d, %d, %d, %d", current_epoch, steps, distance, calories, sleep);

					if (current_epoch == 1402294500) {
						Y_SPRINTF("...");
					}
				}
				pdata++; // heart rate
				pdata++; // Skin touch pad
				pdata++; // unused
				pdata++; // unused
				pdata++; // unused
			}
			else
			{
				pdata += 12;
			}
			counts--;
#if 0
			if ((!counts) && test_flag) {
				counts = 1440;
				pdata = streaming_data;
			}
#endif
		}

		Y_SPRINTF("STREAMING RESULT (%d, %d), s: %d, d: %d, c: %d, sleep: %d (Sleep_by_36hr: %d)", 
			i, entry_count, steps, distance, calories, sleep, sleep_by_36hr);

	}

	free(streaming_data);
}

static void _show_past_days(int days)
{
	int steps, distance, calories, sleep;
	int i;

	for (i = 0; i < days; i++)  {
		TRACKING_get_activity(i, TRACKING_STEPS, &steps);
		TRACKING_get_activity(i, TRACKING_DISTANCE, &distance);
		TRACKING_get_activity(i, TRACKING_CALORIES, &calories);
		TRACKING_get_activity(i, TRACKING_SLEEP, &sleep);

		Y_SPRINTF("TRACKING RESULT (%d), s: %d, d: %d, c: %d, sleep: %d", i, steps, distance, calories, sleep);
	}
}

static void _logging_long_term_sim()
{
	DAY_TRACKING_CTX day;
	DAY_STREAMING_CTX streaming;
	I32U tracking_flash_offset;
	SYSTIME_CTX local;
	int i = 0;
	int sleep_counting_down = 25200;
	int sleep_light_count_in_2sec = 7400;
	int sleep_sound_count_in_2sec = 9400;
	int sleep_rem_count_in_2sec = 0;
	int counts_per_2_second = 4;
	int steps, distance, calories = 0, sleep = 0, calories_diff;
	int b_test_point;

	RTC_get_local_clock(&local);

	Y_SPRINTF("current time: %d, %d, %d, %d, %d, %d", local.year, local.month, local.day, local.hour, local.minute, local.second);

	//for (i = 1; i < (1440 * 6 * 20); i++) {
	for (i = 1; i < 1440 * 30 * 30; i++) { // 20 days, 2 seconds per loop

		cling.touch.b_skin_touch = 1;
		RTC_timer_handler(0);
		cling.hr.current_rate = 78;
		cling.therm.current_temperature = 320;
		cling.sleep.state = SLP_STAT_AWAKE;

		if (sleep_counting_down > 0) {
			sleep_counting_down -= 2;
		}
		else {
			if (sleep_light_count_in_2sec > 0) {
				sleep_light_count_in_2sec--;
				cling.sleep.state = SLP_STAT_LIGHT;
				calories_diff = 19; // 1.2 for rest
			}
			else if (sleep_sound_count_in_2sec > 0) {
				sleep_sound_count_in_2sec--;
				cling.sleep.state = SLP_STAT_SOUND;
				calories_diff = 19; // 1.2 for rest
			}
			else if (sleep_rem_count_in_2sec > 0) {
				sleep_rem_count_in_2sec--;
				cling.sleep.state = SLP_STAT_REM;
				calories_diff = 19; // 1.2 for rest
			}
		}

		if (cling.sleep.state == SLP_STAT_AWAKE) {
#if 0
			//cling.activity.day.calories+=32;
			cling.activity.day.distance += 48;
			cling.activity.day.walking += 4;
			//cling.activity.day.running++;
			//cling.activity.day.sleep = 0;
			cling.sleep.state = SLP_STAT_AWAKE;
			calories_diff = 115; // 1.2 for rest
#else
			//cling.activity.day.calories+=32;
			cling.activity.day.distance += counts_per_2_second * 19;
			//cling.activity.day.walking += 4;
			cling.activity.day.running += counts_per_2_second;
			//cling.activity.day.sleep = 0;
			calories_diff = 200; // 1.2 for rest
#endif
		}

		if ((cling.time.time_since_1970 > 1402243200) && (cling.time.time_since_1970 <= 1402329600)) {
			if (cling.time.local_minute_updated) {
				steps = cling.activity.day.walking + cling.activity.day.running;
				distance = cling.activity.day.distance;
				calories += calories_diff;
				if ((cling.sleep.state == SLP_STAT_LIGHT) || (cling.sleep.state == SLP_STAT_SOUND) || (cling.sleep.state == SLP_STAT_REM)) {
					sleep += 60;
				}
				// Put epoch backwards 60 seconds as we record time stamp at begining of each minute
				N_SPRINTF("[MAIN] input acc: %d, %d, %d, %d, %d", (cling.time.time_since_1970 - 60), steps, (distance >> 4), (calories >> 4), sleep);
			}
		}

		RTC_get_local_clock(&local);

		if ((local.hour == 23) && (local.minute == 59) && (local.second == 58)) {

			N_SPRINTF("current time: %d, %d, %d, %d, %d, %d", local.year, local.month, local.day, local.hour, local.minute, local.second);
			N_SPRINTF("current epoch: %d", cling.time.time_since_1970);

			if (local.day == 30) {
				tracking_flash_offset = 0;
			}

			// Get daily total
			tracking_flash_offset = TRACKING_get_daily_total(&day);

			// Get other streaming details
			TRACKING_get_daily_streaming_stat(&streaming);
			TRACKING_get_daily_streaming_sleep(&streaming);
			if (counts_per_2_second == 5) {
				Y_SPRINTF("-- streaming:  s:%d, ca:%d, ci:%d, d:%d, hr:%d, t:%d, sl:%d, sr:%d, ss:%d",
					streaming.steps,
					streaming.calories_active,
					streaming.calories_idle,
					streaming.distance,
					streaming.heart_rate,
					streaming.temperature,
					streaming.sleep_light,
					streaming.sleep_rem,
					streaming.sleep_sound);
			}

			N_SPRINTF("-- tracking offset (time sync) ---: %d", tracking_flash_offset);
			N_SPRINTF("-- Current stats (sim):  %d, %d, %d, %d", day.walking, day.running, (day.distance >> 4), (day.calories >> 4));
			N_SPRINTF("\n");
		}

		if (cling.time.local_day_updated) {

			counts_per_2_second++;
			//if (counts_per_2_second > 6) 
			{
				counts_per_2_second = 5;
			}
			N_SPRINTF(" -- local day updated (sps: %d), light: %d, sound: %d, rem: %d ---", 
				counts_per_2_second, sleep_light_count_in_2sec, sleep_sound_count_in_2sec, sleep_rem_count_in_2sec);
		}

		b_test_point = 0;
		if (cling.time.local_noon_updated) {
			sleep_counting_down = 36000; // sleep starts in 10 hours
			sleep_light_count_in_2sec = 7500; // 7500x2 seconds -> 15000 seconds = 4 hour 10 minutes
			sleep_sound_count_in_2sec = 8790; // 8790x2 seconds -> 17580 seconds = 4 hour 53 minutes
			sleep_rem_count_in_2sec = 0; // 0 seconds -> 0 hours

			b_test_point = 1;
#if 0
			int s0, s1, s2, s3, s4, s5, s6;

			TRACKING_get_activity(0, TRACKING_SLEEP, &s0);
			TRACKING_get_activity(1, TRACKING_SLEEP, &s1);
			TRACKING_get_activity(2, TRACKING_SLEEP, &s2);
			TRACKING_get_activity(3, TRACKING_SLEEP, &s3);
			TRACKING_get_activity(4, TRACKING_SLEEP, &s4);
			TRACKING_get_activity(5, TRACKING_SLEEP, &s5);
			TRACKING_get_activity(6, TRACKING_SLEEP, &s6);
			Y_SPRINTF(" -- sleep before noon: %d, %d, %d, %d, %d, %d, %d", s0, s1, s2, s3, s4, s5, s6);
#endif
		}

		TRACKING_data_logging();


		if (b_test_point) {
			int s0, s1, s2, s3, s4, s5, s6;
			b_test_point = 0;
			TRACKING_get_activity(0, TRACKING_SLEEP, &s0);
			TRACKING_get_activity(1, TRACKING_SLEEP, &s1);
			TRACKING_get_activity(2, TRACKING_SLEEP, &s2);
			TRACKING_get_activity(3, TRACKING_SLEEP, &s3);
			TRACKING_get_activity(4, TRACKING_SLEEP, &s4);
			TRACKING_get_activity(5, TRACKING_SLEEP, &s5);
			TRACKING_get_activity(6, TRACKING_SLEEP, &s6);
			N_SPRINTF(" -- sleep after noon: %d, %d, %d, %d, %d, %d, %d", s0, s1, s2, s3, s4, s5, s6);
		}
	}

	// Flush the last minute data to flash
	cling.time.local_minute_updated = TRUE;
	TRACKING_data_logging();

	// Display past 7 days of statistics
	_show_past_days(30);

	// Streaming test for up to 7 days
	_streaming_past_days(30);

}

#ifdef _SLEEP_SIMULATION_
static void _sleep_sim()
{
	int i, j, z;

	// Simulation Environment Setup
	//
	// - User wearing device
	// - Heart Rate: 78
	// - Skin Temperature: 320
	//
	cling.touch.b_skin_touch = 1;
	cling.hr.current_rate = 78;
	cling.therm.current_temperature = 320;

	SIM_setup_reminder();

	SIM_setup_idle_alert();

	for (z = 0; z < 2; z++) {

		Y_SPRINTF("DAY -------------- %d ----------------", z);

		for (j = 0; j < (1440 * 60); j++) {
			RTC_timer_handler(0);

			for (i = 0; i < 50; i++) {
				SYSCLK_update_tick();
#if 0
				REMINDER_state_machine();

				NOTIFIC_state_machine();

				USER_state_machine();
#endif
				SENSOR_accel_processing();

				TRACKING_data_logging();
			}

		}
	}
}

extern int activitydata[];
extern int wSteps[];
extern int skin_touch[];

static void _sleep_activity_sim()
{
	int i, j, z, k, overall_cnt;

	// Simulation Environment Setup
	//
	// - User wearing device
	// - Heart Rate: 78
	// - Skin Temperature: 320
	//
	cling.touch.b_skin_touch = 1;
	cling.hr.current_rate = 78;
	cling.therm.current_temperature = 320;

	SIM_setup_reminder();

	SIM_setup_idle_alert();

	cling.sleep.state = 1;
	overall_cnt = 0;

	for (z = 0; z < 2; z++) {

		Y_SPRINTF("DAY -------------- %d ----------------", z);

		for (j = 0; j < 1440; j++) {

			if ((cling.time.local.day == 22) && (cling.time.local.hour == 20) && (cling.time.local.minute == 35)) {
				cling.time.local.day = 22;
			}

			if ((activitydata[overall_cnt + j] == 30) && (wSteps[overall_cnt + j] == 6)) {
				i = 0;
			}

			SLEEP_activity_minute_sim(activitydata[overall_cnt + j], wSteps[overall_cnt + j], skin_touch[overall_cnt + j]);

			for (i = 0; i < 60; i++) {
				RTC_timer_handler(0);

				TRACKING_data_logging();
			}

			if (overall_cnt + j > 2060)
				break;
			Y_SPRINTF("Sleep state: %d, activity cnt: %d, steps: %d, skin: %d", cling.sleep.state, activitydata[overall_cnt + j], wSteps[overall_cnt + j], skin_touch[overall_cnt + j]);
		}
		overall_cnt += 1440;
	}
}
#endif

#ifdef _REMINDER_SIMULATION_
static void _reminder_sim()
{
	int i, j;

	SIM_setup_reminder();

	SIM_setup_idle_alert();

	for (j = 0; j < (1440 * 60); j++) {
		RTC_timer_handler(0);

		for (i = 0; i < 50; i++) {
			SYSCLK_update_tick();

			REMINDER_state_machine();

			NOTIFIC_state_machine();

			USER_state_machine();
		}

	}

}
#endif

int _tmain(int argc, _TCHAR* argv[])
{

	//
	// Simulation Nor Flash based on FLASH size in memory configuration
	//
	// Allocate a large memory, 1 MB
	//
	p_flash = (I8U *)malloc(1024*1024);
	memset(p_flash+FLASH_SIZE, 0, 656 * 1024);

	// Open a logging file
	fLog = fopen("sim_log.txt", "w");
	if (!fLog) {
		exit(1);
	}

	//
	_cling_global_init();

#ifdef USING_VIRTUAL_ACTIVITY_SIM
	SIM_init();
#endif

	HAL_init();

	SYSTEM_init();
	//
	// Algorithms (core pedometer)
	//
	// -- activity detection and classification
	//
	//
	TRACKING_initialization();

	// long term (30 days) logging simulation
#ifdef _LONG_TERM_TRACKING_
	_logging_long_term_sim();
#endif

#ifdef _SLEEP_SIMULATION_
	// Sleep simulation
	//_sleep_sim();
	_sleep_activity_sim();
#endif

#ifdef _REMINDER_SIMULATION_
	_reminder_sim();
#endif


	// Dump out a file from the file system for comparison
	//_file_testing_rd();

	fclose(fLog);

	free(p_flash);

	return 0;
}

