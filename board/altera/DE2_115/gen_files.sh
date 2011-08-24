#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../tools/sopc2dts/sopc2dts.jar -t dts -i ./DE2_115_SOPC.sopcinfo -o ./DE2_115_SOPC.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../tools/sopc2dts/sopc2dts.jar -t uboot -i ./DE2_115_SOPC.sopcinfo -o ./DE2_115_SOPC.h
