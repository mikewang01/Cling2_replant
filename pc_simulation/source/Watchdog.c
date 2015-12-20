/***************************************************************************//**
 * @addtogroup WATCHDOG
 * @{ 
 ******************************************************************************/

#include "main.h"

#ifndef _CLING_PC_SIMULATION_	


#include "nrf_drv_wdt.h"

nrf_drv_wdt_channel_id m_channel_id;

void wdt_event_handler(void)
{
	N_SPRINTF("WATCH DOG");
}

EN_STATUSCODE Watchdog_Init(void)
{
#ifndef _CLING_PC_SIMULATION_
	#if 0
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 16*32768;   //ca 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
	NRF_WDT->TASKS_START = 1;
	#else
	I32U err_code;
	
    //Configure WDT.
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();

	#endif
#endif
	Y_SPRINTF("[WDG] Watchdog init");
  return STATUSCODE_SUCCESS;
}


void Watchdog_Feed(void)
{
	N_SPRINTF("[WDG] Watchdog FEED");
#ifndef _CLING_PC_SIMULATION_
	#if 0
	NRF_WDT->RR[0] = 0x6E524635;  //Reload watchdog register 0
	#else
	nrf_drv_wdt_channel_feed(m_channel_id);
	#endif
#endif
}

#endif

/** @} */
