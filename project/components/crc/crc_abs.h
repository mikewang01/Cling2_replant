/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : file_check_crc.h
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

#ifndef __FILE_CHECK_CRC_H__
#define __FILE_CHECK_CRC_H__
#include "crc_abs_port.h"
#include "string.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
	
#endif /* __cplusplus */
#if (__IS_FS_SUPPORED__ == TRUE)
uint16_t file_check_crc(FILE_T *file, uint32_t size);
#endif
	
uint16_t spi_flash_check_crc(uint32_t start_adress, uint32_t size);

uint16_t mcu_flash_check_crc(uint32_t start_adress, uint32_t size);



/**/
typedef struct {
     uint16_t (*read)(uint32_t start_adress, uint8_t *buffer, uint32_t size);
} mem_crc_obj_t;

uint8_t mem_crc_obj_init(mem_crc_obj_t *p);
uint8_t set_mem_crc_obj_read_method(mem_crc_obj_t *p, uint16_t (*fu)(uint32_t start_adress, uint8_t *buffer, uint32_t size));
uint16_t general_mem_crc_check(mem_crc_obj_t *p, uint32_t start_adress, uint32_t size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FILE_CHECK_CRC_H__ */
