/***************************************************************************/
/**
 * File: gpio.c
 * 
 * Description: All GPIO related for Wristband device
 *
 * Created on December 22, 2015
 *
 ******************************************************************************/

#include "main.h"

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_output(uint32_t pin_number, BOOLEAN b_drive)
#else
static void _gpio_cfg_output(uint32_t pin_number, BOOLEAN b_drive)
#endif
{
#ifndef _CLING_PC_SIMULATION_
	// Output pin, we need to disconnect it from input buffer.
    NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
	if (b_drive) {
		nrf_gpio_pin_set(pin_number);
	} else {
		nrf_gpio_pin_clear(pin_number);
	}
#endif
}

#ifndef _CLING_PC_SIMULATION_
static void _gpio_cfg_disconnect_pull_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config)
{
	// when system is disconnected from input buffer, none of other parameter matters
    NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (pull_config << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
}
#endif

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_disconnect_input(uint32_t pin_number)
#else
static void _gpio_cfg_disconnect_input(uint32_t pin_number)
#endif
{
#ifndef _CLING_PC_SIMULATION_
	// when system is disconnected from input buffer, none of other parameter matters
    NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
#endif
}

#ifndef _CLING_PC_SIMULATION_
static __INLINE void _gpio_cfg_connect_input(uint32_t pin_number, nrf_gpio_pin_sense_t sense_config, nrf_gpio_pin_pull_t pull_config)
{
	// Always enable the input buffer in a separate write operation, before enabling the sense functionality
	NRF_GPIO->PIN_CNF[pin_number] = (NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos)
																			| (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
																			| (pull_config << GPIO_PIN_CNF_PULL_Pos)
																			| (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
																			| (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
	
	if (sense_config != NRF_GPIO_PIN_NOSENSE) {
    // when system is connected to input buffer, we need to configure the sensing and pull
		// Note: drive strength only matters to output
    NRF_GPIO->PIN_CNF[pin_number] = (sense_config << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (pull_config << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
	}
}
#endif

void GPIO_system_powerup()
{
#ifndef _CLING_PC_SIMULATION_	
	// System power up.
	_gpio_cfg_output(GPIO_CHARGER_SD      , TRUE);     // Charger Shut down pin, pull UP to power up system
	
	cling.system.b_powered_up = TRUE;
#endif	
}

void GPIO_system_powerdown()
{
#ifndef _CLING_PC_SIMULATION_
	// System power down.
	_gpio_cfg_output(GPIO_CHARGER_SD      , FALSE);    // Charger Shut down pin, pull down
	
	cling.system.b_powered_up = FALSE;

  // Disconnecting all of the GPIO pin,in addition to CHARGER SD pin.
	_gpio_cfg_disconnect_input(GPIO_RESERVE_0      );   
  _gpio_cfg_disconnect_input(GPIO_THERMISTOR_IN  );   
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MOSI     );  
	_gpio_cfg_disconnect_input(GPIO_CHARGER_IN     );   
	_gpio_cfg_disconnect_input(GPIO_SENSOR_INT_1   ); 
	
  _gpio_cfg_disconnect_input(GPIO_THERMISTOR_REF );   
	_gpio_cfg_disconnect_input(GPIO_RESERVE_6      );  
	_gpio_cfg_disconnect_input(GPIO_TEMPERATURE_POWER_ON);  
	_gpio_cfg_disconnect_input(GPIO_CHARGER_EN     );  
	_gpio_cfg_disconnect_input(GPIO_SPI_0_CS_OLED  );  
	
	_gpio_cfg_disconnect_input(GPIO_SPI_0_CS_NFLASH);   
	_gpio_cfg_disconnect_input(GPIO_OLED_RST       );   
	_gpio_cfg_disconnect_input(GPIO_OLED_A0        );  
	_gpio_cfg_disconnect_input(GPIO_SPI_0_SCK      );   
	_gpio_cfg_disconnect_input(GPIO_CHARGER_CHGFLAG); 
	
	_gpio_cfg_disconnect_input(GPIO_TWI1_CLK       );   
	_gpio_cfg_disconnect_input(GPIO_TWI1_DATA      );  
	_gpio_cfg_disconnect_input(GPIO_TOUCH_INT      );  
	_gpio_cfg_disconnect_input(GPIO_OLED_POWER_ON  );   
	_gpio_cfg_disconnect_input(GPIO_TOUCH_RST      ); 
	
	_gpio_cfg_disconnect_input(GPIO_HOMEKEY        );
	_gpio_cfg_disconnect_input(GPIO_UART_RX        );   
	_gpio_cfg_disconnect_input(GPIO_VIBRATOR_EN    );   
	_gpio_cfg_disconnect_input(GPIO_RESERVE_23     );   
	_gpio_cfg_disconnect_input(GPIO_RESERVE_24     ); 
	
	_gpio_cfg_disconnect_input(GPIO_UART_TX        );   	
	_gpio_cfg_disconnect_input(GPIO_RESERVE_26     );   	
	_gpio_cfg_disconnect_input(GPIO_RESERVE_28     );  
	_gpio_cfg_disconnect_input(GPIO_SENSOR_INT_2   ); 
	_gpio_cfg_disconnect_input(GPIO_SPI_0_CS_ACC   );   
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MISO     );  
#endif
}

void GPIO_init()
{
	// 0 .. 4
#ifndef _CLING_PC_SIMULATION_
	_gpio_cfg_disconnect_input(GPIO_RESERVE_0      );   // Not used
  _gpio_cfg_disconnect_input(GPIO_THERMISTOR_IN  );   // Thermistor input
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MOSI     );   // SPI bus, MOSI
	_gpio_cfg_disconnect_input(GPIO_CHARGER_IN     );   // ADC analog pin	
	_gpio_cfg_connect_input(GPIO_SENSOR_INT_1 , NRF_GPIO_PIN_SENSE_HIGH, NRF_GPIO_PIN_NOPULL); // Needed, acc sense input 1 pin, no pull	

	// 5 .. 9
  _gpio_cfg_disconnect_input(GPIO_THERMISTOR_REF );   // Thermistor reference input
	_gpio_cfg_disconnect_input(GPIO_RESERVE_6      );   // Not used
	_gpio_cfg_disconnect_input(GPIO_TEMPERATURE_POWER_ON);  // Thermistor power on pin, needed
	_gpio_cfg_output(GPIO_CHARGER_EN      , TRUE   );   // Charge enable, drive HIGH
	_gpio_cfg_output(GPIO_SPI_0_CS_OLED   , TRUE   );   // SPI CS (OLED), drive HIGH	
	
	// 10 .. 14
	_gpio_cfg_output(GPIO_SPI_0_CS_NFLASH , TRUE   );   // SPI CS (NOR FLASH), drive HIGH
	_gpio_cfg_output(GPIO_OLED_RST        , TRUE   );   // OLED reset, drive HIGH
	_gpio_cfg_output(GPIO_OLED_A0         , FALSE  );   // OLED A0, drive LOW
	_gpio_cfg_disconnect_input(GPIO_SPI_0_SCK      );   // SPI bus,SCK
	_gpio_cfg_connect_input(GPIO_CHARGER_CHGFLAG , NRF_GPIO_PIN_NOSENSE, NRF_GPIO_PIN_PULLUP); // Charger Flag, no sense, internal pull up
	
	// 15 .. 19	
	_gpio_cfg_disconnect_input(GPIO_TWI1_CLK       );   // I2C bus, CLK	
	_gpio_cfg_disconnect_input(GPIO_TWI1_DATA      );   // I2C	bus, DATA
	_gpio_cfg_connect_input(GPIO_TOUCH_INT, NRF_GPIO_PIN_SENSE_LOW, NRF_GPIO_PIN_PULLUP);  // Touch pin, sense low, no pull
	_gpio_cfg_output(GPIO_OLED_POWER_ON   , TRUE   );   // OLED boost Controller Power On, drive HIGH	
	_gpio_cfg_output(GPIO_TOUCH_RST       , TRUE   );   // TOUCH reset, drive HIGH	
	
	// 20 .. 24
	_gpio_cfg_connect_input(GPIO_HOMEKEY    , NRF_GPIO_PIN_SENSE_LOW, NRF_GPIO_PIN_PULLUP);
	_gpio_cfg_disconnect_input(GPIO_UART_RX        );   // UART pin, not used	
	_gpio_cfg_output(GPIO_VIBRATOR_EN     , FALSE  );   // Vibrator pin, pull down	
	_gpio_cfg_disconnect_input(GPIO_RESERVE_23     );   // Not used	
	_gpio_cfg_disconnect_input(GPIO_RESERVE_24     );   // Not used		

	// 25 .. 29	
	_gpio_cfg_disconnect_input(GPIO_UART_TX        );   // UART pin, debug needed		
	_gpio_cfg_disconnect_input(GPIO_RESERVE_26     );   // Not used		
	/* 27: Charger Shut down pin --- Control system power on and power down*/
	_gpio_cfg_disconnect_input(GPIO_RESERVE_28     );   // Not used		
	_gpio_cfg_connect_input(GPIO_SENSOR_INT_2 , NRF_GPIO_PIN_SENSE_HIGH, NRF_GPIO_PIN_NOPULL); // Needed, acc sense input 2 pin, no pull	
	
	// 30 .. 31	
	_gpio_cfg_output(GPIO_SPI_0_CS_ACC    , TRUE   );   // SPI CS (ACC), drive HIGH	
	_gpio_cfg_disconnect_input(GPIO_SPI_0_MISO     );   // SPI bus, MISO	
	
#endif
}

void GPIO_interrupt_enable()
{
#ifndef _CLING_PC_SIMULATION_
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
	sd_nvic_SetPriority(GPIOTE_IRQn, NRF_APP_PRIORITY_LOW);
	sd_nvic_EnableIRQ(GPIOTE_IRQn);
#endif
}




void GPIO_twi_init(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_
		I32U error_code; 
		nrf_drv_twi_config_t twi_config;
		

	if (twi_master_instance==0) {
#if (TWI0_ENABLED == 1)
		const nrf_drv_twi_t p_twi0_instance = NRF_DRV_TWI_INSTANCE(0);
		
		twi_config.frequency = NRF_TWI_FREQ_400K;
		twi_config.scl = 15;
		twi_config.sda = 16;
		twi_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;
		Y_SPRINTF("[GPIO] twi 0 initialization: %d, %d", error_code, TWI_COUNT);

#endif
	} else {
		const nrf_drv_twi_t p_twi1_instance = NRF_DRV_TWI_INSTANCE(1);
		
		twi_config.frequency = NRF_TWI_FREQ_400K;
		twi_config.scl = 24;
		twi_config.sda = 26;
		twi_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;
		error_code = nrf_drv_twi_init(&p_twi1_instance, &twi_config, NULL, NULL);
		APP_ERROR_CHECK(error_code);
		nrf_drv_twi_enable(&p_twi1_instance);
		cling.system.b_twi_1_ON = TRUE;
		Y_SPRINTF("[GPIO] twi 1 initialization: %d, %d", error_code, TWI_COUNT);
		
		BASE_delay_msec(1);
	}
#endif
}
void GPIO_twi_enable(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_

	if (twi_master_instance==0) {
#if (TWI0_ENABLED == 1)
		const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(0);
		
		nrf_drv_twi_enable(&twi);
		
#endif
	} else if (twi_master_instance==1) {
		const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
		
		if (!cling.system.b_twi_1_ON) {
			nrf_drv_twi_enable(&twi);
			
			cling.system.b_twi_1_ON = TRUE;
			N_SPRINTF("[GPIO] twi 1 enabled");
			BASE_delay_msec(1);
		}
	}
#endif
}

void GPIO_twi_disable(I8U twi_master_instance)
{
#ifndef _CLING_PC_SIMULATION_

	if (twi_master_instance==0) {
#if (TWI0_ENABLED == 1)
		const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(0);
		
		nrf_drv_twi_disable(&twi);
		
#endif
	} else if (twi_master_instance==1) {
		const nrf_drv_twi_t twi = NRF_DRV_TWI_INSTANCE(1);
		
		if (cling.system.b_twi_1_ON) {
			nrf_drv_twi_disable(&twi);
			
			cling.system.b_twi_1_ON = FALSE;
			N_SPRINTF("[GPIO] twi 1 disabled");
			BASE_delay_msec(1);
		}
	}
#endif
}

void GPIO_vbat_adc_config()
{
#ifndef _CLING_PC_SIMULATION_
  const nrf_adc_config_t nrf_adc_config = {
		.resolution = NRF_ADC_CONFIG_RES_10BIT,
		.scaling = NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE,
		.reference = NRF_ADC_CONFIG_REF_VBG};
	
	// Initialize and configure ADC
	nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
#endif
}

void GPIO_vbat_adc_disable()
{
#ifndef _CLING_PC_SIMULATION_
	NRF_ADC->TASKS_STOP = 1;
	NRF_ADC->ENABLE = 0;
	
	// Disable all the input analog pins
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
									| (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos)					/*! Analog input pins disabled. */
									| (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
									| (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
									| (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */ 	

	_gpio_cfg_disconnect_input(GPIO_CHARGER_IN      );  // ADC analog pin
#endif
}

void GPIO_therm_power_on()
{
#ifndef _CLING_PC_SIMULATION_	
	// power on
	_gpio_cfg_output(GPIO_TEMPERATURE_POWER_ON      , TRUE);      // thermistor power on, drive HIGH
#endif
}

void GPIO_therm_power_off()
{
#ifndef _CLING_PC_SIMULATION_	
	// Disonnect Thermistor power on
  _gpio_cfg_disconnect_input(GPIO_TEMPERATURE_POWER_ON);
#endif
}

void GPIO_therm_adc_config(void)
{
#ifndef _CLING_PC_SIMULATION_
  const nrf_adc_config_t nrf_adc_config = {
		.resolution = NRF_ADC_CONFIG_RES_10BIT,
		.scaling = NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD,
		.reference = NRF_ADC_CONFIG_REF_VBG};

	// Initialize and configure ADC
	nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
#endif
}

void GPIO_vibrator_on_block(I8U latency)
{
#ifndef _CLING_PC_SIMULATION_
	I32U t_curr = CLK_get_system_time();
	I32U t_diff = 0;
	RTC_start_operation_clk();
	_gpio_cfg_output(GPIO_VIBRATOR_EN, TRUE);
	while (t_diff < latency) {
		// Feed watchdog every 4 seconds upon RTC interrupt
		Watchdog_Feed();

		sd_app_evt_wait();
		t_diff = CLK_get_system_time();
		t_diff -= t_curr;
		if (t_diff > latency)
			break;
	}
	
	_gpio_cfg_output(GPIO_VIBRATOR_EN, FALSE);
#endif
}

void GPIO_vibrator_set(BOOLEAN b_on)
{
#ifndef _CLING_PC_SIMULATION_	
	if (b_on)
		_gpio_cfg_output(GPIO_VIBRATOR_EN, TRUE);
	else
		_gpio_cfg_output(GPIO_VIBRATOR_EN, FALSE);
#endif	
}

void GPIO_charger_reset()
{
#ifndef _CLING_PC_SIMULATION_

	_gpio_cfg_output(GPIO_CHARGER_EN      , FALSE);      // Charge enable, drive LOW
	BASE_delay_msec(100);
	_gpio_cfg_output(GPIO_CHARGER_EN      , TRUE);      // Charge enable, drive HIGH

#endif
}

void GPIO_interrupt_handle()
{
#ifndef _CLING_PC_SIMULATION_	
	// detect a possible state change
	HOMEKEY_check_on_hook_change();
	
	// No need to check touch controller as system is powered off
	if (!cling.system.b_powered_up)
		return;
	
#ifdef _ENABLE_TOUCH_
	// Touch panel gesture check
	TOUCH_gesture_check();
#endif
#endif	
}
