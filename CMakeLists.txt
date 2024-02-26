cmake_minimum_required(VERSION 3.22)

project(elos-demo VERSION 1.0
    DESCRIPTION "elos in real situation"
    LANGUAGES CXX)

set(repositories
    brakes
    computer
    interface
    oil_thermometer
    speed_sensor)

foreach(val ${repositories})
    add_subdirectory(${val})
endforeach(val)


