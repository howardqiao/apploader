#!/bin/bash

/usr/local/bin/apploader
source /home/pi/apploader/selection.conf
echo "Now loading..."
if [ $sel -eq 0 ]
then
	sleep 2
	emulationstation
elif [ $sel -eq 1 ]
then
	sleep 2
	kodi
elif [ $sel -eq 2 ]
then
	sleep 2
	startx
elif [ $sel -eq 3 ]
then
	exit 0
fi
