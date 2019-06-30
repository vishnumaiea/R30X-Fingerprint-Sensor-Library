# ISL1208-RTC-Library
An Arduino compatible library (will support ESP8266, ESP32 or pretty much any other boards with Arduino support) for Intersil ISL1208 real-time clock IC available in MSOP-8 package. Following are its features.
- Real Time Clock/Calendar
  - Tracks Time in Hours, Minutes, and Seconds
  - Day of the Week, Day, Month, and Year
- 15 Selectable Frequency Outputs
- Single Alarm
   - Settable to the Second, Minute, Hour, Day of the Week, Day, or Month
   - Single Event or Pulse Interrupt Mode
- Automatic Backup to Battery or Super Capacitor
- Power Failure Detection
- On-Chip Oscillator Compensation
- 2 Bytes Battery-Backed User SRAM
- I2C Interface
   - 400kHz Data Transfer Rate
- 400nA Battery Supply Current
- Same Pin Out as ST M41Txx and Maxim DS13xx Devices
- Small Package Options
   - 8 Ld MSOP and SOIC Packages
   - 8 Ld TDFN Package
- Pb-Free Available (RoHS Compliant)

Link to datasheet : https://www.intersil.com/content/dam/intersil/documents/isl1/isl1208.pdf

# Library
### Details
Library version : 1.4.2  
Author : Vishnu M Aiea  
Source : https://github.com/vishnumaiea/ISL1208-RTC-Library  
Author's website : www.vishnumaiea.in  
Initial release : IST 11:49:42 AM, 27-05-2018, Sunday  
Last updated : IST 11:06 AM 25-05-2019, Saturday  
License : MIT  

### Dependancies

```sh
1. stdint.h
2. Arduino.h
3. Wire.h
```

### Constants

All the constants are defined inside the main header file. It includes the RTC's I2C slave address and the internal register addresses.
```sh
#define ISL1208_ADDRESS 0x6F  //I2C slave addess of RTC IC

#define ISL1208_SC      0x00  //seconds register
#define ISL1208_MN      0x01  //minutes register
#define ISL1208_HR      0x02  //hours register
#define ISL1208_DT      0x03  //date register
#define ISL1208_MO      0x04  //month register
#define ISL1208_YR      0x05  //year register
#define ISL1208_DW      0x06  //day of the week register

#define ISL1208_SR     0x07  //status register
#define ISL1208_INT    0x08  //interrupt register
#define ISL1208_ATR    0x0A  //analog trimming register
#define ISL1208_DTR    0x0B  //digital trimming register

#define ISL1208_SCA     0x0C  //alarm seconds register
#define ISL1208_MNA     0x0D  //alarm minutes register
#define ISL1208_HRA     0x0E  //alarm hours register
#define ISL1208_DTA     0x0F  //alarm date register
#define ISL1208_MOA     0x10  //alarm month register
#define ISL1208_DWA     0x11  //alarm day of the week register

#define ISL1208_USR1    0x12  //user memory 1
#define ISL1208_USR2    0x13  //user memory 2
```

### Variables
```sh
bool rtc_debug_enable; //enable this to get verbose output at serial monitor

byte yearValue;     //least significant digits of a year (eg. 18 for 2018, range is from 00 to 99)
byte monthValue;    //month (eg. 01 for January, range is 01 to 12)
byte dateValue;     //date (eg. 24, range is 01 to 31)
byte hourValue;     //hours (eg. 06, range is 01 to 12 for 12 hour format)
byte minuteValue;   //minutes (eg. 55, range is 00 to 59)
byte secondValue;   //seconds (eg. 30, range is 00 to 59)
byte periodValue;   //period of the day for 12 hour format (0 = AM, 1 = PM)

byte monthValueAlarm;   //same as time values
byte dateValueAlarm;
byte hourValueAlarm;
byte minuteValueAlarm;
byte secondValueAlarm;
byte periodValueAlarm;
```

All are public and so you can access them using the object directly.

### Classes
```sh
1. ISL1208_RTC
```
The main class with variables and functions.

### Functions

```sh
ISL1208_RTC(); //constructor
void begin(); //alternate initializer
bool isRtcActive(); //checks if the RTC is available on the I2C bus
bool updateTime(); //update time registers from variables
bool setTime(String); //updates time registers from a formatted time string
bool updateAlarmTime(); //updates alarm registers from variables
bool setAlarmTime(String); //updates alarm registers from a formatted alarm time string
bool fetchTime(); //reads RTC time and alarm registers and updates the variables
int getHour(); //returns the 12 format hour in DEC
int getMinute(); //returns minutes in DEC
int getSecond(); //returns seconds value
int getPeriod(); //returns time period. 0 = AM, 1 = PM
int getDate(); //returns date
int getDay(); //returns day (0 to 6)
int getMonth(); //returns month (0 to 12)
int getYear(); //returns year (00 = 2000, 99 = 2099)
int getAlarmHour();
int getAlarmMinute();
int getAlarmSecond();
int getAlarmPeriod(); //0 = AM, 1 = PM
int getAlarmDate();
int getAlarmDay();
int getAlarmMonth();
String getTimeString(); //returns formatted time string (hh:mm:ss pp)
String getDateString(); //returns formatted date string (DD-MM-YYYY)
String getDayString(); //returns the full name of day
String getDayString(int); //returns the first n chars of day string (n = 1 to 9)
String getDateDayString(); //returns a formatted date string with day name (DD-MM-YYYY DAY)
String getDateDayString(int); //returns a formatted date string with n truncated day name
String getTimeDateString(); //returns a formatted time date string
String getTimeDateDayString(); //does what it says!
String getTimeDateDayString(int); //returns a time, date string with n truncated day string
String getAlarmString();
bool printTime(); //prints time to the serial monitor
bool printAlarmTime(); //prints the alarm time to serial monitor
byte bcdToDec(byte); //converts a BCD value to DEC
byte decToBcd(byte); //converts a DEC value to BCD
```
Functions are explained below.
```sh
1. void begin(); //alternate initializer
```
This is same as the default constructor. Use this to explicitly initialize the object. It resets all the time variables.

```sh
2. bool isRtcActive(); //checks if the RTC is available on the I2C bus
```
This checks if the RTC is available on the I2C bus by reading the ACK signal.

```sh
3. bool updateTime(); //update time registers from variables
```
This updates the RTC time registers with the values present on the time variables available in the class. So if you want to set time, first save the values to the variables and then call this function.

```sh
4. bool setTime(String); //updates time registers from a formatted time string
```
This updates the time from a single formatted time string. Useful updating the time in a single command, for example from serial monitor.
``` TYYMMDDhhmmssp# ``` is the format for time string, where,
- T = indicates time information
- YY = least significant digits of a year (eg. 18 for 2018, range is from 00 to 99)
- MM = month (eg. 01 for January, range is 01 to 12)
- DD = date (eg. 24, range is 01 to 31)
- hh = hours (eg. 06, range is 01 to 12 for 12 hour format)
- mm = minutes (eg. 55, range is 00 to 59)
- ss = seconds (eg. 30, range is 00 to 59)
- p = period of the day for 12 hour format (0 = AM, 1 = PM)
- \# = delimiter

For example, to set the time and date "08:35:12 AM, 05-01-2018", we should send: ``` T1801050835120# ``` , where

- T = indicates time information
- 18 = the year 2018
- 01 = month January
- 05 = date
- 08 = hours
- 35 = minutes
- 12 = seconds
- 0 = AM
- \# = delimiter

```sh
5. bool updateAlarmTime(); //updates alarm registers from variables
```
Updates the alarm registers with the variable values.

```sh
6. bool setAlarmTime(String); //updates alarm registers from a formatted alarm time string
```

Updates the alarm registers with a formatted string like we saw before. Format is ``` AMMDDhhmmssp# ``` where,

- A = indicates alarm information
- MM = month
- DD = date
- hh = hours
- mm = minutes
- ss = seconds
- p = time period (0 = AM, 1 = PM)
- \# = delimiter

```sh
7. bool fetchTime(); //reads RTC time and alarm registers and updates the variables
```

This function reads the RTC registers and updates all the variables including the alarm values.

```sh
8. int getHour(); //returns the 12 format hour in DEC
```
All these get functions first fetch the current time, update the time variables and return the data requested. So you don't need to call fetchTime() everytime. getHour() returns the hours in 12 hour format from 1 to 12 (DEC). 24 hour support will be added later.

```sh
9. int getMinute(); //returns minutes in DEC
```
Returns the minute value in DEC format.

```sh
10. int getSecond(); //returns seconds value
```
Returns seconds in DEC format.

```sh
11. int getPeriod(); //returns time period. 0 = AM, 1 = PM
```
Returns the time period when using 12 hour format. 0 = AM, 1 = PM

```sh
12. int getDate(); //returns date
```
Returns the date in DEC format.

```sh
13. int getDay(); //returns day (0 to 6)
```
Returns the day value in DEC format. 0 = starting day of week, 6 = weekend

```sh
14. int getMonth(); //returns month (0 to 12)
```
Returns month value in DEC format.

```sh
15. int getYear(); //returns year (00 = 2000, 99 = 2099)
```
Returns the year value in DEC format. The RTC actually stores only the two least significant digits of the year in BCD format; from 00 to 99. So 99 can be interpreted as 1999 or 2099. Both will be right becasue the calenders will be same for both centuries. This function interprets 00 as 2000 and 99 as  2099. I don't know why you guys want to go to the past. May be you're building a time machine or something ?


```sh
16. int getAlarmHour();
```

At this point you know what all these functions do. I'm tired of this copy pasting.

```sh
17. int getAlarmMinute();
```

```sh
18. int getAlarmSecond();
```

```sh
19. int getAlarmPeriod(); //0 = AM, 1 = PM
```

```sh
20. int getAlarmDate();
```

```sh
21. int getAlarmDay();
```

```sh
22. int getAlarmMonth();
```

```sh
23. String getTimeString(); //returns formatted time string (hh:mm:ss pp)
```

```sh
24. String getDateString(); //returns formatted date string (DD-MM-YYYY)
```

```sh
25. String getDayString(); //returns the full name of day
```

```sh
26. String getDayString(int); //returns the first n chars of day string (n = 1 to 9)
```

```sh
27. String getDateDayString(); //returns a formatted date string with day name (DD-MM-YYYY DAY)
```

```sh
28. String getDateDayString(int); //returns a formatted date string with n truncated day name
```

```sh
29. String getTimeDateString(); //returns a formatted time date string
```

```sh
30. String getTimeDateDayString(); //does what it says!
```

```sh
31. String getTimeDateDayString(int); //returns a time, date string with n truncated day string
```

```sh
32. String getAlarmString();
```

```sh
33. bool printTime(); //prints time to the serial monitor
```

Prints a formatted time string just after fetching the current time.

```sh
34. bool printAlarmTime(); //prints the alarm time to serial monitor
```

Prints alarm time.

```sh
35. byte bcdToDec(byte); //converts a BCD value to DEC
```

The RTC registers save values in BCD format. So we need to convert to and from BCD when we read or write the alarm registers. This function converts BCD values to DEC.

```sh
36. byte decToBcd(byte); //converts a DEC value to BCD
```

This does the opposite.

Finally, here's the register map of the RTC - https://hackster.imgix.net/uploads/attachments/405702/isl1208_2_f51GNl47GW.png

# Known Issues
### 24 Hour format support

I have not added 24 hour format support for now. The RTC supports it though. I might add this in future if I find it any useful. If you can do it yourself, let me know ;)

