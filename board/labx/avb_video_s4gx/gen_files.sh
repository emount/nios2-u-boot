#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar -t dts --pov-type cpu --pov nios2_qsys_0 --board ./avb_video_s4gx.xml -i ./avb_video_s4gx.sopcinfo --verbose -o ./avb_video_s4gx.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar --type u-boot --pov-type cpu --pov nios2_qsys_0 --input ./avb_video_s4gx.sopcinfo --output ./avb_video_s4gx.h

echo 'Generating environment script image for U-Boot...'
../../../tools/mkimage -T script -C none -n "U-Boot Env Script" -d ./ub.config.scr ./ub.config.img