/********************************************************************************

 **** Copyright (C), 2015, xx xx xx xx info&tech Co., Ltd.                ****

 ********************************************************************************
 * File Name     : mics_function.c
 * Author        : MikeWang
 * Date          : 2015-12-10
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 *
 * Record        :
 * 1.Date        : 2015-12-10
 *   Author      : MikeWang
 *   Modification: Created file

*************************************************************************************************************/
#include "stdint.h"
#include "standard_macros.h"
#include  "mics_function.h"
#include  "nrf51.h"
#define STACK_SPECIAL_CHRACTOR 0x55
uint8_t wechat_delay(void);
/*****************************************************************************
 * Function      : main_stack_check_init
 * Description   : set the stack memory to a specific charactor whici is 0x55
 * Input          : None
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151210
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
uint16_t main_stack_check_init()
{
    extern uint32_t STACK_TOP;
    uint8_t *stack_top = (uint8_t *)STACK_TOP;
    /*stack_top < stack bottom*/
    uint8_t *i;
    for(i = (uint8_t*)__get_MSP() ; i >= (stack_top); i--) {
        *(i) = STACK_SPECIAL_CHRACTOR;
    }
		return TRUE;
}
/*****************************************************************************
 * Function      : mainstack_check_max_used_bytes
 * Description   : check out the max stack size ever be used
 * Input          : None
 * Output        : None
 * Return        :
 * Others        :
 * Record
 * 1.Date        : 20151210
 *   Author      : MikeWang
 *   Modification: Created function

*****************************************************************************/
uint32_t mainstack_check_max_used_bytes()
{
		 /*stack_top < stack bottom*/
    if(wechat_delay() == TRUE) {
        extern uint32_t STACK_TOP;
        uint8_t *stack_top = (uint8_t *)STACK_TOP;
        uint8_t *i;
#if 1
        for(i = stack_top ; i <= (uint8_t*)(__get_MSP()); i++) {
            if(*(i) != STACK_SPECIAL_CHRACTOR) {
                break;
            }
        }
#endif
        return (i - stack_top);
    }
		
		return 0;
}
