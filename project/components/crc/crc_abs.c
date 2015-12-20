/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : file_check_crc.c
 * Author        : MikeWang
 * Date          : 2015-11-17
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 *
 * Record        :
 * 1.Date        : 2015-11-17
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/
#include "crc_abs.h"
#include "crc_abs_port.h"
#include "crc_16.h"
#include "stdio.h"
/*****************************************************************************
 * Function      : file_check_crc
 * Description   : check file crc
 * Input         : FILE_T *file
                uint8_t *buffer
                uint16_t size
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151117
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
#if (__IS_FS_SUPPORED__ == TRUE)
uint16_t file_check_crc(FILE_T *file, uint32_t size)
{
    if(file == NULL) {
        return 0;
    }

    uint8_t p_buffer[MAX_SIZE_READ_ONE_TIME];
    uint16_t crc_16 = 0xFFFF;
    uint16_t i = 0;
    i = size / MAX_SIZE_READ_ONE_TIME;
    /**/
    if(size % MAX_SIZE_READ_ONE_TIME) {
        i++;
    }
    CRC_PRINTF("[file_crc] crc loop: %d", i);
		
    for(; i > 0; i--) {
        /*no data read from file*/
#if 0
        if(FILE_READ(file, p_buffer, (size > MAX_SIZE_READ_ONE_TIME) ? MAX_SIZE_READ_ONE_TIME : size) < 1) {
            return 0;
        }
#else
        FILE_READ(file, p_buffer, (size > MAX_SIZE_READ_ONE_TIME) ? MAX_SIZE_READ_ONE_TIME : size);
#endif

        //crc_16 =  SYSTEM_CRC(p_buffer, (size > MAX_SIZE_READ_ONE_TIME) ? MAX_SIZE_READ_ONE_TIME : size, crc_16);
        crc_16 = cyg_crc16(p_buffer, (size > MAX_SIZE_READ_ONE_TIME) ? MAX_SIZE_READ_ONE_TIME : size, crc_16);
        if(size < MAX_SIZE_READ_ONE_TIME) {
            Y_SPRINTF("[file_crc]file crc = %x %d %x %x %x size=%d", crc_16, i, p_buffer[0], p_buffer[1], p_buffer[2], size);
        }
        size -= MAX_SIZE_READ_ONE_TIME;
    }

    return crc_16;

}
#endif
/*****************************************************************************
 * Function      : spi_flash_check_crc
 * Description   : crc check for spi flash
 * Input         : uint32_t start_adress  :start adress of spi flash
                	uint16_t size : data lenth of file
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151118
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/

uint16_t spi_flash_check_crc(uint32_t start_adress, uint32_t size)
{

    uint8_t p_buffer[MAX_SIZE_READ_ONE_TIME_SPI_FLASH];
    uint16_t crc_16 = 0xFFFF;
    uint16_t i = 0;
    i = size / MAX_SIZE_READ_ONE_TIME_SPI_FLASH;
    /**/
    if(size % MAX_SIZE_READ_ONE_TIME_SPI_FLASH) {
        i++;
    }

    /*make sure we get coresponded crc for a specific data aread in spi flash*/
    for(; i > 0; i--) {
#if 0
        if(SPI_FlASH_READ(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_SPI_FLASH) ? MAX_SIZE_READ_ONE_TIME_SPI_FLASH : size) < 1) {
            return 0;
        }
#else
        SPI_FlASH_READ(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_SPI_FLASH) ? MAX_SIZE_READ_ONE_TIME_SPI_FLASH : size);
#endif
        crc_16 = cyg_crc16(p_buffer, (size > MAX_SIZE_READ_ONE_TIME_SPI_FLASH) ? MAX_SIZE_READ_ONE_TIME_SPI_FLASH : size, crc_16);
        start_adress += (size > MAX_SIZE_READ_ONE_TIME_MCU_FLASH) ? MAX_SIZE_READ_ONE_TIME_MCU_FLASH : size;
        size -= MAX_SIZE_READ_ONE_TIME_SPI_FLASH;
    }

    return crc_16;

}

/*****************************************************************************
 * Function      : spi_flash_check_crc
 * Description   : crc check for spi flash
 * Input         : uint32_t start_adress  :start adress of spi flash
                	uint16_t size : data lenth of file
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151118
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/


uint16_t mcu_flash_check_crc(uint32_t start_adress, uint32_t size)
{

    uint8_t p_buffer[MAX_SIZE_READ_ONE_TIME_MCU_FLASH];
    uint16_t crc_16 = 0xFFFF;
    uint16_t i = 0;
    i = size / MAX_SIZE_READ_ONE_TIME_MCU_FLASH;
    /**/
    if(size % MAX_SIZE_READ_ONE_TIME_MCU_FLASH) {
        i++;
    }

    /*make sure we get coresponded crc for a specific data aread in spi flash*/
    for(; i > 0; i--) {
        /*no data read from file*/
#if 0
        if(MCU_FlASH_READ(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MCU_FLASH) ? MAX_SIZE_READ_ONE_TIME_MCU_FLASH : size) < 1) {
            return 0;
        }
#else
        MCU_FlASH_READ(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MCU_FLASH) ? MAX_SIZE_READ_ONE_TIME_MCU_FLASH : size);
#endif
        crc_16 = cyg_crc16(p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MCU_FLASH) ? MAX_SIZE_READ_ONE_TIME_MCU_FLASH : size, crc_16);
        /*move to next point where to read firmware*/
        start_adress += (size > MAX_SIZE_READ_ONE_TIME_MCU_FLASH) ? MAX_SIZE_READ_ONE_TIME_MCU_FLASH : size;
        size -= MAX_SIZE_READ_ONE_TIME_MCU_FLASH;
    }

    return crc_16;

}


/*****************************************************************************
 * Function      : mem_crc_obj_init
 * Description   : initiate mem crc objetc function
 * Input         : mem_crc_obj_t *p : object pointer
                	
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151118
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
uint8_t mem_crc_obj_init(mem_crc_obj_t *p)
{
    if(p == NULL) {
        return FALSE;
    }
    memset(p, 0, sizeof(mem_crc_obj_t));
    return TRUE;
}
/*****************************************************************************
 * Function      : set_mem_crc_obj_read_method
 * Description   : set mem object read method
 * Input         : mem_crc_obj_t *p : object pointer
									uint16_t (*fu)(uint32_t start_adress, uint8_t *buffer, uint32_t size) : read function
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151118
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
uint8_t set_mem_crc_obj_read_method(mem_crc_obj_t *p, uint16_t (*fu)(uint32_t start_adress, uint8_t *buffer, uint32_t size))
{
    if(p == NULL || fu ==  NULL) {
        return FALSE;
    }
    //p->read = fu;
    return TRUE;
}

/*****************************************************************************
 * Function      : general_mem_crc_check
 * Description   : general api for mem crc check
 * Input         :  mem_crc_obj_t *p : object pointer
									uint16_t (*fu)(uint32_t start_adress, uint8_t *buffer, uint32_t size) : read function
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151118
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/

#define MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT  128
uint16_t general_mem_crc_check(mem_crc_obj_t *p, uint32_t start_adress, uint32_t size)
{
    uint8_t p_buffer[MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT];
    uint16_t crc_16 = 0;
    uint16_t i = 0;
		/*check if object is valid or not*/
		if(p == NULL || p->read == NULL){
			return 0;
		}
		
    i = size / MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT;
    /**/
    if(size % MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT) {
        i++;
    }

    /*make sure we get coresponded crc for a specific data aread in spi flash*/
    for(; i > 0; i--) {
        /*no data read from file*/
#if 0
        if(p->read(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT) ? MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT : size) < 1) {
            return 0;
        }
#else
        p->read(start_adress, p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT) ? MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT : size);
#endif
        crc_16 = cyg_crc16(p_buffer, (size > MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT) ? MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT : size, crc_16);
        /*move to next point where to read firmware*/
        start_adress += (size > MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT) ? MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT : size;
        size -= MAX_SIZE_READ_ONE_TIME_MEM_DEFAULT;
    }

    return crc_16;

}
