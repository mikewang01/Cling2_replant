/***************************************************************************
 * File: interrupt.c
 * 
 * Description: interrupt handler routines
 *
 * Created on Feb 20, 2014
 *
 ******************************************************************************/

#include "main.h"

/* Interrupt handler for ADC data ready event */
void ADC_IRQHandler(void)
{
	N_SPRINTF("[ADC INT] test:  %d",  ((NRF_ADC->CONFIG & 0x0000FF00)>>8));
	
	// check if data ready event occurs
	if (!nrf_adc_conversion_finished())
		return;
	
	nrf_adc_conversion_event_clean();

	//Use the STOP task to save current. Workaround for PAN_028 rev1.2 anomaly 1.
  nrf_adc_stop();
}


void UART0_IRQHandler(void)
{
}

void GPIOTE_IRQHandler(void)
{
	// Event ausing the interrupt must be cleared
	if (NRF_GPIOTE->EVENTS_PORT) {
		NRF_GPIOTE->EVENTS_PORT = 0;
	}
}

