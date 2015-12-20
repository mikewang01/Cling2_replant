/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf51_bitfields.h"
#include "spi_master.h"
#include "main.h"
#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

typedef struct
{
    NRF_SPI_Type * p_nrf_spi;   /**< A pointer to the NRF SPI master */
    IRQn_Type irq_type;         /**< A type of NVIC IRQn */

    uint8_t * p_tx_cmd;      /**< A pointer to TX buffer. */
    uint16_t tx_cmd_length;         /**< A length of TX buffer. */
    uint8_t * p_tx_data;      /**< A pointer to TX buffer. */
    uint16_t tx_data_length;         /**< A length of TX buffer. */
    uint16_t tx_cmd_index;          /**< A index of the current element in the TX buffer. */
    uint16_t tx_data_index;          /**< A index of the current element in the TX buffer. */
    uint16_t tx_overall_index;          /**< A index of the current element in the TX buffer. */

    uint8_t * p_rx_cmd;      /**< A pointer to RX buffer. */
    uint16_t rx_cmd_length;         /**< A length RX buffer. */
    uint8_t * p_rx_data;      /**< A pointer to RX buffer. */
    uint16_t rx_data_length;         /**< A length RX buffer. */
    uint16_t rx_cmd_index;          /**< A index of the current element in the RX buffer. */
    uint16_t rx_data_index;          /**< A index of the current element in the RX buffer. */

    uint16_t max_length;        /**< Max length (Max of the TX and RX length). */
    uint16_t bytes_count;
    uint8_t pin_slave_select;   /**< A pin for Slave Select. */

    spi_master_event_handler_t callback_event_handler;  /**< A handler for event callback function. */

    spi_master_state_t state;   /**< A state of an instance of SPI master. */
    bool started_flag;
    bool disable_all_irq;

} spi_master_instance_t;

#define _static static

_static volatile spi_master_instance_t m_spi_master_instances[SPI_MASTER_HW_ENABLED_COUNT];

/* Function prototypes */
static __INLINE volatile spi_master_instance_t * spi_master_get_instance(
    const spi_master_hw_instance_t spi_master_hw_instance);
static __INLINE void spi_master_send_recv_irq(volatile spi_master_instance_t * const p_spi_instance);

#endif //SPI_MASTER_0_ENABLE || SPI_MASTER_1_ENABLE

#ifdef SPI_MASTER_0_ENABLE
/**
 * @brief SPI0 interrupt handler.
 */
void SPI0_TWI0_IRQHandler(void)
{
    if ((NRF_SPI0->EVENTS_READY == 1) && (NRF_SPI0->INTENSET & SPI_INTENSET_READY_Msk))
    {
        NRF_SPI0->EVENTS_READY = 0;

        volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(SPI_MASTER_0);

        spi_master_send_recv_irq(p_spi_instance);
    }
}
#endif //SPI_MASTER_0_ENABLE

#ifdef SPI_MASTER_1_ENABLE
/**
 * @brief SPI0 interrupt handler.
 */
void SPI1_TWI1_IRQHandler(void)
{
    if ((NRF_SPI1->EVENTS_READY == 1) && (NRF_SPI1->INTENSET & SPI_INTENSET_READY_Msk))
    {
        NRF_SPI1->EVENTS_READY = 0;

        volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(SPI_MASTER_1);

        spi_master_send_recv_irq(p_spi_instance);
    }
}
#endif //SPI_MASTER_1_ENABLE

#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
/**
 * @breif Function for getting an instance of SPI master.
 */
static __INLINE volatile spi_master_instance_t * spi_master_get_instance(
    const spi_master_hw_instance_t spi_master_hw_instance)
{
    if (spi_master_hw_instance < SPI_MASTER_HW_ENABLED_COUNT)
    {
        return &(m_spi_master_instances[(uint8_t)spi_master_hw_instance]);
    }
    return NULL;
}

/**
 * @brief Function for initializing instance of SPI master by default values.
 */
static __INLINE void spi_master_init_hw_instance(NRF_SPI_Type *                   p_nrf_spi,
                                                 IRQn_Type                        irq_type,
                                                 volatile spi_master_instance_t * p_spi_instance,
                                                 volatile bool                    disable_all_irq)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    p_spi_instance->p_nrf_spi = p_nrf_spi;
    p_spi_instance->irq_type  = irq_type;

    p_spi_instance->p_tx_data = NULL;
    p_spi_instance->p_tx_cmd = NULL;
    p_spi_instance->tx_cmd_length   = 0;
    p_spi_instance->tx_data_length   = 0;
    p_spi_instance->tx_cmd_index    = 0;
    p_spi_instance->tx_data_index    = 0;

    p_spi_instance->p_rx_data = NULL;
    p_spi_instance->rx_data_length   = 0;
    p_spi_instance->rx_data_index    = 0;
    p_spi_instance->p_rx_cmd = NULL;
    p_spi_instance->rx_cmd_length   = 0;
    p_spi_instance->rx_cmd_index    = 0;

    p_spi_instance->bytes_count      = 0;
    p_spi_instance->max_length       = 0;
    p_spi_instance->pin_slave_select = 0;

    p_spi_instance->callback_event_handler = NULL;

    p_spi_instance->state           = SPI_MASTER_STATE_DISABLED;
    p_spi_instance->started_flag    = false;
    p_spi_instance->disable_all_irq = disable_all_irq;
}

/**
 * @brief Function for releasing TX or RX buffer.
 */
static __INLINE void spi_master_buffer_release(uint8_t * volatile * const pp_cmd,
                                               uint8_t * volatile * const pp_data,
                                               volatile uint16_t * const  p_cmd_len,
                                               volatile uint16_t * const  p_data_len)
{
    APP_ERROR_CHECK_BOOL(pp_cmd != NULL);
    APP_ERROR_CHECK_BOOL(pp_data != NULL);
    APP_ERROR_CHECK_BOOL(p_cmd_len != NULL);
    APP_ERROR_CHECK_BOOL(p_data_len != NULL);

    *pp_cmd    = NULL;
    *pp_data    = NULL;
    *p_cmd_len = 0;
    *p_data_len = 0;
}

/**
 * @brief Function for sending events by callback.
 */
static __INLINE void spi_master_signal_evt(volatile spi_master_instance_t * const p_spi_instance,
                                           spi_master_evt_type_t                  event_type,
                                           const uint16_t                         data_count)
{
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    if (p_spi_instance->callback_event_handler != NULL)
    {
        spi_master_evt_t event = {SPI_MASTER_EVT_TYPE_MAX, 0};
        event.evt_type   = event_type;
        event.data_count = data_count;
        p_spi_instance->callback_event_handler(event);
    }
}

/**
 * @brief Function insert to a TX buffer another byte or two bytes (depends on flag @ref DOUBLE_BUFFERED).
 */
static __INLINE void spi_master_send_initial_bytes(
    volatile spi_master_instance_t * const p)
{
    APP_ERROR_CHECK_BOOL(p != NULL);
    
    p->tx_overall_index = 0;
	
		if ((p->p_tx_cmd != NULL) && (p->tx_cmd_index  < p->tx_cmd_length)) {
			p->p_nrf_spi->TXD = p->p_tx_cmd[p->tx_cmd_index++];
		} else if ((p->p_tx_data != NULL) && (p->tx_data_index  < p->tx_data_length)) {
			p->p_nrf_spi->TXD = p->p_tx_data[p->tx_data_index++];
		} else {
			p->p_nrf_spi->TXD = SPI_DEFAULT_TX_BYTE;
		}
		
		p->tx_overall_index++;
		
    if (p->tx_overall_index < p->max_length)
    {
			if ((p->p_tx_cmd != NULL) && (p->tx_cmd_index  < p->tx_cmd_length)) {
				p->p_nrf_spi->TXD = p->p_tx_cmd[p->tx_cmd_index++];
			} else if ((p->p_tx_data != NULL) && (p->tx_data_index  < p->tx_data_length)) {
				p->p_nrf_spi->TXD = p->p_tx_data[p->tx_data_index++];
			} else {
				p->p_nrf_spi->TXD = SPI_DEFAULT_TX_BYTE;
			}
			p->tx_overall_index++;
		}
		
}

/**
 * @brief Function for receiving and sending data from IRQ. (The same for both IRQs).
 */
static __INLINE void spi_master_send_recv_irq(volatile spi_master_instance_t * const p)
{
    APP_ERROR_CHECK_BOOL(p != NULL);
    
    p->bytes_count++;

    if (!p->started_flag)
    {
        p->started_flag = true;
        spi_master_signal_evt(p,
                              SPI_MASTER_EVT_TRANSFER_STARTED,
                              p->bytes_count);
    }
    
    uint8_t rx_byte = p->p_nrf_spi->RXD;
    
		if (p->rx_cmd_index < p->rx_cmd_length) {
			p->rx_cmd_index ++;
		} else if ((p->p_rx_data != NULL) && (p->rx_data_index < p->rx_data_length)) {
			p->p_rx_data[p->rx_data_index++] = rx_byte;
    }
        
    if (p->tx_overall_index < p->max_length)
    {
			if ((p->p_tx_cmd != NULL) && (p->tx_cmd_index  < p->tx_cmd_length)) {
				p->p_nrf_spi->TXD = p->p_tx_cmd[p->tx_cmd_index++];
			} else if ((p->p_tx_data != NULL) && (p->tx_data_index  < p->tx_data_length)) {
				p->p_nrf_spi->TXD = p->p_tx_data[p->tx_data_index++];
			} else {
				p->p_nrf_spi->TXD = SPI_DEFAULT_TX_BYTE;
			}			
			p->tx_overall_index++;
    }

    if (p->bytes_count >= p->max_length)
    {
        nrf_gpio_pin_set(p->pin_slave_select);

        uint16_t transmited_bytes = p->tx_cmd_index+p->tx_data_index;

        spi_master_buffer_release(&(p->p_tx_cmd), &(p->p_tx_data),
																&(p->tx_cmd_length), &(p->tx_data_length));
        spi_master_buffer_release(&(p->p_rx_cmd), &(p->p_rx_data),
																&(p->rx_cmd_length), &(p->rx_data_length));

        p->state = SPI_MASTER_STATE_IDLE;

        spi_master_signal_evt(p, SPI_MASTER_EVT_TRANSFER_COMPLETED, transmited_bytes);
   }
}
#endif //defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
uint32_t spi_master_open(const spi_master_hw_instance_t    spi_master_hw_instance,
                         spi_master_config_t const * const p_spi_master_config)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    /* Check against null */
    if (p_spi_master_config == NULL)
    {
        return NRF_ERROR_NULL;
    }

    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    switch (spi_master_hw_instance)
    {
    #ifdef SPI_MASTER_0_ENABLE
        case SPI_MASTER_0:
            spi_master_init_hw_instance(NRF_SPI0, SPI0_TWI0_IRQn, p_spi_instance, p_spi_master_config->SPI_DisableAllIRQ);
            break;
    #endif /* SPI_MASTER_0_ENABLE */

    #ifdef SPI_MASTER_1_ENABLE
        case SPI_MASTER_1:
            spi_master_init_hw_instance(NRF_SPI1, SPI1_TWI1_IRQn, p_spi_instance, p_spi_master_config->SPI_DisableAllIRQ);
            break;
    #endif /* SPI_MASTER_1_ENABLE */

        default:
            break;
    }

    //Configure GPIO
    nrf_gpio_cfg_output(p_spi_master_config->SPI_Pin_SCK);
    nrf_gpio_cfg_output(p_spi_master_config->SPI_Pin_MOSI);
    nrf_gpio_cfg_input(p_spi_master_config->SPI_Pin_MISO, NRF_GPIO_PIN_NOPULL);

    /* Configure SPI hardware */
    p_spi_instance->p_nrf_spi->PSELSCK  = p_spi_master_config->SPI_Pin_SCK;
    p_spi_instance->p_nrf_spi->PSELMOSI = p_spi_master_config->SPI_Pin_MOSI;
    p_spi_instance->p_nrf_spi->PSELMISO = p_spi_master_config->SPI_Pin_MISO;

    p_spi_instance->p_nrf_spi->FREQUENCY = p_spi_master_config->SPI_Freq;

    p_spi_instance->p_nrf_spi->CONFIG =
        (uint32_t)(p_spi_master_config->SPI_CONFIG_CPHA << SPI_CONFIG_CPHA_Pos) |
        (p_spi_master_config->SPI_CONFIG_CPOL << SPI_CONFIG_CPOL_Pos) |
        (p_spi_master_config->SPI_CONFIG_ORDER << SPI_CONFIG_ORDER_Pos);

    /* Clear waiting interrupts and events */
    p_spi_instance->p_nrf_spi->EVENTS_READY = 0;

    APP_ERROR_CHECK(sd_nvic_ClearPendingIRQ(p_spi_instance->irq_type));
    APP_ERROR_CHECK(sd_nvic_SetPriority(p_spi_instance->irq_type, p_spi_master_config->SPI_PriorityIRQ));

    /* Clear event handler */
    p_spi_instance->callback_event_handler = NULL;

    /* Enable interrupt */
    p_spi_instance->p_nrf_spi->INTENSET = (SPI_INTENSET_READY_Set << SPI_INTENCLR_READY_Pos);
    APP_ERROR_CHECK(sd_nvic_EnableIRQ(p_spi_instance->irq_type));

    /* Enable SPI hardware */
    p_spi_instance->p_nrf_spi->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    /* Change state to IDLE */
    p_spi_instance->state = SPI_MASTER_STATE_IDLE;

    return NRF_SUCCESS;
    #else
    return NRF_ERROR_NOT_SUPPORTED;
    #endif
}

void spi_master_close(const spi_master_hw_instance_t spi_master_hw_instance)
{
#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);
		
    /* Disable interrupt */
    APP_ERROR_CHECK(sd_nvic_ClearPendingIRQ(p_spi_instance->irq_type));
    APP_ERROR_CHECK(sd_nvic_DisableIRQ(p_spi_instance->irq_type));

    p_spi_instance->p_nrf_spi->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);

    /* Set Slave Select pin as input with pull-up. */
    nrf_gpio_pin_set(p_spi_instance->pin_slave_select);
    nrf_gpio_cfg_input(p_spi_instance->pin_slave_select, NRF_GPIO_PIN_PULLUP);
    p_spi_instance->pin_slave_select = (uint8_t)0xFF;

    /* Disconnect pins from SPI hardware */
    p_spi_instance->p_nrf_spi->PSELSCK  = (uint32_t)SPI_PIN_DISCONNECTED;
    p_spi_instance->p_nrf_spi->PSELMOSI = (uint32_t)SPI_PIN_DISCONNECTED;
    p_spi_instance->p_nrf_spi->PSELMISO = (uint32_t)SPI_PIN_DISCONNECTED;

    /* Reset to default values */
    spi_master_init_hw_instance(NULL, (IRQn_Type)0, p_spi_instance, false);
    #else
    return;
    #endif
}

__INLINE spi_master_state_t spi_master_get_state(
    const spi_master_hw_instance_t spi_master_hw_instance)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    return p_spi_instance->state;
    #else
    return SPI_MASTER_STATE_DISABLED;
    #endif
}

__INLINE void spi_master_evt_handler_reg(const spi_master_hw_instance_t spi_master_hw_instance,
                                         spi_master_event_handler_t     event_handler)
{
    #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
    volatile spi_master_instance_t * p_spi_instance = spi_master_get_instance(
        spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p_spi_instance != NULL);

    p_spi_instance->callback_event_handler = event_handler;
    #else
    return;
    #endif
}

uint32_t spi_master_send_recv(const spi_master_hw_instance_t spi_master_hw_instance,
                              uint8_t * const p_tx_cmd, const uint16_t tx_cmd_len,
                              uint8_t * const p_tx_data, const uint16_t tx_data_len,
                              uint8_t * const p_rx_data, const uint16_t rx_cmd_len,
															const uint16_t rx_data_len,
															uint32_t spi_pin_sel)
{
#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

    volatile spi_master_instance_t * p = spi_master_get_instance(spi_master_hw_instance);
    APP_ERROR_CHECK_BOOL(p != NULL);

    uint32_t err_code   = NRF_SUCCESS;
    uint16_t max_length = 0;
		uint16_t rx_buf_len, tx_buf_len;
    
    uint8_t nested_critical_region = 0;
		
		rx_buf_len = rx_cmd_len + rx_data_len;
		tx_buf_len = tx_cmd_len + tx_data_len;

    //Check if disable all IRQs flag is set
    if (p->disable_all_irq)
    {
        //Disable interrupts.
        APP_ERROR_CHECK(sd_nvic_critical_region_enter(&nested_critical_region));
    }
    else
    {
        //Disable interrupt SPI.
			for(I16U i=0;i<5000;i++)
			{
			  err_code=sd_nvic_DisableIRQ(p->irq_type);
				if(err_code == NRF_SUCCESS) break;
				else   nrf_delay_us(100);	
				
				if(i == 4999)
				Y_SPRINTF("[SPI] 111  disable spi interrupt fail");
				 return NRF_ERROR_BUSY;	
			}
			//APP_ERROR_CHECK(err_code);
        //APP_ERROR_CHECK(sd_nvic_DisableIRQ(p->irq_type));
			
    }

    //Initialize and perform data transfer
    if (p->state == SPI_MASTER_STATE_IDLE)
    {
        max_length = (rx_buf_len > tx_buf_len) ? rx_buf_len : tx_buf_len;

        if (max_length > 0)
        {
            p->state        = SPI_MASTER_STATE_BUSY;
            p->bytes_count  = 0;
            p->started_flag = false;
            p->max_length   = max_length;

						/* Initialize buffers */
						p->p_tx_cmd = p_tx_cmd;
						p->tx_cmd_length = tx_cmd_len;
						p->p_tx_data = p_tx_data;
						p->tx_data_length = tx_data_len;
						p->tx_cmd_index = 0;
						p->tx_data_index = 0;

						p->p_rx_cmd = 0;
						p->rx_cmd_length = rx_cmd_len;
						p->p_rx_data = p_rx_data;
						p->rx_data_length = rx_data_len;
						p->rx_cmd_index = 0;
						p->rx_data_index = 0;

						//A Slave select must be set as high before setting it as output,
						//because during connect it to the pin it causes glitches.
						nrf_gpio_pin_set(spi_pin_sel);
						nrf_gpio_cfg_output(spi_pin_sel);
            nrf_gpio_pin_clear(spi_pin_sel);
						
						p->pin_slave_select = spi_pin_sel;

            spi_master_send_initial_bytes(p);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_PARAM;
        }
    }
    else
    {
        err_code = NRF_ERROR_BUSY;
    }
    
    //Check if disable all IRQs flag is set.
    if (p->disable_all_irq)
    {   
        //Enable interrupts.
        APP_ERROR_CHECK(sd_nvic_critical_region_exit(nested_critical_region));
    }
    else
    {
        //Enable SPI interrupt.
			for(I16U k=0;k<5000;k++)
			{
			  err_code=sd_nvic_EnableIRQ(p->irq_type);
				if(err_code == NRF_SUCCESS) break;
				else   nrf_delay_us(100);	
				
				if(k == 4999)
				Y_SPRINTF("[SPI] 111  enable spi interrupt fail");
        return NRF_ERROR_BUSY;						
			}
			//APP_ERROR_CHECK(err_code);
       // APP_ERROR_CHECK(sd_nvic_EnableIRQ(p->irq_type));
    }

    return err_code;
    #else
    return NRF_ERROR_NOT_SUPPORTED;
    #endif
}
