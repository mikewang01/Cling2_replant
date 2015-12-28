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
/*
#define GPIO_RESERVE_0             0
#define GPIO_THERMISTOR_IN         1
#define GPIO_SPI_0_MOSI            2
#define GPIO_CHARGER_IN            3
#define GPIO_SENSOR_INT_1          4

#define GPIO_THERMISTOR_REF        5
#define GPIO_RESERVE_6             6   
#define GPIO_TEMPERATURE_POWER_ON  7
#define GPIO_CHARGER_EN            8
#define GPIO_SPI_0_CS_OLED         9

#define GPIO_SPI_0_CS_NFLASH       10
#define GPIO_OLED_RST              11
#define GPIO_OLED_A0               12
#define GPIO_SPI_0_SCK             13
#define GPIO_CHARGER_CHGFLAG       14

#define GPIO_TWI1_CLK              15
#define GPIO_TWI1_DATA             16
#define GPIO_TOUCH_INT             17
#define GPIO_OLED_POWER_ON         18
#define GPIO_TOUCH_RST             19

#define GPIO_HOMEKEY               20
#define GPIO_UART_RX               21
#define GPIO_VIBRATOR_EN           22
#define GPIO_RESERVE_23            23  
#define GPIO_RESERVE_24            24  

#define GPIO_UART_TX               25
#define GPIO_RESERVE_26            26  
#define GPIO_CHARGER_SD            27
#define GPIO_RESERVE_28            28  
#define GPIO_SENSOR_INT_2          29

#define GPIO_SPI_0_CS_ACC          30
#define GPIO_SPI_0_MISO            31

#define GPIO_UART_RTS_FAKE         23
#define GPIO_UART_CTS_FAKE         24
*/

#define GPIO_RESERVE_0             0
#define GPIO_THERMISTOR_IN         1
#define GPIO_SPI_0_MOSI            2
#define GPIO_CHARGER_IN            3
#define GPIO_SENSOR_INT_1          4

#define GPIO_THERMISTOR_REF        5
#define GPIO_RESERVE_6             6   
#define GPIO_TEMPERATURE_POWER_ON  7
#define GPIO_CHARGER_EN            8
#define GPIO_SPI_0_CS_OLED         9

#define GPIO_SPI_0_CS_NFLASH       10
#define GPIO_OLED_RST              11
#define GPIO_OLED_A0               12
#define GPIO_SPI_0_SCK             13
#define GPIO_CHARGER_CHGFLAG       14

#define GPIO_TWI1_CLK              15
#define GPIO_TWI1_DATA             16
#define GPIO_TOUCH_INT             17
#define GPIO_OLED_POWER_ON         18
#define GPIO_TOUCH_RST             19

#define GPIO_HOMEKEY               20
#define GPIO_UART_RX               21
#define GPIO_VIBRATOR_EN           22
#define GPIO_RESERVE_23            23  
#define GPIO_RESERVE_24            24  

#define GPIO_UART_TX               25
#define GPIO_RESERVE_26            26  
#define GPIO_CHARGER_SD            27
#define GPIO_RESERVE_28            28  
#define GPIO_SENSOR_INT_2          29

#define GPIO_SPI_0_CS_ACC          30
#define GPIO_SPI_0_MISO            31

#define GPIO_UART_RTS_FAKE         23
#define GPIO_UART_CTS_FAKE         24
//#define GPIO_NFC_CLK             0
//#define GPIO_NFC_RESET           23
//#define GPIO_NFC_DATA            29

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

// Charger reset
void GPIO_charger_reset(void);
void GPIO_system_powerup(void);
void GPIO_system_powerdown(void);

// Interrupt handle 
void GPIO_interrupt_handle(void);
#endif
