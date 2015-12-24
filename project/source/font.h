#ifndef __FONT__
#define __FONT__

#include <stdint.h>
#include "standard_types.h"


// 5x7 ascii space
#define FONT_ASCII_5X7_SPACE_START	         (1048576)  
#define FONT_ASCII_5X7_SPACE_SIZE	           (768)		  

// 8x16 ascii space
#define FONT_ASCII_8X16_SPACE_START	         (FONT_ASCII_5X7_SPACE_START+FONT_ASCII_5X7_SPACE_SIZE)		
#define FONT_ASCII_8X16_SPACE_SIZE		       (1536)

// 15x16 chinese space
#define FONT_CHINESE_15X16_SPACE_START	     (FONT_ASCII_8X16_SPACE_START+FONT_ASCII_8X16_SPACE_SIZE)		
#define FONT_CHINESE_15X16_SPACE_SIZE		     (668864)


I8U FONT_load_characters(I8U *ptr, char *data, I8U height, BOOLEAN b_center);

#endif // __GT23L16U2Y__
/** @} */
