#~/bin/sh
./uico_bin_to_dat.exe FW04800102A0-I2C-B700-T0-CS-4FC4.bin uicoData.h
cp -f uicoData.h  ../project/source/
rm -f uicoData.h
