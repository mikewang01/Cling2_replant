#include "main.h"

extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];

/**@brief Function for OLED write command.
 *
 */
static void _set_reg(uint8_t command)
{    
#ifndef _CLING_PC_SIMULATION_
	g_spi_tx_buf[0] = command;

	nrf_gpio_pin_clear(GPIO_OLED_A0);

	spi_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 1, 0, 0, g_spi_rx_buf, 0, 0, GPIO_SPI_0_CS_OLED);
#endif
}

static void _set_data(I16U num, I8U *data)
{
#ifndef _CLING_PC_SIMULATION_
	nrf_gpio_pin_set(GPIO_OLED_A0);
	spi_master_tx_rx(SPI_MASTER_0, data, num, 0, 0, g_spi_rx_buf, 0, 0, GPIO_SPI_0_CS_OLED);  
#endif
}

void OLED_power_on(void) 
{
#ifndef _CLING_PC_SIMULATION_
	nrf_gpio_pin_set( GPIO_OLED_RST ); // reset driven high
#endif
}

void OLED_power_off()
{
#ifndef _CLING_PC_SIMULATION_
	// Pull down reset pin
	nrf_gpio_pin_clear( GPIO_OLED_RST ); // reset driven high
#endif
}

void OLED_full_scree_show(I8U *pram)
{
#ifndef _CLING_PC_SIMULATION_
	I8U i;
	I8U *data;
	
	data = pram;
	
	for (i = 0; i < 4; i++) {
		_set_reg(0xb0+i);
		_set_reg(0x10);  // Starting address
		_set_reg(0x00);  // Bug fix - start from 0
		// Set "A0" pin for data buffer refresh
		nrf_gpio_pin_set(GPIO_OLED_A0);
		_set_data(128,data);
		
		data += 128;
	}
#endif
}

void OLED_im_show(OLED_DISPLAY_MODE mode, I8U *pram, I8U offset)
{
#ifndef _CLING_PC_SIMULATION_
	
		_set_reg(0xb0+offset);
		_set_reg(0x10);  // Starting address
		_set_reg(0x02);  // 
		// Set "A0" pin for data buffer refresh
		nrf_gpio_pin_set(GPIO_OLED_A0);
		_set_data(512,pram);
#endif
}

void OLED_set_display(I8U on_off)
{
	if (on_off)
		_set_reg(0xAF);
	else
		_set_reg(0xAE);
}

void OLED_set_contrast(I8U step)
{
	 _set_reg(0x81);
   _set_reg(step);
}

/**@brief Function for OLED init.
 *
 */

void OLED_init(I8U contrast)
{    
#ifndef _CLING_PC_SIMULATION_

    //BASE_delay_msec(100);
    //nrf_gpio_pin_set(GPIO_OLED_RST);	
    _set_reg(0xAE);     	
    _set_reg(0xd5);    
    _set_reg(0xC1);     
    _set_reg(0xA8);
    _set_reg(0x1F);
	  _set_reg(0xAD);
	  _set_reg(0x00);
		_set_reg(0x20);
	  _set_reg(0x02);

    _set_reg(0xD3);    
    _set_reg(0x00);
    _set_reg(0x40);     
	  _set_reg(0x8D);
	  _set_reg(0x8D);
	  _set_reg(0x14);

	   _set_reg(0xA0);
	   _set_reg(0xC8);
	   _set_reg(0xDA);
	   _set_reg(0x12);			
		   //_set_reg(0x81);
	     //_set_reg(0x40);				
		 OLED_set_contrast(contrast); 
		 _set_reg(0xD9);
	   _set_reg(0x22);			
     _set_reg(0xDB);    
     _set_reg(0x00);   
     _set_reg(0xA4);     
     _set_reg(0xA6);    
   // _set_reg(0xAF);  		
#endif
}

void OLED_set_panel_off()
{
	CLING_OLED_CTX *o = &cling.oled;
	
	o->state = OLED_STATE_GOING_OFF;
	cling.ui.display_active = FALSE;
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
	CLING_OLED_CTX *o = &cling.oled;
	
	if (o->state == OLED_STATE_IDLE) {
		return TRUE;
	}
	
	return FALSE;
}

#if 0
static I8U oledstate;
#endif
// This routine implements the main display state machine.  If someone wants to write something to the
// display, it first issues the command SSD1306_SetDisplay(
//static I8U oledstate;
void OLED_state_machine(void) 
{
#ifndef _CLING_PC_SIMULATION_
	I32U t_curr;
	CLING_OLED_CTX *o = &cling.oled;
	#if 0
	if (oledstate != o->state) {
		oledstate = o->state;
		Y_SPRINTF("[OLED] new oled state: %d @ %d", o->state, CLK_get_system_time());
	}
	#endif

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
			OLED_init(0xcc);
			//OLED_init(0x40);
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
				{
					// If screen is turned, dismiss the secondary reminder vibration 
					if ((cling.reminder.state >= REMINDER_STATE_ON) && (cling.reminder.state <= REMINDER_STATE_SECOND_REMINDER)) {
						UI_switch_state(UI_STATE_REMINDER, 0);
						cling.reminder.ui_hh = cling.time.local.hour;
						cling.reminder.ui_mm = cling.time.local.minute;
						cling.reminder.ui_alarm_on = TRUE; // Indicate this is a active alarm reminder
						Y_SPRINTF("[OLED] state reminder: %d, %d, %d", cling.reminder.state, cling.reminder.ui_hh, cling.reminder.ui_mm);
					}
				
					if (cling.notific.state != NOTIFIC_STATE_IDLE) {
#ifdef _ENABLE_ANCS_		
						if (cling.notific.cat_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) {
							UI_switch_state(UI_STATE_NOTIFIC, 0);
						}
#endif	
	}						
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


