/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "main.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "gpio.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "nrf_error.h"

uint8_t g_spi_tx_buf[10];
uint8_t g_spi_rx_buf[10];


void spi_master_disable()
{
	if (cling.system.b_spi_0_ON) {
		spi_master_close(SPI_MASTER_0);
		cling.system.b_spi_0_ON = FALSE;
	}
	
#if 0
	// disable SPI pins
	GPIO_spi_disabled(SPI_MASTER_0);
#ifdef SPI_MASTER_1_ENABLE
	GPIO_spi_disabled(SPI_MASTER_1);
#endif	
#endif
	N_SPRINTF("[SPI] deinit ...");
}

static BOOLEAN m_transfer_completed = TRUE;

void spi_master_0_event_handler(spi_master_evt_t spi_master_evt)
{
//    uint32_t err_code = NRF_SUCCESS;

    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            // Close SPI master.
            m_transfer_completed = TRUE;
            break;

        default:
            // No implementation needed.
            break;
    }
}

void spi_master_1_event_handler(spi_master_evt_t spi_master_evt)
{
    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            // Check if received data is correct.
            m_transfer_completed = TRUE;
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] spi_master_instance       An instance of SPI master module.
 * @param[in] spi_master_event_handler  An event handler for SPI master events.
 * @param[in] lsb                       Bits order LSB if true, MSB if false.
 */
uint32_t spi_master_init(spi_master_hw_instance_t   spi_master_instance,
										 spi_master_event_handler_t spi_master_event_handler,
                     BOOLEAN                 lsb)
{
    uint32_t err_code = NRF_SUCCESS;
	
    // Configure SPI master.
    spi_master_config_t spi_config = SPI_MASTER_INIT_DEFAULT;

    switch (spi_master_instance)
    {
        case SPI_MASTER_0:
        {
            spi_config.SPI_Pin_SCK  = SPIM0_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM0_MOSI_PIN;
        }
        break;
#if 0
        case SPI_MASTER_1:
        {
            spi_config.SPI_Pin_SCK  = SPIM1_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM1_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM1_MOSI_PIN;
        }
        break;
#endif
        default:
            break;
    }

		// Configure character order
    spi_config.SPI_CONFIG_ORDER = (lsb ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst);

		// Open SPI master instance, and configure all the interrupts
    err_code = spi_master_open(spi_master_instance, &spi_config);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    spi_master_evt_handler_reg(spi_master_instance, spi_master_event_handler);
		
		N_SPRINTF("[spi] INIT: %d", spi_master_instance);
		return err_code;
}


BOOLEAN spi_master_op_wait_done()
{
	I16U i;
	
	if (m_transfer_completed)
		return TRUE;

	// Wait for up to 500 ms
	for (i = 0; i < 5000; i++) {
		nrf_delay_us(100);
		
		if (m_transfer_completed)
			break;
	}

	Y_SPRINTF("[SPI] wait cycle: %d", i);
	
	if (m_transfer_completed)
		return TRUE;
	else
		return FALSE;
}

#define SPI_RETRYING_NUM 500
#define SPI_BLOCKING_NUM 50

void spi_master_tx_rx(const spi_master_hw_instance_t spi_master_hw_instance, 
	uint8_t *tx_cmd_buf, uint16_t tx_cmd_size, 
  uint8_t *tx_data_buf, uint16_t tx_data_size, 
  uint8_t *rx_data, uint16_t rx_cmd_size, uint16_t rx_data_size, 
	uint32_t pin_sel)
{
	I32U  err_code, i, t_diff, t_curr;

	if ((SPI_MASTER_0 == spi_master_hw_instance) && (!cling.system.b_spi_0_ON)) {
		
		N_SPRINTF("[SPI] re-init: %d", cling.system.b_spi_0_ON);
		// Enable SPI master
		spi_master_init(SPI_MASTER_0, spi_master_0_event_handler, FALSE);
		cling.system.b_spi_0_ON = TRUE;

		BASE_delay_msec(1);
	}
	

	m_transfer_completed = FALSE;

	// Start transfer.
	for(i=0; i<SPI_RETRYING_NUM; i++)
	{
		err_code=spi_master_send_recv(spi_master_hw_instance,  tx_cmd_buf, tx_cmd_size,
													 tx_data_buf, tx_data_size,rx_data, rx_cmd_size, rx_data_size, pin_sel);	
     
		if(err_code == NRF_SUCCESS) {
			N_SPRINTF("[SPI] tx rx error code: OK");
			break;
		}
		if(i == (SPI_RETRYING_NUM - 1)){
		Y_SPRINTF("[SPI] tx rx error code: %d", err_code);
			// Enable SPI master
		spi_master_init(SPI_MASTER_0, spi_master_0_event_handler, FALSE);
		cling.system.b_spi_0_ON = TRUE;		
		}
		BASE_delay_msec(1);	
	}
	
  if(err_code == NRF_ERROR_BUSY) 
		return;
	APP_ERROR_CHECK(err_code);
	
	// Check if there is a race condition
	if (m_transfer_completed)
		return;
	
	Y_SPRINTF("[SPI] tx rx somehow is not completed, start timer");

	// Block service - wait until we receive a finish event
	t_curr = CLK_get_system_time();
	while (!m_transfer_completed) {
		
		err_code = sd_app_evt_wait();
		APP_ERROR_CHECK(err_code);
	
		if (m_transfer_completed)
			break;
		
		t_diff = CLK_get_system_time() - t_curr;
		
		if (t_diff > 1000) {
			Y_SPRINTF("[SPI] tx rx time out: %d, %d", t_curr, t_diff);
			break;
		}
	}
}

#if 0
uint32_t* spi_master_init(SPIModuleNumber module_number, SPIMode mode, bool lsb_first)
{
  uint32_t config_mode;
	NRF_SPI_Type *spi_base_address = (SPI0 == module_number)? NRF_SPI0 : NRF_SPI1;

	//  As first action, disable the particular interface (in case currently TWI).  
	spi_base_address->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);

    if(SPI0 == module_number)
    {
        /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI1*/
        nrf_gpio_cfg_output(SPI_PSELSCK0);
        nrf_gpio_cfg_output(SPI_PSELMOSI0);
        nrf_gpio_cfg_input(SPI_PSELMISO0, NRF_GPIO_PIN_NOPULL);
				nrf_gpio_cfg_output(SPI_PSELSS0_NFLASH);

        /* Configure pins, frequency and mode */
        spi_base_address->PSELSCK  = SPI_PSELSCK0;
        spi_base_address->PSELMOSI = SPI_PSELMOSI0;
			spi_base_address->PSELMISO = SPI_PSELMISO0; // Not used
				nrf_gpio_pin_set(SPI_PSELSS0_NFLASH);
    }
    else
    {
        /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
			return (uint32_t *)spi_base_address;			
    }

    spi_base_address->FREQUENCY = (uint32_t) SPI_OPERATING_FREQUENCY;

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select mode] */
    switch (mode )
    {
       
        case SPI_MODE0:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE1:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE2:
            config_mode = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        case SPI_MODE3:
            config_mode = (SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos);
            break;
        default:
            config_mode = 0;
            break;
    
    }
    /** @snippet [SPI Select mode] */
    /*lint -restore */

    /*lint -e845 -save // A zero has been given as right argument to operator '!'" */
    /** @snippet [SPI Select endianess] */
    if (lsb_first)
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    else
    {
        spi_base_address->CONFIG = (config_mode | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos));
    }
    /** @snippet [SPI Select endianess] */
    /*lint -restore */

    spi_base_address->EVENTS_READY = 0U;

    /* Enable */
    spi_base_address->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    return (uint32_t *)spi_base_address;
}

static bool _double_buffer_rx_tx(NRF_SPI_Type *spi_base_address, uint16_t tx_size, uint8_t *tx_data, uint8_t *rx_data)
{
    uint32_t counter = 0;
    uint16_t number_of_txd_bytes = 0;
    uint16_t number_of_rxd_bytes = 0;
		uint8_t dummy_read;
	
		if (number_of_txd_bytes < tx_size) {
			if (tx_data == 0)
				spi_base_address->TXD = 0;
			else
				spi_base_address->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);
			number_of_txd_bytes++;
		}
		
		// If multi-word transfer, then load the second byte in the double buffer
		if (number_of_txd_bytes < tx_size) {
			if (tx_data == 0)
				spi_base_address->TXD = 0;
			else
				spi_base_address->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);
			number_of_txd_bytes++;
		}

    while(number_of_rxd_bytes < tx_size)
    {
        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while ((spi_base_address->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
        {
            counter++;
        }

        if (counter == TIMEOUT_COUNTER)
        {
            /* timed out, disable slave (slave select active low) and return with error */
            return false;
        }
        else
        {   /* clear the event to be ready to receive next messages */
            spi_base_address->EVENTS_READY = 0U;
        }

				if (rx_data == 0)
					dummy_read = (uint8_t)spi_base_address->RXD;
				else
					rx_data[number_of_rxd_bytes] = (uint8_t)spi_base_address->RXD;
        number_of_rxd_bytes++;

				if (number_of_txd_bytes < tx_size) {
					if (tx_data == 0)
						spi_base_address->TXD = 0;
					else
						spi_base_address->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);
					number_of_txd_bytes++;
				}
    }
		
		return true;
}

bool spi_master_multi_tx(uint32_t *base_address, uint32_t cs_pin, uint16_t transfer_size, uint16_t cmd_size, uint8_t *tx_cmd, uint8_t *tx_data)
{
		
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(cs_pin);

		if (!_double_buffer_rx_tx((NRF_SPI_Type *) base_address, cmd_size, tx_cmd, 0)) {
			nrf_gpio_pin_set(cs_pin);
			return false;
		}

		if (!_double_buffer_rx_tx((NRF_SPI_Type *) base_address, transfer_size, tx_data, 0)) {
			nrf_gpio_pin_set(cs_pin);
			return false;
		}

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(cs_pin);

    return true;
}

bool spi_master_multi_rx(uint32_t *base_address, uint32_t cs_pin, uint16_t transfer_size, uint16_t cmd_size, uint8_t *tx_data, uint8_t *rx_data)
{
		
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(cs_pin);
	
		if (!_double_buffer_rx_tx((NRF_SPI_Type *)base_address, cmd_size, tx_data, 0)) {
			nrf_gpio_pin_set(cs_pin);
			return false;
		}

		if (!_double_buffer_rx_tx((NRF_SPI_Type *)base_address, transfer_size, 0, rx_data)) {
			nrf_gpio_pin_set(cs_pin);
			return false;
		}

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(cs_pin);

    return true;
}
#endif



