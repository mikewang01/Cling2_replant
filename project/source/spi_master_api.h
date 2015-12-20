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

#ifndef _CLING_PC_SIMULATION_
#include "nrf_drv_spi.h"
#include "standard_types.h"
#include "nordic_common.h"

//#define SPI_MASTER_0_ENABLE

#define SPIM0_SCK_PIN            GPIO_SPI_0_SCK                                     /**< SPI clock GPIO pin number. */
#define SPIM0_MOSI_PIN           GPIO_SPI_0_MOSI                                     /**< SPI Master Out Slave In GPIO pin number. */
#define SPIM0_MISO_PIN           GPIO_SPI_0_MISO                                     /**< SPI Master In Slave Out GPIO pin number. */

/** @brief Enum containing instances of the SPI master driver. */
typedef enum
{
     #ifdef SPI_MASTER_0_ENABLE
    SPI_MASTER_0,   /**< A instance of NRF_SPI0. */
     #endif

     #ifdef SPI_MASTER_1_ENABLE
    SPI_MASTER_1,   /**< A instance of NRF_SPI1. */
     #endif

    SPI_MASTER_HW_ENABLED_COUNT /**< A number of enabled instances of the SPI master. */
} spi_master_hw_instance_t;


void SPI_master_init(spi_master_hw_instance_t   spi_master_instance,I8U pin_cs, bool lsb);

void SPI_master_tx_rx(spi_master_hw_instance_t   spi_master_instance,
                          I8U * tx_data_buf, I16U tx_data_size,
                          I8U * rx_data_buf, I16U rx_data_size,I8U pin_cs);

#endif

#endif // SPI_MASTER_API_H__
