# cacanny

An abstraction for a number of canbus libraries across different devices.

I have a number of devices that I want to connect via CAN:
* Feather M4 CAN: The docs say the Feather can handle 500k but it's not recommended. Running at 250k, two Feathers have been tested at 400 messages per second.
* Teensy 4.1: CAN1 and CAN2 tested at 250k and 1000k; CAN3 available but untested.

What works well:
* I have had two Feathers doing ping-pong with each other at 100Hz on the same bus as a Teensy 4.1 doing ping-pong with itself at 100Hz, adding up to 800 messages per second.
* I have had the Teensy 4.1 doing ping-pong with itself at 1kHz, adding up to 4000 messages per second.

Note: A 125kbit CAN bus can handle in theory about 950 messages per second, but more realistically about 850 messages per second. This would suggest realistic maximum rates of
1700 messages per second for the 250kbit CAN bus and 6800 messages per second for the 1Mbit CAN bus.

What doesn't work quite so well:
* There are some lost pings. In the test setup, the Feathers each have 16 CaCanny message instances to pass around, and clearly there isn't always one available for a pong.
* When restarting the Feather while the bus is busy, there's a flurry of lost messages, around 84 on two occasions; maybe the Feather has a lower-level buffer for 100 messages?

Implemented but failing utterly for unknown reasons:
* Arduino Uno R4 WiFi (test failing): Supports 1000k but not RTR (but see [UNOR4CAN](https://github.com/obdevel/UNOR4CAN)).
* the MCP 2515 breakout: I cannot get these to connect at all. Update: I can at least get the Teensy to talk to one over SPI, and it claims to be at least sending messages but
these aren't being received, so... I suspect the four MCP2515 devices I bought are all broken, although it's possible I'm just failing to understand how to wire them up. 

Pending:
* and an ESP at some point in the near future.