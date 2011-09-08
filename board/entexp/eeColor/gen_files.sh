#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../tools/sopc2dts/sopc2dts.jar -t dts --board ./eeColor_BoardInfo.xml -i ./eecpb_sopc.sopcinfo -o ./eeColor.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../tools/sopc2dts/sopc2dts.jar -t uboot -i ./eecpb_sopc.sopcinfo -o ./eeColor.h
