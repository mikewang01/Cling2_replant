TARGET_CHIP := NRF51822_QFAA_CA
BOARD := BOARD_PCA10001

# Imported source files
STRIIV_WRISTBAND = ..

STRIIV_WRISTBAND_INC = ${STRIIV_WRISTBAND}/Include
APPCOMMINC = ${STRIIV_WRISTBAND_INC}/app_common
BLEINC = ${STRIIV_WRISTBAND_INC}/ble
BLESERVICESINC = ${STRIIV_WRISTBAND_INC}/ble/ble_services
BLESDINC = ${STRIIV_WRISTBAND_INC}/ble/softdevice

INCDIR = $(STRIIV_WRISTBAND_INC} $(APPCOMMINC) $(BLEINC) $(BLESDINC)

STARTUPSRC = ${STRIIV_WRISTBAND}/system_nrf51.c
		 
LIBSRC = ${STRIIV_WRISTBAND}/lib/nrf_delay.c \
		 ${STRIIV_WRISTBAND}/lib/nrf_ecb.c

BLESRC = ${STRIIV_WRISTBAND}/ble/ble_error_log.c \
		 ${STRIIV_WRISTBAND}/ble/ble_advdata.c \
		 ${STRIIV_WRISTBAND}/ble/ble_sensorsim.c \
		 ${STRIIV_WRISTBAND}/ble/ble_stack_handler.c \
		 ${STRIIV_WRISTBAND}/ble/ble_conn_params.c \
		 ${STRIIV_WRISTBAND}/ble/ble_bondmngr.c \
		 ${STRIIV_WRISTBAND}/ble/ble_flash.c \
		 ${STRIIV_WRISTBAND}/ble/ble_radio_notification.c \
		 ${STRIIV_WRISTBAND}/ble/ble_debug_assert_handler.c 
		 

BLESERVICESSRC = ${STRIIV_WRISTBAND}/ble/ble_services/ble_dis.c \
		 ${STRIIV_WRISTBAND}/ble/ble_services/ble_bas.c \
		 ${STRIIV_WRISTBAND}/ble/ble_services/ble_hrs.c \
		 ${STRIIV_WRISTBAND}/ble/ble_services/ble_srv_common.c 
		 
APPCOMMONSRC = ${STRIIV_WRISTBAND}/app_common/app_timer.c

APPSRC = ${STRIIV_WRISTBAND}/main.c \
		 ${STRIIV_WRISTBAND}/hal.c \
		 ${STRIIV_WRISTBAND}/dbg.c \
		 ${STRIIV_WRISTBAND}/system.c \
		 ${STRIIV_WRISTBAND}/fs_fat.c \
		 ${STRIIV_WRISTBAND}/fs_file.c \
		 ${STRIIV_WRISTBAND}/fs_nflash_rw.c \
		 ${STRIIV_WRISTBAND}/fs_norflash.c \
		 ${STRIIV_WRISTBAND}/fs_root.c \
		 ${STRIIV_WRISTBAND}/gcp_api.c \
		 ${STRIIV_WRISTBAND}/auth.c \
		 ${STRIIV_WRISTBAND}/user.c \
		 ${STRIIV_WRISTBAND}/RTC.c \
		 ${STRIIV_WRISTBAND}/SystemClock.c \
		 ${STRIIV_WRISTBAND}/power.c \
		 ${STRIIV_WRISTBAND}/Watchdog.c \
		 ${STRIIV_WRISTBAND}/base.c \
		 ${STRIIV_WRISTBAND}/activity.c \
		 ${STRIIV_WRISTBAND}/pedometer.c \
		 ${STRIIV_WRISTBAND}/btle.c \
		 ${STRIIV_WRISTBAND}/usb_api.c \
		 ${STRIIV_WRISTBAND}/keypad.c 

#C sources here.
CSRC = $(STARTUPSRC) \
	   $(LIBSRC) \
	   $(BLESRC) \
	   $(BLESERVICESSRC) \
	   $(APPCOMMONSRC) \
	   $(APPSRC)

# List ASM source files here
STARTUPASM = ${STRIIV_WRISTBAND}/gcc/gcc_startup_nrf51.s 
		 
ASMSRC = $(STARTUPASM)

LDFLAGS += -specs=nano.specs 

OUTPUT_FILENAME := wristband_gcc

PROGFILES := C:/Program Files (x86)
SDK_PATH = $(PROGFILES)/Nordic\ Semiconductor/nRF51\ SDK_v4.4.1.31827/Nordic/nrf51822/

POST_LD_FLAGS += -lm

#Uncomment correct line if you have softdevice programmed on the chip.
DEVICE_VARIANT := xxaa

USE_SOFTDEVICE := S110

include ${STRIIV_WRISTBAND}/gcc/Makefile.common
