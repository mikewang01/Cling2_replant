/***************************************************************************/
/**
 * File: oled.c
 *
 * Description : Color oled driver
 * 
 * Dot Matrix  : 96*3*64
 * 
 * Driver IC   : SSD1331
 *
 * Created on December 22, 2015
 *
 ******************************************************************************/
 
#include "main.h"
#include "photo.h"

extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];


/**@brief Function for OLED write command.
 */
static void _set_reg(uint8_t command)
{    
#ifndef _CLING_PC_SIMULATION_
  g_spi_tx_buf[0] = command;
	
  nrf_gpio_pin_clear(GPIO_OLED_A0);
	
  SPI_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 1, g_spi_rx_buf, 0, GPIO_SPI_0_CS_OLED);
#endif
}

/**@brief Function for OLED write data.
 */
static void _set_data(I16U num, I8U *data)
{
#ifndef _CLING_PC_SIMULATION_
  nrf_gpio_pin_set(GPIO_OLED_A0);
	
  SPI_master_tx_rx(SPI_MASTER_0, data, num,g_spi_rx_buf,0, GPIO_SPI_0_CS_OLED);  
#endif
}

void OLED_power_on(void) 
{
#ifndef _CLING_PC_SIMULATION_
  // Pull up reset pin and vcc boost Controller pin.
  nrf_gpio_pin_set( GPIO_OLED_RST );       // Reset driven high
  nrf_gpio_pin_set( GPIO_OLED_POWER_ON );  // Boost Controller Power On, drive HIGH
#endif
}

void OLED_power_off()
{
#ifndef _CLING_PC_SIMULATION_
  // Pull down reset pin and vcc boost Controller pin.
  nrf_gpio_pin_clear( GPIO_OLED_RST );      // reset driven low
  nrf_gpio_pin_clear( GPIO_OLED_POWER_ON ); // Boost Controller Power On, drive low 
#endif
}

void OLED_full_scree_show(I8U *pram)
{
#ifndef _CLING_PC_SIMULATION_
  // Behind the waiting for update.	
#endif  
}

void OLED_im_show(OLED_DISPLAY_MODE mode, I8U *pram, I8U offset)
{
#ifndef _CLING_PC_SIMULATION_
  // Behind the waiting for update.	
#endif
}

void OLED_display_all_red()
{
#ifndef _CLING_PC_SIMULATION_	
  I8U num_1,num_2;
  I8U data[2];
	
  data[0] = 0xf8;
  data[1] = 0x00;
	
  for(num_1 = 0; num_1 < 64; num_1++)
  {
	  for(num_2 = 0; num_2 <96 ; num_2++)
	  {
	    _set_data(2,data); 
	  }	
  }
#endif	
}

void OLED_display_all_green()
{ 
#ifndef _CLING_PC_SIMULATION_	
  I8U num_1,num_2;
  I8U data[2];
	
  data[0] = 0x07;
  data[1] = 0xe0;
	
  for(num_1 = 0; num_1 < 64; num_1++)
  {
	  for(num_2 = 0; num_2 < 96; num_2++)
	  {
	    _set_data(2,data); 
	  }
  }	
#endif
}

void OLED_display_all_blue()
{
#ifndef _CLING_PC_SIMULATION_	
  I8U num_1,num_2;
  I8U data[2];
	
  data[0] = 0x00;
  data[1] = 0x1f;
	
  for(num_1 = 0; num_1 < 64; num_1++)
  {
	  for(num_2 = 0; num_2 < 96; num_2++)
	  {
      _set_data(2,data); 
	  } 
  }	
#endif	
}  

void OLED_set_display(I8U on_off)
{
  if (on_off)
	_set_reg(0xAF);
  else
	_set_reg(0xAE);
}

void OLED_set_contrast(OLED_COLOR_TYPE tpye, I8U step)
{
  if(tpye == OLED_COLOR_RED)
	_set_reg(0x83); 
  else if(tpye == OLED_COLOR_GREEN)
    _set_reg(0x82); 
  else if(tpye == OLED_COLOR_BLUE)  
    _set_reg(0x81); 
  
  _set_reg(step);
}

/**@brief Function for OLED init.
 */
void OLED_init(I8U red_contrast, I8U green_contrast, I8U blue_contrast)
{    
#ifndef _CLING_PC_SIMULATION_	
  I16U pwm, delta;

  _set_reg(0xfd); // Command lock
  _set_reg(0x12);
  _set_reg(0xae); // Display off
  _set_reg(0xa4); // Normal Display mode

  _set_reg(0x15); // Set column address
  _set_reg(0x00); // Column address start 00
  _set_reg(0x5f); // Column address end 95

  _set_reg(0x75); // Set row address
  _set_reg(0x00); // Row address start 00
  _set_reg(0x3f); // Row address end 63	

  _set_reg(0x87); // Master current control
  _set_reg(0x09); // 9/16(160uA)

  // Set Contrast for Color Red
  OLED_set_contrast(OLED_COLOR_RED, red_contrast);
	
	// Set Contrast for Color Green
  OLED_set_contrast(OLED_COLOR_GREEN, green_contrast);	
	
	// Set Contrast for Color Blue
  OLED_set_contrast(OLED_COLOR_BLUE, blue_contrast);	
	
  _set_reg(0x8a);
  _set_reg(0x61);
  _set_reg(0x8b);
  _set_reg(0x62);
  _set_reg(0x8c);
  _set_reg(0x63);

  _set_reg(0xa0); // Set re-map & data format
  _set_reg(0x72); // Horizontal address increment
  _set_reg(0xa1); // Set display start line
  _set_reg(0x00); // Start 00 line
  _set_reg(0xa2); // Set display offset
  _set_reg(0x00);
  _set_reg(0xa8); // Set multiplex ratio
  _set_reg(0x3f); // 64MUX
  _set_reg(0xad);
  _set_reg(0x8f);
  _set_reg(0xb0); // set power save
  _set_reg(0x1a); //
  _set_reg(0xb1);
  _set_reg(0xf1); // Phase 2 period Phase 1 period
  _set_reg(0xb3); // Set Display Clock Divide Ratio/ Oscillator Frequency
  _set_reg(0xd0); // 0.97MHZ 0xd0
  _set_reg(0xbb); // set pre-charge
  _set_reg(0x3e); // 
  _set_reg(0xbe); // set Vcomh
  _set_reg(0x30); // 0.83Vref
  _set_reg(0xB9);
  _set_reg(0xB8); // Set Gray Scale Table
  
  pwm = 1;
  //The gray scale table setting below is for reference only
  delta=1;
  pwm=pwm;       _set_reg(pwm);//pw1
  pwm=pwm+delta; _set_reg(pwm);//pw3
  pwm=pwm+delta; _set_reg(pwm);//pw5
  pwm=pwm+delta; _set_reg(pwm);//pw7
	
  delta=2;
  pwm=pwm+delta; _set_reg(pwm);//pw9
  pwm=pwm+delta; _set_reg(pwm);//pw11
  pwm=pwm+delta; _set_reg(pwm);//pw13
  pwm=pwm+delta; _set_reg(pwm);//pw15
	
  delta=2;
  pwm=pwm+delta; _set_reg(pwm);//pw17
  pwm=pwm+delta; _set_reg(pwm);//pw19
  pwm=pwm+delta; _set_reg(pwm);//pw21
  pwm=pwm+delta; _set_reg(pwm);//pw23
	
  delta=3;
  pwm=pwm+delta; _set_reg(pwm);//pw25
  pwm=pwm+delta; _set_reg(pwm);//pw27
  pwm=pwm+delta; _set_reg(pwm);//pw29
  pwm=pwm+delta; _set_reg(pwm);//pw31
	
  delta=4;
  pwm=pwm+delta; _set_reg(pwm);//pw33
  pwm=pwm+delta; _set_reg(pwm);//pw35
  pwm=pwm+delta; _set_reg(pwm);//pw37
  pwm=pwm+delta; _set_reg(pwm);//pw39
	
  delta=5;
  pwm=pwm+delta; _set_reg(pwm);//pw41
  pwm=pwm+delta; _set_reg(pwm);//pw43
  pwm=pwm+delta; _set_reg(pwm);//pw45
  pwm=pwm+delta; _set_reg(pwm);//pw47
	
  delta=6;
  pwm=pwm+delta; _set_reg(pwm);//pw49
  pwm=pwm+delta; _set_reg(pwm);//pw51
  pwm=pwm+delta; _set_reg(pwm);//pw53
  pwm=pwm+delta; _set_reg(pwm);//pw55
	
  delta=7;
  pwm=pwm+delta; _set_reg(pwm);//pw57
  pwm=pwm+delta; _set_reg(pwm);//pw59
  pwm=pwm+delta; _set_reg(pwm);//pw61
  pwm=pwm+delta; _set_reg(pwm);//pw63

  _set_reg(0xad);  // Select external VCC supply at Display ON
  _set_reg(0x8e);  // Select External VP voltage supply
#endif	
}

void OLED_set_panel_off()
{
#ifndef _CLING_PC_SIMULATION_	
	CLING_OLED_CTX *o = &cling.oled;
	
	o->state = OLED_STATE_GOING_OFF;
	cling.ui.display_active = FALSE;
#endif	
}

BOOLEAN OLED_set_panel_on()
{
#ifndef _CLING_PC_SIMULATION_
	CLING_OLED_CTX *o = &cling.oled;

	// Start 20 ms timer for screen rendering
	RTC_start_operation_clk();
	
	// We are about to turn on OLED, if BLE is in idle mode, we should start advertising
	if (BTLE_is_idle()) {
		BTLE_execute_adv(TRUE);
	}
	
	if (OLED_STATE_IDLE == o->state) {
		o->state = OLED_STATE_APPLYING_POWER;
		
		Y_SPRINTF("[OLED] panel is turned on");
		
		return TRUE;
	} 
#endif
	return FALSE;
}

BOOLEAN OLED_is_panel_idle()
{
#ifndef _CLING_PC_SIMULATION_	
	CLING_OLED_CTX *o = &cling.oled;
	
	if (o->state == OLED_STATE_IDLE) {
		return TRUE;
	}
#endif	
	return FALSE;
}


/**@brief Function for OLED display state machine.
 */
void OLED_state_machine(void) 
{
#ifndef _CLING_PC_SIMULATION_
	I32U t_curr;
	CLING_OLED_CTX *o = &cling.oled;

	t_curr = CLK_get_system_time();
	
	if (o->state != OLED_STATE_IDLE) {
		// Start system timer
		RTC_start_operation_clk();
	}

	switch (o->state) {
		case OLED_STATE_IDLE:
			break;
		case OLED_STATE_APPLYING_POWER:
		{
			// turn on the power pins and wait the appropriate time
			o->ts = CLK_get_system_time(); // stores time we start this
			OLED_power_on();
			o->state = OLED_STATE_RESET_OLED;
			break;
		}
		case OLED_STATE_RESET_OLED:
		{
			if ((t_curr - o->ts) > OLED_POWER_START_DELAY_TIME){
				// Pulse the reset low for the minimum time.
				nrf_gpio_pin_clear(GPIO_OLED_RST);
				// now pulse reset for at least 3us
				BASE_delay_msec(2);
				nrf_gpio_pin_set(GPIO_OLED_RST);
				o->state = OLED_STATE_INIT_REGISTERS;
			}
			break;
		}
		case OLED_STATE_INIT_REGISTERS:
		{
			OLED_init(0xc0, 0x65, 0x95);
			o->state = OLED_STATE_INIT_UI;
			break;
		}
		case OLED_STATE_INIT_UI:
		{
			// CLING screen timeout
			cling.ui.display_active = TRUE;
			
			// Update display timeout base.
			cling.ui.display_to_base = CLK_get_system_time();
		
			N_SPRINTF("[0LED] ui to base: %d", cling.ui.display_to_base);
			
			// Reset blinking state
			cling.ui.clock_sec_blinking = TRUE;
			
			// If screen is turned, dismiss the secondary reminder vibration 
			if ((cling.reminder.state >= REMINDER_STATE_ON) && (cling.reminder.state <= REMINDER_STATE_SECOND_REMINDER)) {
				cling.ui.notif_type = NOTIFICATION_TYPE_REMINDER;
				UI_switch_state(UI_STATE_NOTIFICATIONS, 0);
				cling.reminder.ui_hh = cling.time.local.hour;
				cling.reminder.ui_mm = cling.time.local.minute;
				cling.reminder.ui_alarm_on = TRUE; // Indicate this is a active alarm reminder
				Y_SPRINTF("[OLED] state reminder: %d, %d, %d", cling.reminder.state, cling.reminder.ui_hh, cling.reminder.ui_mm);
			}
		
			if (cling.notific.state != NOTIFIC_STATE_IDLE) {
				UI_switch_state(UI_STATE_NOTIFICATIONS, 0);
			}						
				
			o->state = OLED_STATE_ON;
			break;
		}
		case OLED_STATE_ON:
			break;
		case OLED_STATE_GOING_OFF:
		{
			OLED_set_display(0);    /*display off*/
			OLED_power_off();
			o->ts = CLK_get_system_time(); // stores time we start this
			o->state = OLED_STATE_OFF;
			break;
		}
		case OLED_STATE_OFF:
		{
			// don't let power come back on for 100ms
			if (CLK_get_system_time() - o->ts > OLED_POWER_OFF_DELAY_TIME){
				o->state = OLED_STATE_IDLE;
			}
			break;
		}
		default:
			break;
	}
#endif
}
