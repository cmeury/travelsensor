# travelsensor
Learning to use the Arduino platform; simple code to display temperature, light level and pressure on a liquid crystal display. It cycles through the read-outs and offers the option to calibrate the display value on the fly using two buttons (useful for adjusting the altitude to make up for changes in atmospheric pressure due to the weather).

I used the [SparkFun Inventor's Kit v3.1](https://www.sparkfun.com/products/12001) and an additionally bought [pressure sensor](https://www.sparkfun.com/products/11824) to develop this little device as a week-end project while being on vacation in the Swiss alps. Here is a [picture](circuit.jpg) (without the pressure sensor but everything else).

## Dependencies
- [BMP180 Breakout Library](https://github.com/sparkfun/BMP180_Breakout/)

## License
- Main code is licensed under Apache Version 2, see LICENSE file.
- The timer code comes from the [Metro library](https://github.com/thomasfredericks/Metro-Arduino-Wiring) but was rewritten and modified to use arrays in Sketch. That particular code (timer_reset, timer_check) is licensed differently than the main travelsensor.ino (see TIMER-LICENSE file).
