/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : file_check_crc_port.h
 * Author        : MikeWang
 * Date          : 2015-11-17
 * Description   : file_check_crc.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2015-11-17
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/
#ifndef __FILE_CHECK_PORT_CRC_H__
#define __FILE_CHECK_PORT_CRC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*to tell system if fiel system is supported or not*/
#define __IS_FS_SUPPORED__ TRUE
	
	
	
#if (__IS_FS_SUPPORED__ == TRUE)
#include "fs_file.h"
#endif

	
#include "stdint.h"
#include "nflash_spi.h"
#include "sysflash_rw.h"
#include "main.h"

/*PRINTF FUNCTIONS PORTING*/
#define CRC_PRINTF Y_SPRINTF
	
#if (__IS_FS_SUPPORED__ == TRUE)
/*file realted port function*/
#define FILE_OPEN(__x, __y, __z)
#define FILE_READ(__x, __y, __z)   FILE_fread(__x, __y, __z)//(FILE_CTX *fc, I8U * buffer, I8U count);
#define FILE_WRITE(__x, __y, __z)
#define MAX_SIZE_READ_ONE_TIME  128
#define FILE_T   FILE_CTX
#endif
	

/*spi flash realated porting function*/
#define SPI_FlASH_READ(__x, __y, __z)  NOR_readData(__x, __z, __y)
#define MAX_SIZE_READ_ONE_TIME_SPI_FLASH 128

/*mcu flash realated porting function*/
#define MCU_FlASH_READ(__x, __y, __z)  SYSFLASH_drv_read_sector(__x, __y, __z)
#define MAX_SIZE_READ_ONE_TIME_MCU_FLASH 128



#ifdef __cplusplus
#if __cplusplus
} 
#endif
#endif /* __cplusplus */


#endif /* __FILE_CHECK_CRC_H__ */
