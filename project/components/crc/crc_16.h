/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : crc16.h
 * Author        : MikeWang
 * Date          : 2015-11-17
 * Description   : crc16.c header file
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2015-11-17
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/

#ifndef __CRC_16_H__
#define __CRC_16_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "stdint.h"
uint16_t cyg_crc16(unsigned char *buf, int len,  uint16_t seed);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CRC16_H__ */
