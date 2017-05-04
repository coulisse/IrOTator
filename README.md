# IrOTator
(https://coulisse.github.io/IrOTator/)
This is a client/server application used to control an antenna rotator, simply using a web browser.

## Why
I'm an ham radio and I would to use a TV Antenna rotator, in order to point my antenna beam in a specific direction.
Since I wouldn't make any hole in my house walls, I decided to control the rotator using a WIFI system.

## What you need
- esp8266 (nodemcu package)
- hmc5883l (compass)
- 2 Channel DC 5V Relay Module
- 2 P82b715 (i2c extender) module
- TV antenna rotator

## The Software
The software is composed of two components: a server and a client

### Server
The server "esp8266Rotator" will be installed on the nodemcu card. The server establish a WiFi connection with your network and will wait for command from the client component.
The server read the direction of the rotator, using the chip hmc5883l and then sends the information to the client. Moreover it reads the commands from the client and pilotate the relays, for rotating the antenna, untill the position required is reached.
This software is written as an Arduino sketch.
You can find it in the "esp8266Rotator" directory.

### Client
The client is a web based application. Its consists of an html, a control javascript and a canvas gauge js. There is also a main.css in order to give a look to the application.
This application connects to the server, receive from it the direction of the antenna and send the commands to the server for rotating the antenna.
You can find it in the "webapp" directory.

## Wiring
![](esp8266Rotator/schema/schema.png?raw=true)

## Howto install and use
1. go in the  "esp8266Rotator" directory.
2. put your WiFi credentials in the file login.h
3. install "esp8266Rotator" on your nodemcu using Arduino IDE. See how to on  [instructables](http://www.instructables.com/id/Quick-Start-to-Nodemcu-ESP8266-on-Arduino-IDE/).
4. check the ip address of the nodemcu.
5. copy the "weapp" directory in your preferred directory.
6. open the rotator.html with your preferred browser.
7. put the ip address of the nodemcu in the connection string, then connect to the rotoator and play with it!

If you want you could change the azimuthal map in the webapp/img/map.png using your position location. In order to do it see on [ns6t](http://ns6t.net/azimuth/azimuth.html).


## TODO
- Implement Internet Explorer
- Implement EDGE compatibility

## Thanks
I'm using the JS component [Canvas Gauges](https://canvas-gauges.com/) for display the compass.

## Licensing
Please see the LICENSE file

## Links
See my [blog](http://iu1bow.blogspot.it/)
