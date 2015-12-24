/***************************************************************************/
/**
 * File: spi_master_api.c
 *
 * Description: Characters display function.
 *
 * Created on December 22, 2015
 *
 ******************************************************************************/
 
#include "main.h"
#include "nrf.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "nrf_error.h"
#include "nrf_drv_spi.h"

uint8_t g_spi_tx_buf[10];
uint8_t g_spi_rx_buf[10];

static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);

static BOOLEAN  m_transfer_completed = TRUE;

static void _spi_master_op_wait_done()
{
	if (m_transfer_completed)
		return;
	
	BASE_delay_msec(100);

	Y_SPRINTF("[SPI] transfer incompleted, wait for 100 ms");
}

/**@brief Handler for SPI0 master events.
 *
 * @param[in] event SPI master event.
 */
void spi_master_0_event_handler(nrf_drv_spi_event_t event)
{
	switch (event)
	{
		case NRF_DRV_SPI_EVENT_DONE:

				nrf_drv_spi_uninit(&m_spi_master_0);
				m_transfer_completed = TRUE;
				break;

		default:
				// No implementation needed.
				break;
	}
}

/**@brief Function for initializing a SPI master driver.
 */
void SPI_master_init(spi_master_hw_instance_t   spi_master_instance,I8U pin_cs, bool lsb)
{
	I32U err_code = NRF_SUCCESS;

	nrf_drv_spi_config_t config ;
	
	config.sck_pin      = SPIM0_SCK_PIN;
	config.mosi_pin     = SPIM0_MOSI_PIN;
	config.miso_pin     = SPIM0_MISO_PIN;
	config.ss_pin       = pin_cs;
	config.orc          = 0xCC;
	config.mode         = NRF_DRV_SPI_MODE_0;
	config.bit_order    = (lsb ?  NRF_DRV_SPI_BIT_ORDER_LSB_FIRST : NRF_DRV_SPI_BIT_ORDER_MSB_FIRST);

	if (OTA_if_enabled()){
	  config.irq_priority = APP_IRQ_PRIORITY_HIGH;
	  config.frequency    = NRF_DRV_SPI_FREQ_8M;
	}else{
		config.irq_priority = APP_IRQ_PRIORITY_LOW;
	  config.frequency    = NRF_DRV_SPI_FREQ_4M;
	}

	if (spi_master_instance == SPI_MASTER_0)
	{
		err_code = nrf_drv_spi_init(&m_spi_master_0, &config, spi_master_0_event_handler);
		
		if (err_code == NRF_ERROR_INVALID_STATE) {
				Y_SPRINTF("[SPI] Uninitialized - figure out why!");
				nrf_drv_spi_uninit(&m_spi_master_0);
				err_code = nrf_drv_spi_init(&m_spi_master_0, &config, spi_master_0_event_handler);
		}
		APP_ERROR_CHECK(err_code);
	}
}


/**@brief Function for sending and receiving data.
*/
void SPI_master_tx_rx(spi_master_hw_instance_t   spi_master_instance,
                          I8U * tx_data_buf, I16U tx_data_size,
                          I8U * rx_data_buf, I16U rx_data_size, I8U pin_cs)
{
  I32U err_code;

	_spi_master_op_wait_done();

	if(!m_transfer_completed){
		nrf_drv_spi_uninit(&m_spi_master_0);
		SPI_master_init(spi_master_instance,pin_cs,false);	
		Y_SPRINTF("[SPI] Reinitialize SPI due to uncompletion");
	}
	else{
	  	SPI_master_init(spi_master_instance,pin_cs,false);	
	}
  
	m_transfer_completed = FALSE;
	N_SPRINTF("[SPI] before");
  err_code = nrf_drv_spi_transfer(&m_spi_master_0,tx_data_buf, tx_data_size, rx_data_buf, rx_data_size );
	N_SPRINTF("[SPI] after");
  
  APP_ERROR_CHECK(err_code);
}
