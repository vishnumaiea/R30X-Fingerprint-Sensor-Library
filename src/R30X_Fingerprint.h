
//=========================================================================//
//                                                                         //
//  ## R30X Fingerprint Sensor Library ##                                  //
//                                                                         //
//  Filename : R30X_Fingerprint.h                                          //
//  Description : Header file for R30X_Fingerprint library for R30X series //
//                fingerprint sensors.                                     //
//  Library version : 1.2.0                                                //
//  Author : Vishnu M Aiea                                                 //
//  Src : https://github.com/vishnumaiea/R30X-Fingerprint-Sensor-Library   //
//  Author's website : https://www.vishnumaiea.in                          //
//  Initial release : IST 07:35 PM, 08-04-2019, Monday                     //
//  License : MIT                                                          //
//                                                                         //
//  Last modified : IST 08:21 AM 12-12-2019, Thursday                      //
//                                                                         //
//=========================================================================//

#ifndef R30X_FINGERPRINT_H
#define R30X_FINGERPRINT_H

#include "Arduino.h"

#ifndef HAVE_HWSERIAL1  //if more than one hardware serial ports are not present
  #include "SoftwareSerial.h"
#endif

//=========================================================================//

// #if !defined(ARDUINO_AVR_UNO) && !defined(ARDUINO_AVR_MINI) && !defined(ARDUINO_AVR_NANO)
//   #define FPS_DEBUG   //uncomment this line to enable debug info to be printed
// #endif

// #define FPS_DEBUG   //uncomment this line to enable debug info to be printed

#define debugPort Serial  //the serisl port to which debug info will be sent

//=========================================================================//
//Response codes from FPS to the commands sent to it
//FPS = Fingerprint Scanner

#define FPS_RESP_OK                      0x00U   //command executed successfully
#define FPS_RESP_RECIEVEERR              0x01U   //packet receive error
#define FPS_RESP_NOFINGER                0x02U   //no finger detected
#define FPS_RESP_ENROLLFAIL              0x03U   //failed to enroll the finger
#define FPS_RESP_OVERDISORDERFAIL        0x04U   //failed to generate character file due to over-disorderly fingerprint image
#define FPS_RESP_OVERWETFAIL             0x05U   //failed to generate character file due to over-wet fingerprint image
#define FPS_RESP_OVERDISORDERFAIL2       0x06U   //failed to generate character file due to over-disorderly fingerprint image
#define FPS_RESP_FEATUREFAIL             0x07U   //failed to generate character file due to over-wet fingerprint image
#define FPS_RESP_DONOTMATCH              0x08U   //fingers do not match
#define FPS_RESP_NOTFOUND                0x09U   //no valid match found
#define FPS_RESP_ENROLLMISMATCH          0x0AU   //failed to combine character files (two character files (images) are used to create a template)
#define FPS_RESP_BADLOCATION             0x0BU   //addressing PageID is beyond the finger library
#define FPS_RESP_INVALIDTEMPLATE         0x0CU   //error when reading template from library or the template is invalid
#define FPS_RESP_TEMPLATEUPLOADFAIL      0x0DU   //error when uploading template
#define FPS_RESP_PACKETACCEPTFAIL        0x0EU   //module can not accept more packets
#define FPS_RESP_IMAGEUPLOADFAIL         0x0FU   //error when uploading image
#define FPS_RESP_TEMPLATEDELETEFAIL      0x10U   //error when deleting template
#define FPS_RESP_DBCLEARFAIL             0x11U   //failed to clear fingerprint library
#define FPS_RESP_WRONGPASSOWRD           0x13U   //wrong password
#define FPS_RESP_IMAGEGENERATEFAIL       0x15U   //fail to generate the image due to lackness of valid primary image
#define FPS_RESP_FLASHWRITEERR           0x18U   //error when writing flash
#define FPS_RESP_NODEFINITIONERR         0x19U   //no definition error
#define FPS_RESP_INVALIDREG              0x1AU   //invalid register number
#define FPS_RESP_INCORRECTCONFIG         0x1BU   //incorrect configuration of register
#define FPS_RESP_WRONGNOTEPADPAGE        0x1CU   //wrong notepad page number
#define FPS_RESP_COMPORTERR              0x1DU   //failed to operate the communication port
#define FPS_RESP_INVALIDREG              0x1AU   //invalid register number
#define FPS_RESP_SECONDSCANNOFINGER      0x41U   //secondary fingerprint scan failed due to no finger
#define FPS_RESP_SECONDENROLLFAIL        0x42U   //failed to enroll second fingerprint
#define FPS_RESP_SECONDFEATUREFAIL       0x43U   //failed to generate character file due to lack of enough features
#define FPS_RESP_SECONDOVERDISORDERFAIL  0x44U   //failed to generate character file due to over-disorderliness
#define FPS_RESP_DUPLICATEFINGERPRINT    0x45U   //duplicate fingerprint

//-------------------------------------------------------------------------//
//Received packet verification status codes from host device

#define FPS_RX_OK                        0x00U  //when the response is correct
#define FPS_RX_BADPACKET                 0x01U  //if the packet received from FPS is badly formatted
#define FPS_RX_WRONG_RESPONSE            0x02U  //unexpected response
#define FPS_RX_TIMEOUT                   0x03U  //when no response was received

//-------------------------------------------------------------------------//
//Packet IDs

#define FPS_ID_STARTCODE              0xEF01U
#define FPS_ID_STARTCODEHIGH          0xEFU
#define FPS_ID_STARTCODELOW           0x01U
#define FPS_ID_COMMANDPACKET          0x01U
#define FPS_ID_DATAPACKET             0x02U
#define FPS_ID_ACKPACKET              0x07U
#define FPS_ID_ENDDATAPACKET          0x08U

//-------------------------------------------------------------------------//
//Command codes

#define FPS_CMD_SCANFINGER            0x01U    //scans the finger and collect finger image
#define FPS_CMD_IMAGETOCHARACTER      0x02U    //generate char file from a single image and store it to one of the buffers
#define FPS_CMD_MATCHTEMPLATES        0x03U    //match two fingerprints precisely
#define FPS_CMD_SEARCHLIBRARY         0x04U    //search the fingerprint library
#define FPS_CMD_GENERATETEMPLATE      0x05U    //combine both character buffers and generate a template
#define FPS_CMD_STORETEMPLATE         0x06U    //store the template on one of the buffers to flash memory
#define FPS_CMD_LOADTEMPLATE          0x07U    //load a template from flash memory to one of the buffers
#define FPS_CMD_EXPORTTEMPLATE        0x08U    //export a template file from buffer to computer
#define FPS_CMD_IMPORTTEMPLATE        0x09U    //import a template file from computer to sensor buffer
#define FPS_CMD_EXPORTIMAGE           0x0AU    //export fingerprint image from buffer to computer
#define FPS_CMD_IMPORTIMAGE           0x0BU    //import an image from computer to sensor buffer
#define FPS_CMD_DELETETEMPLATE        0x0CU    //delete a template from flash memory
#define FPS_CMD_CLEARLIBRARY          0x0DU    //clear fingerprint library
#define FPS_CMD_SETSYSPARA            0x0EU    //set system configuration register
#define FPS_CMD_READSYSPARA           0x0FU    //read system configuration register
#define FPS_CMD_SETPASSWORD           0x12U    //set device password
#define FPS_CMD_VERIFYPASSWORD        0x13U    //verify device password
#define FPS_CMD_GETRANDOMCODE         0x14U    //get random code from device
#define FPS_CMD_SETDEVICEADDRESS      0x15U    //set 4 byte device address
#define FPS_CMD_PORTCONTROL           0x17U    //enable or disable comm port
#define FPS_CMD_WRITENOTEPAD          0x18U    //write to device notepad
#define FPS_CMD_READNOTEPAD           0x19U    //read from device notepad
#define FPS_CMD_HISPEEDSEARCH         0x1BU    //highspeed search of fingerprint
#define FPS_CMD_TEMPLATECOUNT         0x1DU    //read total template count
#define FPS_CMD_SCANANDRANGESEARCH    0x32U    //read total template count
#define FPS_CMD_SCANANDFULLSEARCH     0x34U    //read total template count

#define FPS_DEFAULT_TIMEOUT                 2000  //UART reading timeout in milliseconds
#define FPS_DEFAULT_BAUDRATE                57600 //9600*6
#define FPS_DEFAULT_RX_DATA_LENGTH          64    //the max length of data in a received packet
#define FPS_DEFAULT_SECURITY_LEVEL          3     //the threshold at which the fingerprints will be matched
#define FPS_DEFAULT_SERIAL_BUFFER_LENGTH    300   //length of the buffer used to read the serial data
#define FPS_DEFAULT_PASSWORD                0xFFFFFFFF
#define FPS_DEFAULT_ADDRESS                 0xFFFFFFFF
#define FPS_BAD_VALUE                       0x1FU //some bad value or paramter was delivered

//=========================================================================//
//main class

class R30X_Fingerprint {
  public:

  #if defined(__AVR__) || defined(ESP8266)
    R30X_Fingerprint (SoftwareSerial *ss, uint32_t password = FPS_DEFAULT_PASSWORD, uint32_t address = FPS_DEFAULT_ADDRESS);
  #endif
  
  R30X_Fingerprint (HardwareSerial *hs, uint32_t password = FPS_DEFAULT_PASSWORD, uint32_t address = FPS_DEFAULT_ADDRESS);

  //common parameters
  uint32_t devicePasswordL; //32-bit single value version of password (L = long)
  uint32_t deviceAddressL;  //module's address
  uint16_t startCodeL; //packet start marker
  uint8_t devicePassword[4]; //array version of password
  uint8_t deviceAddress[4]; //device address as an array
  uint8_t startCode[2]; //packet start marker
  uint32_t deviceBaudrate;  //UART speed
  uint8_t securityLevel;  //threshold level for fingerprint matching
  uint16_t dataPacketLength; //the max length of data in packet. can be 32, 64, 128 or 256

  //transmit packet parameters
  uint8_t txPacketType; //type of packet
  uint16_t txPacketLengthL; //length of packet (Data + Checksum)
  uint8_t txInstructionCode;  //instruction to be sent to FPS
  uint16_t txPacketChecksumL; //checksum long value
  uint8_t txPacketLength[2];  //packet length as an array
  uint8_t* txDataBuffer; //packet data buffer
  uint16_t txDataBufferLength; //length of actual data in a packet
  uint8_t txPacketChecksum[2];  //packet checksum as an array

  //receive packet parameters
  uint8_t rxPacketType; //type of packet
  uint16_t rxPacketLengthL; //packet length long
  uint8_t rxConfirmationCode; //the return codes from the FPS
  uint16_t rxPacketChecksumL; //packet checksum long
  uint8_t rxPacketLength[2];  //packet length as an array
  uint8_t* rxDataBuffer; //packet data buffer
  uint32_t rxDataBufferLength;  //the length of the data only. this doesn't include instruction or confirmation code
  uint8_t rxPacketChecksum[2];  //packet checksum as array

  uint16_t fingerId; //location of fingerprint in the library
  uint16_t matchScore;  //the match score of comparison of two fingerprints
  uint16_t templateCount; //total number of fingerprint templates in the library
  uint16_t statusRegister;  //contents of the FPS status register

  void begin (uint32_t baud); //initializes the communication port
  void resetParameters (void); //initialize and reset and all parameters
  uint8_t verifyPassword (uint32_t password = FPS_DEFAULT_PASSWORD); //verify the user supplied password
  uint8_t setPassword (uint32_t password);  //set FPS password
  uint8_t setAddress (uint32_t address = FPS_DEFAULT_ADDRESS);  //set FPS address
  uint8_t setBaudrate (uint32_t baud);  //set UART baudrate, default is 57000
  uint8_t setSecurityLevel (uint8_t level); //set the threshold for fingerprint matching
  uint8_t setDataLength (uint16_t length); //set the max length of data in a packet
  uint8_t portControl (uint8_t value);  //turn the comm port on or off
  uint8_t sendPacket (uint8_t type, uint8_t command, uint8_t* data = NULL, uint16_t dataLength = 0); //assemble and send packets to FPS
  uint8_t receivePacket (uint32_t timeout=FPS_DEFAULT_TIMEOUT); //receive packet from FPS
  uint8_t readSysPara (void); //read FPS system configuration
  uint8_t captureAndRangeSearch (uint16_t captureTimeout, uint16_t startId, uint16_t count); //scan a finger and search a range of locations
  uint8_t captureAndFullSearch (void);  //scan a finger and search the entire library
  uint8_t generateImage (void); //scan a finger, generate an image and store it in the buffer
  uint8_t exportImage (void); //export a fingerprint image from the sensor to the computer
  uint8_t importImage (uint8_t* dataBuffer);  //import a fingerprint image from the computer to sensor
  uint8_t generateCharacter (uint8_t bufferId); //generate character file from image
  uint8_t generateTemplate (void);  //combine the two character files and generate a single template
  uint8_t exportCharacter (uint8_t bufferId); //export a character file from the sensor to computer
  uint8_t importCharacter (uint8_t bufferId, uint8_t* dataBuffer);  //import a character file to the sensor from computer
  uint8_t saveTemplate (uint8_t bufferId, uint16_t location);  //store the template in the buffer to a location in the library
  uint8_t loadTemplate (uint8_t bufferId, uint16_t location); //load a template from library to one of the buffers
  uint8_t deleteTemplate (uint16_t startLocation, uint16_t count);  //delete a set of templates from library
  uint8_t clearLibrary (void);  //delete all templates from library
  uint8_t matchTemplates (void);  //match the templates stored in the two character buffers
  uint8_t searchLibrary (uint8_t bufferId, uint16_t startLocation, uint16_t count); //search the library for a template stored in the buffer
  uint8_t getTemplateCount (void);  //get the total no. of templates in the library

  private:

  Stream *mySerial; //stream class is used to facilitate communication

  #if defined(__AVR__) || defined(ESP8266)
    SoftwareSerial *swSerial; //for those devices with only one hardware UART
  #endif
  
  HardwareSerial *hwSerial; //for those devices with multiple hardware UARTs
};

//=========================================================================//

#endif

//=========================================================================//