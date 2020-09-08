# R30X-Fingerprint-Sensor-Library
This is an Arduino compatible library for **R30X** series optical fingerprint sensor/scanner from Hangzhou Grow Technology. The library is written in a manner to be easily readable and thus modifiable.

- Library version : 1.3.1
- Author : Vishnu Mohanan (@vishnumaiea)
- Source : https://github.com/vishnumaiea/R30X-Fingerprint-Sensor-Library
- Author's website : www.vishnumaiea.in
- Initial release : +05:30 07:35 PM, 08-04-2019, Monday
- Last updated : +05:30 06:10:01 PM, 20-07-2020 Monday
- License : MIT

A detailed tutorial on interfacing the modules and using the library is available on my project website here : https://www.vishnumaiea.in/projects/hardware/interfacing-r307-optical-fingerprint-scanner-with-arduino (this repo may be newer than what's described in the tutorial). I still need to implement some functions for importing and exporting fingerprint templates and images from and to the sensor.

To install this library to your computer, open the _Library Manager_ from the _Arduino IDE_ and search for "R30X fingerprint scanner"
. Then install the latest version from the list.

The example sketch was tested with **Arduino Due** and **Arduino Uno** using **R307** fingerprint scanner. To wire up, connect the TX and RX pins to the TX1 and RX1 pins of Due or Mega. If you're using Uno or similar boards with only one hardware UART, use **SoftwareSerial** for the fingerprint sensor and hardware UART for debugging. The example can invoke all implemented functions from a serial terminal with short commands and input parameters. Below is the list of available commands.

- **clrlib** - clear library
- **tmpcnt** - get templates count
- **readsys** - read system parameters
- **setdatlen \<data length\>** - set data length
- **capranser \<timeout\> \<start location\> \<quantity\>** - capture and range search library for fingerprint
- **capfulser** - capture and full search the library for fingerprint
- **enroll \<location\>** - enroll new fingerprint
- **verpwd \<password\>** - verify 4 byte device password
- **setpwd \<password\>** - set new 4 byte device password
- **setaddr \<address\>** - set new 4 byte device address
- **setbaud \<baudrate\>** - set the baudrate
- **reinitprt \<baudrate\>** - reinitialize the port without changing device configuration
- **setseclvl \<level\>** - set security level
- **genimg** - generate image
- **genchar \<buffer id\>** - generate character file from image
- **gentmp** - generate template from character buffers
- **savtmp \<buffer id\> \<location\>** - save template to library from buffer
- **lodtmp \<buffer id\> \<location\>** - load template from library to buffer
- **deltmp \<start location\> \<quantity\>** - delete one or more templates from library
- **mattmp** - precisely match two templates available on buffers
- **serlib \<buffer id\> \<start location\> \<quantity\>** - search library for content on the buffer

All commands and parameters must be separated by **single whitespace**.
