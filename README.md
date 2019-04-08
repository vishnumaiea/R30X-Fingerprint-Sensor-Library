# R30X-Fingerprint-Sensor-Library
This i an Arduino compatible library for **R30X** series optical fingerprint sensors. The library is written in a manner to be easily readable and thus modifiable.

I still need to implement some functions such as downloading and uploading images. I'll do that soon and will also publish a detailed tutoral on this.

The example sketch was written for **Arduino Due** and **R307** fingerprint sensor. To wire up, connect the TX and RX pins to the TX1 and RX1 pins of Due or Mega. If you're using Uno or similar boards with only one UART, use SoftwareSerial for the fingerprint sensor and hardware UART for debugging. The example can invoke all implemented functions from a serial terminal with short commands and parameters. The list of available commands are,

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
- **setseclvl \<level\>** - set security level
- **genimg** - generate image
- **genchar \<buffer id\>** - generate character file from image
- **gentmp** - generate template from character buffers
- **savtmp \<buffer id\> \<location\>** - save template to library from buffer
- **lodtmp \<buffer id\> \<location\>** - load template from library to buffer
- **deltmp \<start location\> \<quantity\>** - delete one or more templates from library
- **mattmp** - precisely match two templates available on buffers
- **serlib \<buffer id\> \<start location\> \<quantity\>** - search library for content on the buffer

All commands and parameters must be separated by single whitespace.
