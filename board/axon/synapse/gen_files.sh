#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar -t dts --pov-type cpu --pov nios2_qsys_0 --board ./synapse.xml -i ./synapse.sopcinfo --verbose -o ./synapse.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar --type u-boot --pov-type cpu --pov nios2_qsys_0 --input ./synapse.sopcinfo --output ./synapse.h

echo 'Generating environment script for U-Boot...'
../../../tools/mkimage -T script -C none -n "U-Boot Env Script" -d ./ub.config.scr ./ub.config.img
../../../tools/mkimage -T script -C none -n "U-Boot Env Script" -d ./ub.config-yocto.scr ./ub.config-yocto.img