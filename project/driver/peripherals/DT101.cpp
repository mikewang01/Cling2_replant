#include "DT101.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// When you are writing these:
//
//     _i2c_write_reg(0x00, COMMAND_GET_DATA);
//     _i2c_write_reg(0x01, SYSTEM_INFO);
//
//     _i2c_read_reg(0x20, 1, (Data+0));
//     _i2c_read_reg(0x21, 1, (Data+1));
//
// Are you using the 0x48 address? These should be using the 0x48 address because this is trying to get the Revision of firmware from
// the application code. Since the application code doesn't exist it will just NAK, and then it will be done. However, if you are trying
// to use the bootloader address 0x58, you are getting 0x10 returned, which means that the bootloader does not understand what you are
// trying to send it.
//
// From the note above, it says the following:
//
// Main application address for PSOC4 controller is 0x48 (7-bit).
// Bootloader address for PSOC1 controller is 0x38 (7-bit). This will not be used.
// Bootloader address for PSOC4 controller is 0x58 (7-bit).
//
// The function 'try_firmware_update' will use both addresses, as it needs to access information from both the 0x48 main application,
// and 0x58 the bootloader. When it tries to access the System information, it will be using the 0x48 address, but when it tries to
// access bootloader commands, by sending the bootloader bin file lines via I2C, it will use the 0x58 address.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define TUNING                            0x01
#define DURATOUCH_I2C_ADDRESS_APP_0x48            0x90        // 0x48
#define DURATOUCH_I2C_ADDRESS_BOOTLOADER_0x58      0xB0        // 0x58

#define COMMAND_GET_DATA                  0x85        // Get Signature from Controller
#define COMMAND_READPACKET                0x81

#define SYSTEM_INFO                       0x01        // System Information
#define WRITE_START_ACK_CMD								0X20

#define PRODUCT_ID_4_3in                  0xFF
#define PRODUCT_ID_7_0in                  0xFE
#define PRODUCT_ID_10_1in                 0xFD

#define USED_PRODUCT_ID_4_3in             0x03
#define USED_PRODUCT_ID_7_0in             0x07
#define USED_PRODUCT_ID_10_1in            0x0A

#define CUSTOM_PRODUCT_ID_7_0in           0x01

#define MAX_REFRESH_PER_FINGER_4_3in      16000
#define MAX_REFRESH_PER_FINGER_7_0in      18500
#define MAX_REFRESH_PER_FINGER_10_1in     22500
#define MAX_REFRESH_PER_FINGER_default    25000

#define TRACKID_MAX                       32000
#define PRESSURE_MAX                      0xFF

#define SIGNATURE_RESPONSE_LENGTH	        18          // SIGNATURE returns 13 bytes
#define PAD_LENGTH			                   0          // No padding needed.
#define UPGRADE_SIGNATURE_LENGTH	         8
#define	FLASH_ROW_SIZE			              64          // ** Set to 64 for <Redacted>

#define UPGRADE_PREAMBLE_LENGTH           10
#define UPGRADE_BLOCK_LENGTH             (12+ FLASH_ROW_SIZE +2)		   // 78
#define UPGRADE_SEGMENT_LENGTH		        32
#define UPGRADE_EXIT_LENGTH		            10

#define UICO_BIN_SIGNATURE		            'I'                          // binary always begins with this first byte

// Bootloader commands (FF xx)
#define COMMAND_ENTER_BOOTLOAD           0x38
#define COMMAND_FLASH_WRITE              0x39
#define COMMAND_FLASH_VERIFY             0x3A                         // Obsolete, automatically done when flash writen
#define COMMAND_BOOTLOAD_DONE            0x3B
#define COMMAND_BOOTLOAD_SELECT          0x3C

// Bootloader status code
#define BOOT_COMPLETED_OK                0x01                        // Never sent because it reboots
#define IMAGE_VERIFY_ERROR               0x02
#define FLASH_CHECKSUM_ERROR             0x04
#define FLASH_PROTECTION_ERROR           0x08
#define COMM_CHECKSUM_ERROR              0x10
#define BOOTLOAD_MODE                    0x20
#define INVALID_KEY                      0x40
#define INVALID_COMMAND_ERROR            0x80

#define LARGE_FORMAT                        0

#define UICO_ACCESS_REG_COMMAND  0X00
#define UICO_USER_COMMAND 0x8C
#define UICO_USER_SPECIFIC_COMMAND_CALIBRATION_WHEN_FLOATIING 0XA0
#define UICO_USER_SPECIFIC_COMMAND_CALIBRATION_WHEN_FIXTURE   0XA5
#define UICO_CALIBRATION_LENTH   3
#define UICO_ERROR_CODE 0xff
#define UICO_CALIBRATION_PROCCES_LENTH  2
#define UICO_CALIBRATION_SUCESS_CODE  0X00
#define UICO_CALIBRATION_FAILED_CODE  0XFE
#define UICO_DEBUG


static uint8_t is_calibration_sucess =  false;/*INFICATE IF MANUAL CALIBRATION HAS BEEN EXCEXUTED SUCESSFULLY*/
static struct DT101::uico_response_event res[5];
static uint8_t prev_code;

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/
DT101::DT101(PinName sda, PinName scl, bool active) : _i2c(sda, scl), samplerate(64)
{
    set_active(active);
    //samplerate = 64;
    DT101::chip_init();
}

//Since the MMA lacks a WHO_AM_I register, we can only check if there is a device that answers to the I2C address
DT101::uico_chip_status DT101::test_connection( void )
{

    DT101::uico_chip_status chip_status = UICO_IN_UNKOWN_STATUS;
    /*
    1.first step is to detect if bootloaer is there or not if failed, we take it as the chip is not gangeg to twi bus
    2.second step: if bootloader existed, then detect if app status, if app not existed just upgrate the app
    */
    // Write Stop Acknowledge (0x20, 0x01)
    //buf[0] = 0x20;
    //buf[1] = 0x01;
#if 1
    char t[] = {0xFF, 0x38, 0x00 , 0x01 , 0x02, 0x03, 0x04, 0x05 , 0x06 , 0x07};
    int err_code = DT101::bootloader_write(t, sizeof(t));
    if (err_code == 0) {
        /*exit bootloader mode t omake sure th echip gonna be responsive to contrller*/
        char t[] = {0xFF, 0x3B, 0x00 , 0x01 , 0x02, 0x03, 0x04, 0x05 , 0x06 , 0x07};
        int err_code = DT101::bootloader_write(t, sizeof(t));
        printf("[UICO] chip detected on twi bus");

    } else {
        chip_status = UICO_NOT_EXISTED;
        printf("[UICO] chip not existed on twi bus");
    }
#endif
    if(chip_status != UICO_NOT_EXISTED) {
        char t[] = {0x00, 0x81};
        /*NOW THAT, WE HAVE MADE SURE THAT CHIP IS EXISTED, NETX STEP IS KUST AIM TO DETECT IF APP HAS BEEN BURNED
        	INTO CHIP OR NOT, TO ACHIEVE THIS WE PRETEND TO SEND A FEW FAKE DATA,*/
        int err_code = DT101::app_write(t, sizeof(t));
        if (err_code == 0) {
            chip_status = UICO_IN_NORMAL_STATUS;
            printf("[UICO] chip in normal status");

        } else {
            chip_status = UICO_APP_NOT_EXISTED;
            printf("[UICO] chip detected but app is not detected");

        }
    }
    return chip_status;

}

bool DT101::set_active(bool state)
{
#define UICO_POWER_DEEP_SLEEP_CMD   0X83
#define UICO_POWER_ACTIVE_CMD       0X81
    char buf[2];
    uint8_t i = 0;
    buf[i++] = 0x00;
    if(state == true) {
        buf[i++] = UICO_POWER_ACTIVE_CMD;
    } else {
        buf[i++] = UICO_POWER_DEEP_SLEEP_CMD;
    }
    if(DT101::app_write(buf, i) != 0) {
        return false;
    }
    return true;
}

void DT101::read_data(int *data)
{

}


void DT101::read_data(float *data)
{
    int intdata[3];
    read_data(intdata);
    for (int i = 0; i < 3; i++)
        data[i] = intdata[i] / DT101_SENSITIVITY;
}

//////////////////////////////////////////////
///////////////PRIVATE////////////////////////
//////////////////////////////////////////////


int DT101::app_write( char *data, size_t lenth )
{
    return _i2c.write(DURATOUCH_I2C_ADDRESS_APP_0x48, data, lenth);
}

int DT101::bootloader_write( char *data, size_t lenth )
{
    return _i2c.write(DURATOUCH_I2C_ADDRESS_BOOTLOADER_0x58, data, lenth);
}

int DT101::app_read(char *data, int length)
{
    return _i2c.read(DURATOUCH_I2C_ADDRESS_BOOTLOADER_0x58, data, length);
}

int DT101::bootloader_read(char *data, int length)
{
    return _i2c.read(DURATOUCH_I2C_ADDRESS_BOOTLOADER_0x58, data, length);
}

//////////////////////////////////////////////////////
///////////////app related api////////////////////////
/////////////////////////////////////////////////////
#if 0
int prepare_system_information()
{
#define PREPARE_SYSTEM_INF_LENTH  3
    uint8_t buf[PREPARE_SYSTEM_INF_LENTH];
    uint16_t i = 0;
    buf[i++] = 0x00;
    buf[i++] = COMMAND_GET_DATA;
    buf[i++] = SYSTEM_INFO;
    return DT101::app_write((char*)buf, i);
}

int start_acknowledge()
{
    /*--------------------------- Acknowledge Any Response ---------------------------*/
    // Write Start Acknowledge (0x20)
#define START_ACKLKOWLEGE_LENTH  1
    uint8_t buf[START_ACKLKOWLEGE_LENTH];
    uint16_t i = 0;
    buf[i++] = WRITE_START_ACK_CMD;
    return DT101::app_write((char*)buf, i);

}
#endif
int DT101::stop_acknowledge()
{
#define STOP_ACKLKOWLEGE_LENTH  2
    uint8_t buf[STOP_ACKLKOWLEGE_LENTH];
    uint16_t i = 0;
    // Write Stop Acknowledge (0x20, 0x01)
    buf[i++] = WRITE_START_ACK_CMD;
    buf[i++] = 0X01;
    return DT101::app_write((char*)buf, i);
}


/******************************************************************************
 * FunctionName : read_unkown_lenth_from_uico_touch
 * Description	: when a isr happens  we do not  even khow the lenth we need
 * Parameters	: buffer_p :  data buffer
 *                 rx_lenth :  data lenth has been read
 * Returns		: true: sucess false£º failed
*******************************************************************************/
/*
        write to Touch IC: 0x20 // required; changes to the response buffer
        read 2 bytes from Touch IC and store in touch_data // determine the number of bytes in the response (byte 1 in the response)
        len = touch_data[1]

        if(touch_data[1] > 0) {
            read(len + 2) bytes from Touch IC and store in touch_data
        }

        write to Touch IC: 0x20 0x01 // required; clears the touch IC interrupt line;
*/

int DT101::read_unkown_lenth_from_chip(char *buffer_p, uint16_t *rx_lenth)
{
#if 0

#else
    char *buffer = buffer_p; //[100];

    uint8_t i = 0;
    if(buffer == NULL) {
        return  false;
    }
    /*--------------------------- Acknowledge Any Response ---------------------------*/
    // Write Start Acknowledge (0x20)
    buffer[0] = WRITE_START_ACK_CMD;
    DT101::app_write(buffer, 1);

    /*--------------------------- Read Response  ---------------------------*/
    // Read First 2 bytes
    DT101::app_read (buffer, 2);
    // N_SPRINTF("[UICO]: 0x%02x 0x%02x", buffer[0], buffer[1]);
    *rx_lenth =  buffer[1];
    *rx_lenth += 2;
    DT101::app_read(buffer,  *rx_lenth);
    // N_SPRINTF("[UICO]: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    // Write Stop Acknowledge
    DT101::stop_acknowledge();

    return true;
#endif
}



bool DT101::chip_init()
{
    uint8_t  buf[128];
    uint16_t i = 0;
    uint8_t  total_sensor_cnt, column_sensor_cnt;
    uint32_t resolution_x = 0;
    uint32_t resolution_y = 0;
    uint32_t screen_size_code = 0;
    uint32_t refresh_rate = 0;
    uint8_t  touch_num_supported = 0;
    // Initialize static valables
    memset(res, 0, sizeof(res));
    // Start firmware update
    memset(buf, 0xff, sizeof(buf));

    //_try_firmware_update();
    while( !((buf[0] == COMMAND_GET_DATA) && (buf[1] == 0x11)) ) {
        /*--------------------------- Write Command ---------------------------*/
        buf[0] = 0x00;
        buf[1] = COMMAND_GET_DATA;
        buf[2] = SYSTEM_INFO;
        if(DT101::app_write((char*)buf, 3) != 0) {
            return false;
        }

        /*--------------------------- Acknowledge Any Response ---------------------------*/
        // Write Start Acknowledge (0x20)
        buf[0] = 0x20;
        if(DT101::app_write((char*)buf, 3) != 0) {
            return false;
        }

        /*--------------------------- Read Response  ---------------------------*/
        // Read First 2 bytes
        if(DT101::app_read((char*)buf, 2) != 0) {
            return false;
        }


        if( !((buf[0] == COMMAND_GET_DATA) && (buf[1] == 0x11)) ) {
            /*--------------------------- Acknowledge for Correct Command  ---------------------------*/
            // Write Stop Acknowledge
            stop_acknowledge();
        }

    }
    printf("[UICO init]: 0x%02x 0x%02x 0x%02x", buf[0], buf[1], 0x55);
    // Read 2 + buf[1] bytes
    if(DT101::app_read((char*)buf, 2 + buf[1]) != 0) {
        return false;
    }

    // Write Stop Acknowledge (0x20, 0x01)
    if(DT101::stop_acknowledge() != 0) {
        return false;
    }
    resolution_x = (buf[13] << 8) + buf[14];
    resolution_y = (buf[15] << 8) + buf[16];
    touch_num_supported = buf[7];
    screen_size_code    = buf[6];

    // N_SPRINTF("[UICO]: %d %d %d %d", resolution_x, resolution_y, touch_num_supported, screen_size_code);

    if( refresh_rate == 0 ) {
        switch(screen_size_code) {
            case PRODUCT_ID_4_3in:
                refresh_rate = MAX_REFRESH_PER_FINGER_4_3in;
                break;
            case PRODUCT_ID_7_0in:
                refresh_rate = MAX_REFRESH_PER_FINGER_7_0in;
                break;
            case PRODUCT_ID_10_1in:
                refresh_rate = MAX_REFRESH_PER_FINGER_10_1in;
                break;
            case USED_PRODUCT_ID_4_3in:
                refresh_rate = MAX_REFRESH_PER_FINGER_4_3in;
                break;
            case USED_PRODUCT_ID_7_0in:
                refresh_rate = MAX_REFRESH_PER_FINGER_7_0in;
                break;
            case USED_PRODUCT_ID_10_1in:
                refresh_rate = MAX_REFRESH_PER_FINGER_10_1in;
                break;
            case CUSTOM_PRODUCT_ID_7_0in :
                refresh_rate = MAX_REFRESH_PER_FINGER_7_0in;
                break;
            default:
                refresh_rate = MAX_REFRESH_PER_FINGER_default;
                break;
        }
    }
    total_sensor_cnt = buf[4];
    column_sensor_cnt = buf[5];
    // N_SPRINTF("[UICO]: Total Sensor Count: %d Column Sensor Count: %d", total_sensor_cnt, column_sensor_cnt);
    // N_SPRINTF("[UICO]: Product Screen Size Code: %x", screen_size_code);
    // N_SPRINTF("[UICO]: Number of Touches Supported: %d", touch_num_supported);
    // N_SPRINTF("[UICO]: X Resolution: %d Y Resolution: %d", resolution_x, resolution_y);
    return true;
}

