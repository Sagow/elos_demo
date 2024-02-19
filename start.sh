#!/bin/bash
# mkdir /tmp/elos
# elosd &
cd brakes
make && ./brakes &
cd ../computer
make && ./computer &
cd ../oil_thermometer
make && ./oil_thermometer &
cd ../speed_sensor
make && ./speed_sensor &
cd ../interface
make && ./interface