#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar -t dts --board ./synapse.xml -i ./synapse.sopcinfo -o ./synapse.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar --type u-boot --input ./synapse.sopcinfo --output ./synapse.h
