#ifndef __FONT__
#define __FONT__

#include <stdint.h>
#include "standard_types.h"

//#define _ENABLE_FONT_TRANSFER_


// 5x7 ascii space
#define FONT_ASCII_5X7_SPACE_START	           (376832)  //start addr:368k 	
#define FONT_ASCII_5X7_SPACE_SIZE	             (768)		 //occupy 768 bytes


// 8x16 ascii space
#define FONT_ASCII_8X16_SPACE_START	           (FONT_ASCII_5X7_SPACE_START+FONT_ASCII_5X7_SPACE_SIZE)		
#define FONT_ASCII_8X16_SPACE_SIZE		         (1536)


//15x16 chinese space
#define FONT_SIMPLE_CHINESE_SPACE_START	       (FONT_ASCII_8X16_SPACE_START+FONT_ASCII_8X16_SPACE_SIZE)		
#define FONT_SIMPLE_CHINESE_SPACE_SIZE		     (668864)



#ifdef _ENABLE_FONT_TRANSFER_
void  FONT_flash_setup(void);
#endif

I8U   FONT_load_characters(I8U *ptr,char *data,I8U height, BOOLEAN b_center);

#endif // __GT23L16U2Y__
/** @} */
