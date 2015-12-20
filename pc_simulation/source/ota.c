/***************************************************************************/
/**
* File:    ota.c
*
* Description: Over-the-air update
*
******************************************************************************/

#include "main.h"
#include "sysflash_rw.h"
#ifndef _CLING_PC_SIMULATION_
#include "crc_abs.h"

static void _start_OTA()
{
    FILE_CTX *fc;
    I32U data;
    uint32_t *config;

    Y_SPRINTF("[OTA] perform OTA, initialize all the parameters, and go reboot");

    // Clean up the setting page
    SYSFLASH_drv_page_erasure(255);

    fc = FILE_fopen((I8U *)"app.bin", FILE_IO_READ);
    /*=================check out crc to make sure the data in flash has been writen tolally correct=========================*/
    uint16_t crc16_origin = fc->crc;
    uint16_t crc16_check = file_check_crc(fc, fc->size);
    Y_SPRINTF("[OTA] original file crc = %x,  crc16_check = %x ", crc16_origin , crc16_check);
    if(crc16_origin == crc16_check) {
        Y_SPRINTF("[OTA] nor flash crc checkout sucessfully");
    } else {
        Y_SPRINTF("[OTA] failed to checkout flash crc ");
        /*skip */
        goto failed;
    }

    // 1. File starting address
    config = (uint32_t *)APP_UPDATE_FLAG_START_ADDR;
    data = (fc->begin_cluster + 512) << 7;
    SYSFLASH_drv_write(config, &data, 1);

    if(*config != data) {
        Y_SPRINTF("[OTA] failed to write start address writed ");
        /*skip following steps*/
        goto failed;
    }
    Y_SPRINTF("[OTA] start address: %d", data);

    // 2. File length
    config = (uint32_t *)APP_UPDATE_FLAG_FILE_LEN;
    data = fc->size;
    SYSFLASH_drv_write(config, &data, 1);
    if(*config != data) {
        Y_SPRINTF("[OTA] failed to write file length ");
        /*skip following steps*/
        goto failed;
    }
    Y_SPRINTF("[OTA] file length: %d", data);

    FILE_fclose(fc);

    // flag files as deleted
    FILE_delete((I8U *)"ota_start.txt");
    FILE_delete((I8U *)"app.bin");

    // 3. OTA flag
    data = 0x55504454;
    config = (uint32_t *)APP_UPDATE_FLAG_ENABLE;
    SYSFLASH_drv_write(config, &data, 1);
    if(*config != data) {
        Y_SPRINTF("[OTA] failed to write start updating flag ");
        /*skip following steps*/
        goto failed;
    }
    // Release NOR flash power down before rebooting the device
    NOR_releasePowerDown();

    Y_SPRINTF("[OTA] Reboot to upgrade firmware");

    BASE_delay_msec(100);

    SYSTEM_restart_from_reset_vector();
failed:
		/*if get error message do some handling before reboot*/
		FILE_fclose(fc);
    // flag files as deleted
    FILE_delete((I8U *)"ota_start.txt");
    FILE_delete((I8U *)"app.bin");
		NOR_releasePowerDown();
		Y_SPRINTF("[OTA] upgrade failed, roll back to old firmware");
		BASE_delay_msec(100);
    SYSTEM_restart_from_reset_vector();
}
#endif

void OTA_main()
{
    // Over-the-air update
    //
    // 1. app.bin - application firmware update
    //
    // 2. sd.bin - softdevice firmware update
    //
    // 3. cy.bin - touch IC firmware update
    //
#ifndef _CLING_PC_SIMULATION_
    if (FILE_if_exists((I8U *)"ota_start.txt")) {

        Y_SPRINTF("[MAIN] found: ota_start.txt");

        if (FILE_if_exists((I8U *)"app.bin") == TRUE) {
            // Auto update txt file is sent from iOS once all MCU binary pieces are correct, and
            // ready for self-update.
            Y_SPRINTF("[MAIN] found: app.bin");

            _start_OTA();
        } else if (FILE_if_exists((I8U *)"sd.bin") == TRUE) {
            // We should update SoftDevice
        } else if (FILE_if_exists((I8U *)"cy.bin") == TRUE) {
        }
    }
#endif
}

BOOLEAN OTA_if_enabled()
{
    if (cling.ota.b_update) {
#if 0
        // Switch to fast connection mode to exchange data (no need, just
        if (HAL_set_conn_params(TRUE)) {
            Y_SPRINTF("OTA update with FAST Connection interval");
        }
#endif
        return TRUE;
    } else {
        return FALSE;
    }
}

void OTA_set_state(BOOLEAN b_enable)
{
    // Set OTA flag
    cling.ota.b_update = b_enable;

    // Turn on OLED
    if (b_enable) {

        if (cling.system.mcu_reg[REGISTER_MCU_BATTERY] > 90) {

            // Turn on screen
            UI_turn_on_display(UI_STATE_TOUCH_SENSING, 0);
        }
    }
}



