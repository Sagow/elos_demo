This small project is used to demonstrate some of the opportunities offered by the free library elos (https://github.com/Elektrobit/elos)
Through several executables, it simulates different parts of a car, only communicating through elos.

To lauch it, you will need elos and X11 (simple graphic interface), then start elosd and simply use the "start.sh" script.

The different car parts are :
- the brakes : listen to the computer, and emits the current state of wear
- the computer : listens to the interface, the oil thermometer and the speed sensor. If the signal from interface seems correct, it will forward it. If the oil temperature gets critical, it will emit the "slow down" signal on its own
- the interface : listens to the brakes, the oil thermometer and the speed sensor. A basic graphic and keyboard interface allows you to interact with the car. It displays the oil_temperature (normal, high, critical), the current speed, and the brakes wear. The upper-arrow key emits the "speed up" signal, while the lower-arrow emits the "slow down" signal. The 'p' key sends the "power off" signal, which is only forwarded by the commuter if the current speed is zero
- the oil_thermometer : listens to the speed sensor, and emits the current state of the temperature of the motor oil
- the speed sensor : listens to the comuter, and emits the current speed

