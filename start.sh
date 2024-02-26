#!/bin/bash
# mkdir /tmp/elos
# elosd &
cd build
cd brakes
./brakes &
cd ../computer
./computer &
cd ../oil_thermometer
./oil_thermometer &
cd ../speed_sensor
./speed_sensor &
cd ../interface
./interface