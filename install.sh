#!/bin/bash
mkdir /tmp/alarm_manager/
chmod 0777 /tmp/alarm_manager/
rm -f messages.txt modul alarmManager libalarm.so deleter
make
make clean
rm /usr/lib/libalarm.so
cp libalarm.so /usr/lib/libalarm.so
