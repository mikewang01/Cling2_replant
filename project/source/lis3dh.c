/***************************************************************************/
/** @file lis3dh.c
 *  @brief Driver implementation for ST lis3dh accelerometer.
 *
 *  Created on December 22, 2015
 *
 ******************************************************************************/
 
#include "main.h"

extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];


static void _set_reg(I8U reg_idx, I8U config)
{	
	g_spi_tx_buf[0] = reg_idx;
	g_spi_tx_buf[1] = config;
#ifndef _CLING_PC_SIMULATION_
	N_SPRINTF("[spi] 13");
	SPI_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 2, g_spi_rx_buf,  0, GPIO_SPI_0_CS_ACC);
#endif
}

static void _get_reg(I8U reg_idx)
{	
	g_spi_tx_buf[0] = reg_idx | 0x80;
	g_spi_tx_buf[1] = 0;
#ifndef _CLING_PC_SIMULATION_
	N_SPRINTF("[spi] 12");
	SPI_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 2,  g_spi_rx_buf,  2, GPIO_SPI_0_CS_ACC);
#endif
}

static void _get_data(I8U reg_idx)
{	
	g_spi_tx_buf[0] = reg_idx | 0xc0;
#ifndef _CLING_PC_SIMULATION_
	N_SPRINTF("[spi] 11");
	SPI_master_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 7, g_spi_rx_buf, 7, GPIO_SPI_0_CS_ACC);
#endif
}

I8U LIS3DH_who_am_i()
{	
	_get_reg(WHO_AM_I);
	
	return g_spi_rx_buf[1];
}

EN_STATUSCODE LIS3DH_normal_FIFO()
{
	if (cling.user_data.b_screen_tapping) 
	{
		// Tap detection
		// Tap configuration: enable interrupt doble tap-tap on X/Y/Z axis
		_set_reg(TAP_CFG, 0x3F);
		
		// Tap src ... (status check)
		
		// Tap threshold
		// 0x29 - 0.03125*48 = 1.5 g
		_set_reg(TAP_THS, 0x30);
		
		// Tap time limit
		// 320 ms - 16 x 20 ms (50 Hz)
		_set_reg(TAP_TIME_LIMIT, 0x10);
		
		// Tap time latency
		// 200 ms - 10 x 20 ms (50 Hz)
		_set_reg(TAP_TIME_LATENCY, 0x0A);
		
		// Tap time window
		// 640 ms - 32 x 20 ms (50 Hz)
		_set_reg(TAP_TIME_WINDOW, 0x20);
	}

	/* INT1_THS */
	_set_reg(INT_THS1, 0x00);

	/* INT1_DURATION */
	_set_reg(INT_DUR1, 0x00);
	
	/* CTRL_REG1 */
	// Low power mode, X/Y/Z all enabled, 50 Hz
	_set_reg(CTRL_REG1, 0x47);

	/* CTRL_REG2 */
	// default
	_set_reg(CTRL_REG2, 0x00);
	
	if (cling.user_data.b_screen_tapping) {
		N_SPRINTF("[LIS3DH] tapping interrupt: 0x84");

		/* CTRL_REG3 */
		// Disable data ready pin, enable FIFO watermak interrupt
		_set_reg(CTRL_REG3, 0x84);
	} else {
		N_SPRINTF("[LIS3DH] tapping interrupt: 0x04");
		_set_reg(CTRL_REG3, 0x04);
	}

	/* CTRL_REG4 */
	// LSB @ lower address, Full scale selection at +/- 4G, High resolution enabled
	_set_reg(CTRL_REG4, 0x18);

	/* CTRL_REG5 */
	// Enable FIFO, Latch interrupt request
	_set_reg(CTRL_REG5, 0x48);

	/* FIFO_CTRL_REG */
	// FIFO in streaming mode, water mark: 25 samples -> 500 ms
	// Note: FIFO has to be enabled first before configuring FIFO register
	_set_reg(FIFO_CTRL_REG, 0x99);
	
	/* CTRL_REG6 */
	// 0: Interrupt active HIGH, 1: Interrupt active LOW
	_set_reg(CTRL_REG6, 0x00);
	
	return STATUSCODE_SUCCESS;
}

EN_STATUSCODE LIS3DH_init()
{	

	return LIS3DH_normal_FIFO();
}

EN_STATUSCODE LIS3DH_inertial_wake_up_init()
{
#ifndef _CLING_PC_SIMULATION_
	/* CTRL_REG1 */
	// Low power mode, X/Y/Z all enabled, 50 Hz
	_set_reg(CTRL_REG1, 0x47);
	BASE_delay_msec(1);
	
	/* CTRL_REG2 */
	// High pass filter enabled on data and interrupt
	_set_reg(CTRL_REG2, 0x09);
	
	/* CTRL_REG3 */
	// Interrupt driven to INT1, enable FIFO watermark
	_set_reg(CTRL_REG3, 0x40);
	
	/* CTRL_REG4 */
	// FS = 2 g
	_set_reg(CTRL_REG4, 0x00);

	/* CTRL_REG5 */
	// Enable FIFO, Latch interrupt request
	_set_reg(CTRL_REG5, 0x48);

	/* INT1_THS, 16*8 = 128 mg */
	_set_reg(INT_THS1, 0x08);

	/* INT1_DURATION */
	_set_reg(INT_DUR1, 0x01);
	
	// Dummy read to force the HP filter to current acceleration value
	_get_reg(REF_DATACAPTURE);
	
	/* INT CFG */
	_set_reg(INT_CFG1, 0x2a);
	
	/* Clear interrupt */
	_get_reg(INT_SRC1);
#endif
	return STATUSCODE_SUCCESS;
}

EN_STATUSCODE LIS3DH_close()
{
	/* CTRL_REG1 */
	// Low power mode, X/Y/Z all enabled, 50 Hz
	_set_reg(CTRL_REG1, 0x08);

	return STATUSCODE_SUCCESS;
}

I8U LIS3DH_get_tap()
{
	_get_reg(TAP_SRC);
	
	return g_spi_rx_buf[1];
}

I8U LIS3DH_get_interrupt()
{
	// Read interrupt 
	_get_reg(INT_SRC1);
	
	N_SPRINTF("[LIS3DH] int: %02x, %d", g_spi_rx_buf[1], CLK_get_system_time());
	
	return g_spi_rx_buf[1];
}

I8U LIS3DH_is_FIFO_ready()
{
	I8U fss = 0;
	
	_get_reg(FIFO_SRC_REG);
	
	fss = g_spi_rx_buf[1] & 0x1f;

	// Check ZYXDA bit is valid (a new set of data) 
	//
#if 0
	if (g_spi_rx_buf[1] & 0x80) {
		Y_SPRINTF("[LIS3DH] FIFO WATER MARK samples: %d", fss);
	} else {
		Y_SPRINTF("[LIS3DH] FIFO normal samples: %d", fss);
	}
#endif
#ifdef _SLEEP_SIMULATION_
	return 1;
#else
	return fss;
#endif
}

void LIS3DH_retrieve_data(ACC_AXIS *xyz)
{
	// Read acceleration data
	_get_data(0x28);
#ifdef _SLEEP_SIMULATION_
	SIM_get_accelerometer(&xyz->x, &xyz->y, &xyz->z);
#else
	// Get all 3-axis data
	memcpy(xyz, g_spi_rx_buf+1, 6);
#endif
	N_SPRINTF("[LIS3DH] data: %04x,%04x,%04x,", xyz->x, xyz->y, xyz->z);
	N_SPRINTF("[LIS3DH] data: %d,%d,%d,", xyz->x, xyz->y, xyz->z);
	N_SPRINTF("[LIS3DH] data: %02x,%02x,%02x,%02x,%02x,%02x,", g_spi_rx_buf[1], g_spi_rx_buf[2], g_spi_rx_buf[3], g_spi_rx_buf[4], g_spi_rx_buf[5], g_spi_rx_buf[6]);
}

BOOLEAN LIS3DH_is_data_ready(ACC_AXIS *xyz)
{
	_get_reg(STATUS_REG);
	
	// Check ZYXDA bit is valid (a new set of data) 
	//
	if (!(g_spi_rx_buf[1] & 0x08)) {
		Y_SPRINTF("[LIS3DH] something else happened");
		return FALSE;
	}
	
	N_SPRINTF("[LIS3DH] status: %02x, %d", g_spi_rx_buf[1], CLK_get_system_time());
	
	// Read acceleration data
	_get_data(0x28);
	
	// Get all 3-axis data
	memcpy(xyz, g_spi_rx_buf+1, 6);
	
	//nrf_gpio_pin_set(27);
	
	N_SPRINTF("[LIS3DH] data: %04x,%04x,%d,", xyz->x, xyz->y, xyz->z);
	N_SPRINTF("[LIS3DH] data: %d,%d,%d,", xyz->x, xyz->y, xyz->z);
	// 
	// sample data: (low, high), (low, high), (low, high)
	//
	N_SPRINTF("[LIS3DH] data: %02x,%02x,%02x,%02x,%02x,%02x,", g_spi_rx_buf[1], g_spi_rx_buf[2], g_spi_rx_buf[3], g_spi_rx_buf[4], g_spi_rx_buf[5], g_spi_rx_buf[6]);
	return TRUE;
}

