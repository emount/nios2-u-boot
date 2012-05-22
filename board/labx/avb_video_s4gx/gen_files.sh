#!/bin/sh
echo 'Generating DTS file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar -t dts --board ./avb_video_s4gx.xml -i ./avb_video_s4gx.sopcinfo -o ./avb_video_s4gx.dts

echo 'Generating U-Boot header file from SOPCINFO...'
java -jar ../../../../nios2-tools/sopc2dts/sopc2dts.jar --type u-boot --input ./avb_video_s4gx.sopcinfo --output ./avb_video_s4gx.h