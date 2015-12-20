/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

/**@file
 *
 * @defgroup spi_master_example_main spi_master_config
 * @{
 * @ingroup spi_master_example
 *
 * @brief Defintions needed for the SPI master driver.
 */
 
#ifndef SPI_MASTER_API_H__
#define SPI_MASTER_API_H__

#define SPI_OPERATING_FREQUENCY (0x02000000uL << (uint32_t)Freq_8Mbps)  /**< Slave clock frequency. */
#if 0
// SPI0. 
#define SPIM1_SCK_PIN            GPIO_SPI_1_SCK                                     /**< SPI clock GPIO pin number. */
#define SPIM1_MOSI_PIN           GPIO_SPI_1_MOSI                                     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM1_MISO_PIN           GPIO_SPI_1_MISO                                     /**< SPI Master In Slave Out GPIO pin number. */
#endif

// 
#define SPIM0_SCK_PIN            GPIO_SPI_0_SCK                                     /**< SPI clock GPIO pin number. */
#define SPIM0_MOSI_PIN           GPIO_SPI_0_MOSI                                     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM0_MISO_PIN           GPIO_SPI_0_MISO                                     /**< SPI Master In Slave Out GPIO pin number. */

#define SPI_PSELSS0_OLED        GPIO_SPI_0_CS_OLED                                     /**< SPI Slave Select GPIO pin number. */
#define SPI_PSELSS0_ACC         GPIO_SPI_0_CS_ACC                                     /**< SPI Slave Select GPIO pin number. */
#define SPI_PSELSS0_PPG         GPIO_SPI_0_CS_PPG                               /**< SPI Slave Select GPIO pin number. */
#define SPI_PSELSS0_NFLASH      GPIO_SPI_0_CS_NFLASH

#define TIMEOUT_COUNTER         0x3000uL                                /**< Timeout for SPI transaction in units of loop iterations. */

#ifndef _CLING_PC_SIMULATION_
#include "spi_master.h"

uint32_t spi_master_init(spi_master_hw_instance_t   spi_master_instance,
                            spi_master_event_handler_t spi_master_event_handler,
                            uint8_t                 lsb);
void spi_master_tx_rx(const spi_master_hw_instance_t spi_master_hw_instance, 
	uint8_t *tx_cmd_buf, uint16_t tx_cmd_size, 
  uint8_t *tx_data_buf, uint16_t tx_data_size, 
  uint8_t *rx_data, uint16_t rx_cmd_size, uint16_t rx_data_size, 
	uint32_t pin_sel);
void spi_master_0_event_handler(spi_master_evt_t spi_master_evt);
void spi_master_1_event_handler(spi_master_evt_t spi_master_evt);
void spi_master_disable(void);
BOOLEAN spi_master_op_wait_done(void);
#endif

#endif // SPI_MASTER_API_H__
