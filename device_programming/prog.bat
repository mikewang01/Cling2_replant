nrfjprog.exe --eraseall --reset --clockspeed 2000
nrfjprog.exe --clockspeed 2000 --programs .\s110_nrf51_8.0.0_softdevice.hex --program .\Cling2A_1_11.hex .\Cling2Bootloader.hex --verify --pinreset 
