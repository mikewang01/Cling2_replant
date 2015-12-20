/***************************************************************************/
/**
 * File: uart.c
 * 
 * Description: UART driver mainly for debugging purpose
 *
 * Created on Jan 26, 2014
 *
 ******************************************************************************/

#include "main.h"
#ifndef _CLING_PC_SIMULATION_
//#include "simple_uart.h"
void simple_uart_put(uint8_t cr)
{
  NRF_UART0->TXD = (uint8_t)cr;

  while (NRF_UART0->EVENTS_TXDRDY!=1)
  {
    // Wait for TXD data to be sent
  }

  NRF_UART0->EVENTS_TXDRDY=0;
}

void simple_uart_putstring(const uint8_t *str)
{
  uint_fast8_t i = 0;
  uint8_t ch = str[i++];
  while (ch != '\0')
  {
    simple_uart_put(ch);
    ch = str[i++];
  }
}

void simple_uart_config(  uint8_t rts_pin_number,
                          uint8_t txd_pin_number,
                          uint8_t cts_pin_number,
                          uint8_t rxd_pin_number,
                          bool hwfc)
{
  nrf_gpio_cfg_output(txd_pin_number);
  nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);  

  NRF_UART0->PSELTXD = txd_pin_number;
  NRF_UART0->PSELRXD = rxd_pin_number;


  NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;
  NRF_UART0->EVENTS_RXDRDY    = 0;
}
#endif
void UART_init(void)
{
		// UART initialization
#ifndef _CLING_PC_SIMULATION_

    simple_uart_config(GPIO_UART_RTS_FAKE, GPIO_UART_TX, GPIO_UART_CTS_FAKE, GPIO_UART_RX, false);

	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos;
#endif
}

void UART_print(char *str)
{
	if (!cling.system.b_powered_up)
		return;
	
#ifndef _CLING_PC_SIMULATION_
	simple_uart_putstring((const uint8_t *)str);
#endif
}

void UART_disabled()
{
#ifndef _CLING_PC_SIMULATION_
		// Disable UART if we don't use it for debug
		NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
#endif
}

