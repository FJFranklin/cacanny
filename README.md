# cacanny

An abstraction for a number of canbus libraries across different devices.

I have a number of devices that I want to connect via CAN:
* Feather M4 CAN: The docs say the Feather can handle 500k but it's not recommended. Running at 250k, two Feathers have been tested at 400 messages per second.
* Arduino Uno R4 WiFi (test failing): Supports 1000k but not RTR (but see [UNOR4CAN](https://github.com/obdevel/UNOR4CAN)).
* Teensy 4.1 (tested at 250k)

Pending:

* the MCP 2515 breakout
* and an ESP at some point in the near future.