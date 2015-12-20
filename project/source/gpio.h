/***************************************************************************//**
 * File gpio.h
 * 
 * Description: User Interface header
 *
 *
 ******************************************************************************/

#ifndef __GPIO_HEADER__
#define __GPIO_HEADER__

#include "standards.h"


#define GPIO_NFC_CLK         0
#define GPIO_THERMISTOR_REF  1
#define GPIO_SPI_0_CS_ACC    2
#define GPIO_CHARGER_IN      3
#define GPIO_RESERVE_4       4

#define GPIO_THERMISTOR_IN   5
#define GPIO_SPI_0_CS_OLED   6
#define GPIO_OLED_RST        7
#define GPIO_UART_TX         8
#define GPIO_VIBRATOR_EN     9

#define GPIO_RESERVE_10      10
#define GPIO_TOUCH_INT       11
#define GPIO_RESERVE_12      12
#define GPIO_RESERVE_13      13
#define GPIO_RESERVE_14      14

#define GPIO_TEMPERATURE_POWER_ON 15
#define GPIO_SENSOR_INT_1    16
#define GPIO_RESERVE_17      17
#define GPIO_CHARGER_CHGFLAG 18
#define GPIO_CHARGER_EN      19

#define GPIO_CHARGER_SD      20
#define GPIO_HOMEKEY         21
#define GPIO_SPI_0_CS_NFLASH 22
#define GPIO_NFC_RESET       23
#define GPIO_TWI1_CLK        24

#define GPIO_SPI_0_MISO      25
#define GPIO_TWI1_DATA       26
#define GPIO_SPI_0_MOSI      27
#define GPIO_SPI_0_SCK       28
#define GPIO_NFC_DATA        29

#define GPIO_UART_RX         30
#define GPIO_OLED_A0         31

#define GPIO_UART_RTS_FAKE   13
#define GPIO_UART_CTS_FAKE   14

// Initialization
void GPIO_init(void);

// ADC analog pin configuration for battery
void GPIO_vbat_adc_config(void);
void GPIO_vbat_adc_disable(void);

// ADC analog pin configuration for thermistor
void GPIO_therm_adc_config(void);
void GPIO_therm_power_on(void);
void GPIO_therm_power_off(void);

// TWI pin configuration
void GPIO_twi_init(I8U twi_master_instance);
void GPIO_twi_enable(I8U twi_master_instance);
void GPIO_twi_disable(I8U twi_master_instance);

// Vibrator control
void GPIO_vibrator_set(BOOLEAN b_on);
void GPIO_vibrator_on_block(I8U latency);

// Interrupt control
void GPIO_interrupt_enable(void);
void GPIO_spi1_disconnect(void);

// Charger reset
void GPIO_charger_reset(void);
void GPIO_system_powerup(void);
void GPIO_system_powerdown(void);

// Interrupt handle 
void GPIO_interrupt_handle(void);

#endif
