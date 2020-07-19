
//=========================================================================//
//                                                                         //
//  ## R30X Fingerprint Sensor Library ##                                  //
//                                                                         //
//  Filename : R30X_FPS.cpp                                        //
//  Description : CPP file for R30X_FPS library for R30X series    //
//                fingerprint sensors.                                     //
//  Library version : 1.2.0                                                //
//  Author : Vishnu M Aiea                                                 //
//  Src : https://github.com/vishnumaiea/R30X-Fingerprint-Sensor-Library   //
//  Author's website : https://www.vishnumaiea.in                          //
//  Initial release : IST 07:35 PM, 08-04-2019, Monday                     //
//  License : MIT                                                          //
//                                                                         //
//  Last modified : +05:30 06:59:32 PM, 19-07-2020 Sunday
//                                                                         //
//=========================================================================//

#include "R30X_FPS.h"

//=========================================================================//
//constructor for SoftwareSerial interface

#if defined(__AVR__) || defined(ESP8266)
  R30X_FPS::R30X_FPS (SoftwareSerial *ss, uint32_t password, uint32_t address) {
    hwSerial = NULL;  //set to null since we won't be using hardware serial
    swSerial = ss;
    mySerial = swSerial;  //will be working with sw serial

    //storing 32-bit values as 8-bit values in arrays can make many operations easier later
    devicePassword[0] = password & 0xFFU;
    devicePassword[1] = (password >> 8) & 0xFFU;
    devicePassword[2] = (password >> 16) & 0xFFU;
    devicePassword[3] = (password >> 24) & 0xFFU;
    devicePasswordL = password;

    deviceAddress[0] = address & 0xFFU;
    deviceAddress[1] = (address >> 8) & 0xFFU;
    deviceAddress[2] = (address >> 16) & 0xFFU;
    deviceAddress[3] = (address >> 24) & 0xFFU;
    deviceAddressL = address;

    startCode[0] = FPS_ID_STARTCODE & 0xFFU; //packet start marker
    startCode[1] = (FPS_ID_STARTCODE >> 8) & 0xFFU;

    resetParameters(); //initialize and reset and all parameters
  }
#endif

//=========================================================================//
//constructor for hardware serial interface

R30X_FPS::R30X_FPS (HardwareSerial *hs, uint32_t password, uint32_t address) {
  #if defined(__AVR__) || defined(ESP8266)
    swSerial = NULL;
  #endif
  hwSerial = hs;
  mySerial = hwSerial;

  //storing 32-bit values as 8-bit values in arrays can make many operations easier later
  devicePassword[0] = password & 0xFFU; //these can be altered later
  devicePassword[1] = (password >> 8) & 0xFFU;
  devicePassword[2] = (password >> 16) & 0xFFU;
  devicePassword[3] = (password >> 24) & 0xFFU;
  devicePasswordL = password;

  deviceAddress[0] = address & 0xFFU;
  deviceAddress[1] = (address >> 8) & 0xFFU;
  deviceAddress[2] = (address >> 16) & 0xFFU;
  deviceAddress[3] = (address >> 24) & 0xFFU;
  deviceAddressL = address;

  startCode[0] = FPS_ID_STARTCODE & 0xFFU; //packet start marker
  startCode[1] = (FPS_ID_STARTCODE >> 8) & 0xFFU;

  resetParameters();  //initialize and reset and all parameters
}

//=========================================================================//
//initializes the serial port
//the baudrate received here will override the default one

void R30X_FPS::begin (uint32_t baudrate) {
  delay(1000);  //one second delay to let the sensor 'boot up'

  deviceBaudrate = baudrate;  //save the new baudrate

  if (hwSerial) hwSerial->begin(baudrate);

  #if defined(__AVR__) || defined(ESP8266)
    if (swSerial) swSerial->begin(baudrate);
  #endif
}

//=========================================================================//
//reset some parameters

void R30X_FPS::resetParameters (void) {
  deviceBaudrate = FPS_DEFAULT_BAUDRATE;  //this will be later altered by begin()
  securityLevel = FPS_DEFAULT_SECURITY_LEVEL;  //threshold level for fingerprint matching
  dataPacketLength = FPS_DEFAULT_RX_DATA_LENGTH;

  txPacketType = FPS_ID_COMMANDPACKET; //type of packet
  txInstructionCode = FPS_CMD_VERIFYPASSWORD; //
  txPacketLength[0] = 0;
  txPacketLength[1] = 0;
  txPacketLengthL = 0;
  txDataBuffer = NULL; //packet data buffer
  txDataBufferLength = 0;
  txPacketChecksum[0] = 0;
  txPacketChecksum[1] = 0;
  txPacketChecksumL = 0;

  rxPacketType = FPS_ID_COMMANDPACKET; //type of packet
  rxConfirmationCode = FPS_CMD_VERIFYPASSWORD; //
  rxPacketLength[0] = 0;
  rxPacketLength[1] = 0;
  rxPacketLengthL = 0;
  rxDataBuffer = NULL; //packet data buffer
  rxDataBufferLength = 0;
  rxPacketChecksum[0] = 0;
  rxPacketChecksum[1] = 0;
  rxPacketChecksumL = 0;

  fingerId = 0; //initialize them
  matchScore = 0;
  templateCount = 0;
}

//=========================================================================//
//send a data packet to the FPS (fingerprint scanner)

uint8_t R30X_FPS::sendPacket (uint8_t type, uint8_t command, uint8_t* data, uint16_t dataLength) {
  if(data != NULL) {  //sometimes there's no additional data except the command
    txDataBuffer = data;
    txDataBufferLength = dataLength;
  }
  else {
    txDataBuffer = NULL;
    txDataBufferLength = 0;
  }

  txPacketType = type; //type of packet - 1 byte
  txInstructionCode = command; //instruction code - 1 byte
  txPacketLengthL = txDataBufferLength + 3; //1 byte for command, 2 bytes for checksum
  txPacketLength[0] = txPacketLengthL & 0xFFU; //get lower byte
  txPacketLength[1] = (txPacketLengthL >> 8) & 0xFFU; //get high byte

  txPacketChecksumL = txPacketType + txPacketLength[0] + txPacketLength[1] + txInstructionCode; //sum of packet ID and packet length bytes

  for(int i=0; i<txDataBufferLength; i++) {
    txPacketChecksumL += txDataBuffer[i]; //add rest of the data bytes
  }

  txPacketChecksum[0] = txPacketChecksumL & 0xFFU; //get low byte
  txPacketChecksum[1] = (txPacketChecksumL >> 8) & 0xFFU; //get high byte

  mySerial->write(startCode[1]); //high byte is sent first
  mySerial->write(startCode[0]);
  mySerial->write(deviceAddress[3]); //high byte is sent first
  mySerial->write(deviceAddress[2]);
  mySerial->write(deviceAddress[1]);
  mySerial->write(deviceAddress[0]);
  mySerial->write(txPacketType);
  mySerial->write(txPacketLength[1]); //high byte is sent first
  mySerial->write(txPacketLength[0]);
  mySerial->write(txInstructionCode);

  for(int i=(txDataBufferLength-1); i>=0; i--) {
    mySerial->write(txDataBuffer[i]); //send high byte first
  }

  mySerial->write(txPacketChecksum[1]);
  mySerial->write(txPacketChecksum[0]);

  #ifdef FPS_DEBUG
    debugPort.print(F("Sent packet = "));
    debugPort.print(startCode[1], HEX); //high byte is sent first
    debugPort.print(F("-"));
    debugPort.print(startCode[0], HEX);
    debugPort.print(F("-"));
    debugPort.print(deviceAddress[3], HEX); //high byte is sent first
    debugPort.print(F("-"));
    debugPort.print(deviceAddress[2], HEX);
    debugPort.print(F("-"));
    debugPort.print(deviceAddress[1], HEX);
    debugPort.print(F("-"));
    debugPort.print(deviceAddress[0], HEX);
    debugPort.print(F("-"));
    debugPort.print(txPacketType, HEX);
    debugPort.print(F("-"));
    debugPort.print(txPacketLength[1], HEX); //high byte is sent first
    debugPort.print(F("-"));
    debugPort.print(txPacketLength[0], HEX);
    debugPort.print(F("-"));
    debugPort.print(txInstructionCode, HEX);
    debugPort.print(F("-"));

    for(int i=(txDataBufferLength-1); i>=0; i--) {
      debugPort.print(txDataBuffer[i], HEX); //send high byte first
      debugPort.print(F("-"));
    }

    debugPort.print(txPacketChecksum[1], HEX);
    debugPort.print(F("-"));
    debugPort.print(txPacketChecksum[0], HEX);
    debugPort.println();
    debugPort.print(F("txInstructionCode = "));
    debugPort.println(txInstructionCode, HEX);
    debugPort.print(F("txDataBufferLength = "));
    debugPort.println(txDataBufferLength, HEX);
    debugPort.print(F("txPacketLengthL = "));
    debugPort.println(txPacketLengthL);
    // debugPort.print(F("rxPacketLength[] = "));
    // debugPort.print(rxPacketLength[1], HEX);
    // debugPort.print(F("-"));
    // debugPort.println(rxPacketLength[0], HEX);
  #endif

  return FPS_RX_OK;
}

//=========================================================================//
//receive a data packet from the FPS and extract values

uint8_t R30X_FPS::receivePacket (uint32_t timeout) {
  uint8_t* dataBuffer;
  if(dataPacketLength < 64) { //data buffer length should be at least 64 bytes
    dataBuffer = new uint8_t[64](); //this contains only the data
  }
  else {
    dataBuffer = new uint8_t[FPS_DEFAULT_RX_DATA_LENGTH]();
  }

  rxDataBuffer = dataBuffer;
  uint8_t serialBuffer[FPS_DEFAULT_SERIAL_BUFFER_LENGTH] = {0}; //serialBuffer will store high byte at the start of the array
  uint16_t serialBufferLength = 0;
  uint8_t byteBuffer = 0;

  #ifdef FPS_DEBUG
    debugPort.println();
    debugPort.println(F("Reading response."));
  #endif

  //wait for message for a specific period
  while (timeout > 0) {
    if(mySerial->available()) {
      byteBuffer = mySerial->read();
      #ifdef FPS_DEBUG
        // debugPort.print(F("Response byte found = "));
        // debugPort.println(byteBuffer, HEX);
      #endif
      serialBuffer[serialBufferLength] = byteBuffer;
      serialBufferLength++;
    }

    timeout--;
    delay(1);
  }

  if(serialBufferLength == 0) {
    #ifdef FPS_DEBUG
      debugPort.println(F("Serial timed out."));
      debugPort.println(F("This usually means the baud rate is not correct."));
    #endif
    return FPS_RX_TIMEOUT;
  }

  if(serialBufferLength < 10) {
    #ifdef FPS_DEBUG
      debugPort.println(F("Received bad packet with length < 10"));
    #endif
    return FPS_RX_BADPACKET;
  }

  uint16_t token = 0; //a position counter/indicator

  //the following loop checks each segments of the data packet for errors, and retrieve the correct ones
  while(true) {
    switch (token) {
      case 0: //test packet start codes
        if(serialBuffer[token] == startCode[1])
          break;
        else {
          #ifdef FPS_DEBUG //enable it to get debug info
            debugPort.println(F("Error at 0 : Start Code"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }

      case 1:
        if(serialBuffer[token] == startCode[0])
          break;
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 1 : Start Code"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }

      case 2: //test device address
        if(serialBuffer[token] == deviceAddress[3])
          break;
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 2 : Device Address"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }
      
      case 3:
        if(serialBuffer[token] == deviceAddress[2])
          break;
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 3 : Device Address"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }

      case 4:
        if(serialBuffer[token] == deviceAddress[1])
          break;
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 4 : Device Address"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }
      
      case 5:
        if(serialBuffer[token] == deviceAddress[0])
          break;
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 5 : Device Address"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;
        }

      case 6: //test for valid packet type
        if((serialBuffer[token] == FPS_ID_COMMANDPACKET) || (serialBuffer[token] == FPS_ID_DATAPACKET) || (serialBuffer[token] == FPS_ID_ACKPACKET) || (serialBuffer[token] == FPS_ID_ENDDATAPACKET)) {
          rxPacketType = serialBuffer[token]; //store the packet ID to class variable
          break;
        }
        else {
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 6 : Unknown Response"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_WRONG_RESPONSE;
        }

      case 7: //read packet data length
        if((serialBuffer[token] > 0) || (serialBuffer[token + 1] > 0)) {
          rxPacketLength[0] = serialBuffer[token + 1];  //lower byte
          rxPacketLength[1] = serialBuffer[token];  //higher byte
          rxPacketLengthL = uint16_t(rxPacketLength[1] << 8) + rxPacketLength[0]; //calculate the full length value
          rxDataBufferLength = rxPacketLengthL - 3; //subtract 2 for checksum and 1 for command
          token++; //because we read one additional bytes here
          break;
        }

        else {
         #ifdef FPS_DEBUG
            debugPort.println(F("Error at 7 : Unknown Response"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_WRONG_RESPONSE;
        }

      case 9: //read confirmation or instruction code
        rxConfirmationCode = serialBuffer[token]; //the first byte of data will be either instruction or confirmation code
        break;

      case 10: //read data
        for(int i=0; i < rxDataBufferLength; i++) {
          rxDataBuffer[(rxDataBufferLength - 1) - i] = serialBuffer[token + i]; //store low values at start of the rxDataBuffer array
        }
        break;
      
      case 11: //read checksum
        if(rxDataBufferLength == 0) { //sometimes there's no data other than the confirmation code
          rxPacketChecksum[0] = serialBuffer[token]; //lower byte
          rxPacketChecksum[1] = serialBuffer[token - 1]; //high byte
          rxPacketChecksumL = uint16_t(rxPacketChecksum[1] << 8) + rxPacketChecksum[0]; //calculate L value

          uint16_t tempSum = 0; //temp checksum 

          tempSum = rxPacketType + rxPacketLength[0] + rxPacketLength[1] + rxConfirmationCode;

          if(rxPacketChecksumL == tempSum) { //check if the calculated checksum matches the received one
            #ifdef FPS_DEBUG
              debugPort.println(F("Checksum matching success."));
              debugPort.print(F("Received = "));
              debugPort.print(rxPacketChecksum[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketChecksum[0], HEX);
              debugPort.print(F("Received L = "));
              debugPort.println(rxPacketChecksumL, HEX);
              debugPort.print(F("Calculated = "));
              debugPort.print(byte(tempSum >> 8), HEX);
              debugPort.print(F("-"));
              debugPort.println(byte(tempSum & 0xFFU), HEX);
              debugPort.print(F("Calculated L = "));
              debugPort.println(tempSum, HEX);
              debugPort.print(F("Received packet = "));

              for(int i=0; i < serialBufferLength; i++) {
                debugPort.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }
              debugPort.println();
              debugPort.print(F("Data stream = none"));

              debugPort.println();
              debugPort.print(F("rxConfirmationCode = "));
              debugPort.println(rxConfirmationCode, HEX);
              debugPort.print(F("rxDataBufferLength = "));
              debugPort.println(rxDataBufferLength, HEX);
              debugPort.print(F("rxPacketLengthL = "));
              debugPort.println(rxPacketLengthL);
              debugPort.print(F("rxPacketLength[] = "));
              debugPort.print(rxPacketLength[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketLength[0], HEX);
            #endif

            return FPS_RX_OK; //packet read success
          }

          else { //if the checksums do not match
            #ifdef FPS_DEBUG
              debugPort.println(F("Checksums match fail."));
              debugPort.print(F("Received = "));
              debugPort.print(rxPacketChecksum[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketChecksum[0], HEX);
              debugPort.print(F("Received L = "));
              debugPort.println(rxPacketChecksumL, HEX);
              debugPort.print(F("Calculated = "));
              debugPort.print(byte(tempSum >> 8), HEX);
              debugPort.print(F("-"));
              debugPort.println(byte(tempSum & 0xFFU), HEX);
              debugPort.print(F("Calculated L = "));
              debugPort.println(tempSum, HEX);
              debugPort.print(F("Received packet = "));

              for(int i=0; i < serialBufferLength; i++) {
                debugPort.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }
              debugPort.println();
              debugPort.print(F("Data stream = none"));
              
              debugPort.println();
              debugPort.print(F("rxConfirmationCode = "));
              debugPort.println(rxConfirmationCode, HEX);
              debugPort.print(F("rxDataBufferLength = "));
              debugPort.println(rxDataBufferLength, HEX);
              debugPort.print(F("rxPacketLengthL = "));
              debugPort.println(rxPacketLengthL, HEX);
              debugPort.print(F("rxPacketLength[] = "));
              debugPort.print(rxPacketLength[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketLength[0], HEX);
            #endif

            return FPS_RX_BADPACKET;  //then that's an error
          }
          break;
        }

        //-------------------------------------------------------------------------//

        else if((serialBuffer[token + (rxDataBufferLength-1)] > 0) || ((serialBuffer[token + 1 + (rxDataBufferLength-1)] > 0))) {
          rxPacketChecksum[0] = serialBuffer[token + 1 + (rxDataBufferLength-1)]; //lower byte
          rxPacketChecksum[1] = serialBuffer[token + (rxDataBufferLength-1)]; //high byte
          rxPacketChecksumL = uint16_t(rxPacketChecksum[1] << 8) + rxPacketChecksum[0]; //calculate L value

          uint16_t tempSum = 0; //temp checksum 

          tempSum = rxPacketType + rxPacketLength[0] + rxPacketLength[1] + rxConfirmationCode;

          for(int i=0; i < rxDataBufferLength; i++) {
            tempSum += rxDataBuffer[i]; //calculate data checksum
          }

          if(rxPacketChecksumL == tempSum) { //check if the calculated checksum matches the received one
            #ifdef FPS_DEBUG
              debugPort.println(F("Checksums match success."));
              debugPort.print(F("Received = "));
              debugPort.print(rxPacketChecksum[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketChecksum[0], HEX);
              debugPort.print(F("Received L = "));
              debugPort.println(rxPacketChecksumL, HEX);
              debugPort.print(F("Calculated = "));
              debugPort.print(byte(tempSum >> 8), HEX);
              debugPort.print(F("-"));
              debugPort.println(byte(tempSum & 0xFFU), HEX);
              debugPort.print(F("Calculated L = "));
              debugPort.println(tempSum, HEX);
              debugPort.print(F("Received packet = "));

              for(int i=0; i < serialBufferLength; i++) {
                debugPort.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }
              debugPort.println();
              debugPort.print(F("Data stream = "));

              for(int i=0; i < rxDataBufferLength; i++) {
                debugPort.print(rxDataBuffer[(rxDataBufferLength-1) - i], HEX);
                if(i != (rxDataBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }

              debugPort.println();
              debugPort.print(F("rxConfirmationCode = "));
              debugPort.println(rxConfirmationCode, HEX);
              debugPort.print(F("rxDataBufferLength = "));
              debugPort.println(rxDataBufferLength, HEX);
              debugPort.print(F("rxPacketLengthL = "));
              debugPort.println(rxPacketLengthL, HEX);
              debugPort.print(F("rxPacketLength[] = "));
              debugPort.print(rxPacketLength[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketLength[0], HEX);
            #endif

            return FPS_RX_OK; //packet read success
          }

          else { //if the checksums do not match
            #ifdef FPS_DEBUG
              debugPort.println(F("Checksums match fail."));
              debugPort.print(F("Received = "));
              debugPort.print(rxPacketChecksum[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketChecksum[0], HEX);
              debugPort.print(F("Received L = "));
              debugPort.println(rxPacketChecksumL, HEX);
              debugPort.print(F("Calculated = "));
              debugPort.print(byte(tempSum >> 8), HEX);
              debugPort.print(F("-"));
              debugPort.println(byte(tempSum & 0xFFU), HEX);
              debugPort.print(F("Calculated L = "));
              debugPort.println(tempSum, HEX);
              debugPort.print(F("Received packet = "));

              for(int i=0; i < serialBufferLength; i++) {
                debugPort.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }
              debugPort.println();
              debugPort.print(F("Data stream = "));

              for(int i=0; i < rxDataBufferLength; i++) {
                debugPort.print(rxDataBuffer[(rxDataBufferLength-1) - i], HEX);
                if(i != (rxDataBufferLength - 1)) {
                  debugPort.print(F("-"));
                }
              }
              
              debugPort.println();
              debugPort.print(F("rxConfirmationCode = "));
              debugPort.println(rxConfirmationCode, HEX);
              debugPort.print(F("rxDataBufferLength = "));
              debugPort.println(rxDataBufferLength, HEX);
              debugPort.print(F("rxPacketLengthL = "));
              debugPort.println(rxPacketLengthL, HEX);
              debugPort.print(F("rxPacketLength[] = "));
              debugPort.print(rxPacketLength[1], HEX);
              debugPort.print(F("-"));
              debugPort.println(rxPacketLength[0], HEX);
            #endif

            return FPS_RX_BADPACKET;  //then that's an error
          }
          break;
        }

        //-------------------------------------------------------------------------//

        else { //if the checksum received is 0
          #ifdef FPS_DEBUG
            debugPort.println(F("Error at 12 : Checksum"));
            debugPort.print(F("Received packet = "));
            for(int i=0; i < serialBufferLength; i++) {
              debugPort.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                debugPort.print(F("-"));
              }
            }
            debugPort.println();
          #endif

          return FPS_RX_BADPACKET;  //that too an error
        }
        break;
    
      default:
        break;
    }
    token++; //increment to progressively scan the packet
  }
}

//=========================================================================//
//verify if the password set by user is correct

uint8_t R30X_FPS::verifyPassword (uint32_t inputPassword) {
  uint8_t inputPasswordBytes[4] = {0};  //to store the split password
  inputPasswordBytes[0] = inputPassword & 0xFFU;  //save each bytes
  inputPasswordBytes[1] = (inputPassword >> 8) & 0xFFU;
  inputPasswordBytes[2] = (inputPassword >> 16) & 0xFFU;
  inputPasswordBytes[3] = (inputPassword >> 24) & 0xFFU;

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_VERIFYPASSWORD, inputPasswordBytes, 4); //send the command and data

  uint8_t response = receivePacket(); //read response
  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) {
      //save the input password if it is correct
      //this is actually redundant, but can make sure the right password is available to execute further commands
      devicePasswordL = inputPassword;
      devicePassword[0] = inputPasswordBytes[0]; //save the new password as array
      devicePassword[1] = inputPasswordBytes[1];
      devicePassword[2] = inputPasswordBytes[2];
      devicePassword[3] = inputPasswordBytes[3];

      #ifdef FPS_DEBUG
        debugPort.println(F("Password is correct."));
        debugPort.print(F("Current Password = "));
        debugPort.println(devicePasswordL, HEX);
      #endif

      return FPS_RESP_OK; //password is correct
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Password is not correct."));
        debugPort.print(F("Tested Password = "));
        debugPort.println(inputPassword, HEX);
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif

      return rxConfirmationCode;  //password is not correct and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//set a new 4 byte password

uint8_t R30X_FPS::setPassword (uint32_t inputPassword) {
  uint8_t inputPasswordBytes[4] = {0};
  inputPasswordBytes[0] = inputPassword & 0xFFU;
  inputPasswordBytes[1] = (inputPassword >> 8) & 0xFFU;
  inputPasswordBytes[2] = (inputPassword >> 16) & 0xFFU;
  inputPasswordBytes[3] = (inputPassword >> 24) & 0xFFU;

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETPASSWORD, inputPasswordBytes, 4); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confrim code will be saved when the response is received
      devicePasswordL = inputPassword; //save the new password (Long)
      devicePassword[0] = inputPasswordBytes[0]; //save the new password as array
      devicePassword[1] = inputPasswordBytes[1];
      devicePassword[2] = inputPasswordBytes[2];
      devicePassword[3] = inputPasswordBytes[3];

      #ifdef FPS_DEBUG
        debugPort.println(F("Setting password successful."));
        debugPort.print(F("New password = "));
        debugPort.println(devicePasswordL, HEX);
      #endif

      return FPS_RESP_OK; //password setting complete
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Setting password failed."));
        debugPort.print(F("Input Password = "));
        debugPort.println(inputPassword, HEX);
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//set a new 4 byte device address. if the operation is successful, the new
//address will be saved

uint8_t R30X_FPS::setAddress (uint32_t address) {
  uint8_t addressArray[4] = {0}; //just so that we do not need to alter the existing address before successfully changing it
  addressArray[0] = address & 0xFF;
  addressArray[1] = (address >> 8) & 0xFF;
  addressArray[2] = (address >> 16) & 0xFF;
  addressArray[3] = (address >> 24) & 0xFF;

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETDEVICEADDRESS, addressArray, 4); //send the command and data

  deviceAddressL = address; //save the new address (Long)
  deviceAddress[0] = addressArray[0]; //save the new address as array
  deviceAddress[1] = addressArray[1];
  deviceAddress[2] = addressArray[2];
  deviceAddress[3] = addressArray[3];

  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if((rxConfirmationCode == FPS_RESP_OK) || (rxConfirmationCode == 0x20U)) { //the confrim code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Setting address success."));
        debugPort.print(F("New address = "));
        debugPort.println(deviceAddressL, HEX);
      #endif

      return FPS_RESP_OK; //address setting complete
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Setting address failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//change the baudrate and reinitialize the port. the new baudrate will be
//saved after successful execution

uint8_t R30X_FPS::setBaudrate (uint32_t baud) {
  uint8_t baudNumber = baud / 9600; //check is the baudrate is a multiple of 9600
  uint8_t dataArray[2] = {0};

  if((baudNumber > 0) && (baudNumber < 13)) { //should be between 1 (9600bps) and 12 (115200bps)
    dataArray[0] = baudNumber;  //low byte
    dataArray[1] = 4; //the code for the system parameter number, 4 means baudrate

    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETSYSPARA, dataArray, 2); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == FPS_RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        deviceBaudrate = baud;
        
        if (hwSerial) { //if using hardware serial
          hwSerial->end();  //end the existing serial port
          hwSerial->begin(deviceBaudrate);  //restart the port with new baudrate
        }

        #if defined(__AVR__) || defined(ESP8266)
          if (swSerial) { //if using software serial
            swSerial->end();  //stop existing serial port
            swSerial->begin(deviceBaudrate);  //restart the port with new baudrate
          }
        #endif

        #ifdef FPS_DEBUG
          debugPort.println(F("Setting baudrate success."));
        #endif
        return FPS_RESP_OK; //baudrate setting complete
      }
      else {
        #ifdef FPS_DEBUG
          debugPort.println(F("Setting baudrate failed."));
          debugPort.print(F("rxConfirmationCode = "));
          debugPort.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FPS_DEBUG
      debugPort.println(F("Bad baudrate value."));
      debugPort.println(F("Setting baudrate failed."));
    #endif
    return FPS_BAD_VALUE;
  }
}

//=========================================================================//
//change the security level - or the threshold for matching two fingerprint
//templates

uint8_t R30X_FPS::setSecurityLevel (uint8_t level) {
  uint8_t dataArray[2] = {0};

  if((level > 0) && (level < 6)) { //should be between 1 and 5
    dataArray[0] = level;  //low byte
    dataArray[1] = 5; //the code for the system parameter number, 5 means the security level

    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETSYSPARA, dataArray, 2); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == FPS_RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        #ifdef FPS_DEBUG
          debugPort.println(F("Setting new security level success."));
          debugPort.print(F("Old value = "));
          debugPort.println(securityLevel, HEX);
          debugPort.print(F("New value = "));
          debugPort.println(level, HEX);
        #endif
        securityLevel = level;  //save new value
        return FPS_RESP_OK; //security level setting complete
      }
      else {
        #ifdef FPS_DEBUG
          debugPort.println(F("Setting security level failed."));
          debugPort.print(F("Current value = "));
          debugPort.println(securityLevel, HEX);
          debugPort.print(F("rxConfirmationCode = "));
          debugPort.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FPS_DEBUG
      debugPort.println(F("Bad security level value."));
      debugPort.println(F("Setting security level failed."));
    #endif
    return FPS_BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//set the max length of data bytes that can be received from the module

uint8_t R30X_FPS::setDataLength (uint16_t length) {
   #ifdef FPS_DEBUG
    debugPort.println(F("Setting new data length.."));
  #endif

  uint8_t dataArray[2] = {0};

  if((length == 32) || (length == 64) || (length == 128) || (length == 256)) { //should be 32, 64, 128 or 256 bytes
    if(length == 32)
      dataArray[0] = 0;  //low byte
    else if(length == 64)
      dataArray[0] = 1;  //low byte
    else if(length == 128)
      dataArray[0] = 2;  //low byte
    else if(length == 256)
      dataArray[0] = 3;  //low byte

    dataArray[1] = 6; //the code for the system parameter number
    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETSYSPARA, dataArray, 2); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == FPS_RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        dataPacketLength = length;  //save the new data length

        #ifdef FPS_DEBUG
          debugPort.println(F("Setting data length success."));
          debugPort.print(F("dataPacketLength = "));
          debugPort.println(dataPacketLength);
        #endif

        return FPS_RESP_OK; //length setting complete
      }
      else {
        #ifdef FPS_DEBUG
          debugPort.println(F("Setting data length failed."));
          debugPort.print(F("rxConfirmationCode = "));
          debugPort.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FPS_DEBUG
      debugPort.println(F("Bad data length value."));
      debugPort.println(F("Setting data length failed."));
    #endif
    return FPS_BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//turns on/off the communication port

uint8_t R30X_FPS::portControl (uint8_t value) {
  #ifdef FPS_DEBUG
    if(value == 1)
      debugPort.println(F("Turning port on.."));
    else
      debugPort.println(F("Turning port off.."));
  #endif

  uint8_t dataArray[1] = {0};

  if((value == 0) || (value == 1)) { //should be either 1 or 0
    dataArray[0] = value;
    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_PORTCONTROL, dataArray, 1); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == FPS_RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        #ifdef FPS_DEBUG
          if(value == 1)
            debugPort.println(F("Turning port on success."));
          else
            debugPort.println(F("Turning port off success."));
        #endif
        return FPS_RESP_OK; //port setting complete
      }
      else {
        #ifdef FPS_DEBUG
          debugPort.println(F("Turning port on/off failed."));
          debugPort.print(F("rxConfirmationCode = "));
          debugPort.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    return FPS_BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//read system configuration

uint8_t R30X_FPS::readSysPara() {
  #ifdef FPS_DEBUG
    debugPort.println(F("Reading system parameters.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_READSYSPARA); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      statusRegister = uint16_t(rxDataBuffer[15] << 8) + rxDataBuffer[14];  //high byte + low byte
      securityLevel = rxDataBuffer[8];

      if(rxDataBuffer[2] == 0)
        dataPacketLength = 32;
      else if(rxDataBuffer[2] == 1)
        dataPacketLength = 64;
      else if(rxDataBuffer[2] == 2)
        dataPacketLength = 128;
      else if(rxDataBuffer[2] == 3)
        dataPacketLength = 256;

      deviceBaudrate = rxDataBuffer[0] * 9600;  //baudrate is retrieved as a multiplier

      #ifdef FPS_DEBUG
        debugPort.println(F("Reading system parameters success."));
        debugPort.print(F("statusRegister = "));
        debugPort.println(statusRegister);
        debugPort.print(F("securityLevel = "));
        debugPort.println(securityLevel);
        debugPort.print(F("dataPacketLength = "));
        debugPort.println(dataPacketLength);
        debugPort.print(F("deviceBaudrate = "));
        debugPort.println(deviceBaudrate);
      #endif

      return FPS_RESP_OK;
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Reading system parameters failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//returns the total template count in the flash memory

uint8_t R30X_FPS::getTemplateCount() {
  #ifdef FPS_DEBUG
    debugPort.println(F("Reading template count.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_TEMPLATECOUNT); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      templateCount = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //high byte + low byte

      #ifdef FPS_DEBUG
        debugPort.println(F("Reading template count success."));
        debugPort.print(F("templateCount = "));
        debugPort.println(templateCount);
      #endif

      return FPS_RESP_OK;
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Reading template count failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//scans the fingerprint and finds a match within specified range
//timeout = 100-25500 milliseconds
//startId = #1-#1000
//range = 1-1000

uint8_t R30X_FPS::captureAndRangeSearch (uint16_t captureTimeout, uint16_t startLocation, uint16_t count) {
  if(captureTimeout > 25500) { //25500 is the max timeout the device supports
    #ifdef FPS_DEBUG
      debugPort.println(F("Capture and range search failed."));
      debugPort.println(F("Bad capture timeout."));
      debugPort.print(F("captureTimeout = "));
      debugPort.println(captureTimeout);
    #endif
    return FPS_BAD_VALUE;
  }

  if((startLocation > 1000) || (startLocation < 1)) { //if not in range (0-999)
    #ifdef FPS_DEBUG
      debugPort.println(F("Capture and range search failed."));
      debugPort.println(F("Bad start ID"));
      debugPort.print(F("startId = #"));
      debugPort.println(startLocation);
    #endif

    return FPS_BAD_VALUE;
  }

  if((startLocation + count) > 1001) { //if range overflows
    #ifdef FPS_DEBUG
      debugPort.println(F("Capture and range search failed."));
      debugPort.println(F("startLocation + count can't be greater than 1001."));
      debugPort.print(F("startLocation = #"));
      debugPort.println(startLocation);
      debugPort.print(F("count = "));
      debugPort.println(count);
      debugPort.print(F("startLocation + count = "));
      debugPort.println(startLocation + count);
    #endif
    return FPS_BAD_VALUE;
  }

  uint8_t dataArray[5] = {0}; //need 5 bytes here

  //generate the data array
  dataArray[4] = uint8_t(captureTimeout / 140);  //this byte is sent first
  dataArray[3] = ((startLocation-1) >> 8) & 0xFFU;  //high byte
  dataArray[2] = uint8_t((startLocation-1) & 0xFFU);  //low byte
  dataArray[1] = (count >> 8) & 0xFFU; //high byte
  dataArray[0] = uint8_t(count & 0xFFU); //low byte

  #ifdef FPS_DEBUG
    debugPort.println(F("Starting capture and range search."));
    debugPort.print(F("captureTimeout = "));
    debugPort.println(captureTimeout);
    debugPort.print(F("startLocation = #"));
    debugPort.println(startLocation);
    debugPort.print(F("count = "));
    debugPort.println(count);
    debugPort.print(F("startLocation + count = "));
    debugPort.println(startLocation + count);
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SCANANDRANGESEARCH, dataArray, 5); //send the command, there's no additional data
  uint8_t response = receivePacket(captureTimeout + 100); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //high byte + low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //data length will be 4 here

      #ifdef FPS_DEBUG
        debugPort.println(F("Capture and range search success."));
        debugPort.print(F("fingerId = #"));
        debugPort.println(fingerId);
        debugPort.print(F("matchScore = "));
        debugPort.println(matchScore);
      #endif

      return FPS_RESP_OK;
    }
    else {
      fingerId = 0;
      matchScore = 0;

      #ifdef FPS_DEBUG
        debugPort.println(F("Fingerprint not found."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif

      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//scans the fingerprint and finds a match within the full range of library
//a timeout can not be specified here

uint8_t R30X_FPS::captureAndFullSearch () {
  #ifdef FPS_DEBUG
    debugPort.println(F("Starting capture and full search."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SCANANDFULLSEARCH); //send the command, there's no additional data
  uint8_t response = receivePacket(3000); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //high byte + low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //data length will be 4 here

      #ifdef FPS_DEBUG
        debugPort.println(F("Capture and full search success."));
        debugPort.print(F("fingerId = #"));
        debugPort.println(fingerId);
        debugPort.print(F("matchScore = "));
        debugPort.println(matchScore);
      #endif

      return FPS_RESP_OK;
    }
    else {
      fingerId = 0;
      matchScore = 0;

      #ifdef FPS_DEBUG
        debugPort.println(F("Fingerprint not found."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif

      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//scan the fingerprint, generate an image and store it in the image buffer

uint8_t R30X_FPS::generateImage () {
  #ifdef FPS_DEBUG
    debugPort.println(F("Generating fingerprint image.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SCANFINGER); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Image saved to buffer successfully."));
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Generating fingerprint failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//export the image stored in one of the buffers to the computer
//this is not fully implemented. please do not use it

uint8_t R30X_FPS::exportImage () {
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_EXPORTIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//import an image file from computer to one of the buffers.
//this is not fully implemented. please do not use it

uint8_t R30X_FPS::importImage (uint8_t* dataBuffer) {
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_IMPORTIMAGE, dataBuffer, 64); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//generate a character file from image stored in image buffer and store it in
//one of the two character buffers

uint8_t R30X_FPS::generateCharacter (uint8_t bufferId) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FPS_DEBUG
      debugPort.println(F("Generating character file failed."));
      debugPort.println(F("Bad value. bufferId can only be 1 or 2."));
      debugPort.print(F("bufferId = "));
      debugPort.println(bufferId);
    #endif

    return FPS_BAD_VALUE;
  }
  uint8_t dataBuffer[1] = {bufferId}; //create data array

  #ifdef FPS_DEBUG
    debugPort.println(F("Generating character file.."));
    debugPort.print(F("Character bufferId = "));
    debugPort.println(bufferId);
  #endif
  
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_IMAGETOCHARACTER, dataBuffer, 1);
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Generating character file successful."));
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Generating character file failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);

        if(rxConfirmationCode == FPS_RESP_OVERDISORDERFAIL2) {
          debugPort.println(F("Character file overly disordered."));
        }

        if(rxConfirmationCode == FPS_RESP_FEATUREFAIL) {
          debugPort.println(F("Character file feature fail."));
        }

        if(rxConfirmationCode == FPS_RESP_IMAGEGENERATEFAIL) {
          debugPort.println(F("Valid image not available."));
        }

      #endif

      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//combine the two character files from buffers and generate a template
//the template will be saved to the both the buffers

uint8_t R30X_FPS::generateTemplate () {
  #ifdef FPS_DEBUG
    debugPort.println(F("Generating template from char buffers.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_GENERATETEMPLATE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Generating template success."));
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Generating template failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//export a character file from one of the buffers to the computer
//this is not completely implemented. please do not use it

uint8_t R30X_FPS::exportCharacter (uint8_t bufferId) {
  uint8_t dataBuffer[1] = {bufferId}; //create data array
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_IMPORTIMAGE);
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//import a character file from computer to one of the buffers
//this is not completely implemented. please do not use it

uint8_t R30X_FPS::importCharacter (uint8_t bufferId, uint8_t* dataBuffer) {
  uint8_t dataArray[sizeof(dataBuffer)+1] = {0}; //create data array
  dataArray[sizeof(dataBuffer)];
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_IMPORTIMAGE);
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//store the contents of one of the two template (character) buffers to a
//location on the fingerprint library

uint8_t R30X_FPS::saveTemplate (uint8_t bufferId, uint16_t location) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FPS_DEBUG
      debugPort.println(F("Storing template failed."));
      debugPort.println(F("Bad value. bufferId can only be 1 or 2."));
      debugPort.print(F("bufferId = "));
      debugPort.println(bufferId);
    #endif

    return FPS_BAD_VALUE;
  }

  if((location > 1000) || (location < 1)) { //if the value is not in range
    #ifdef FPS_DEBUG
      debugPort.println(F("Generating template failed."));
      debugPort.println(F("Bad value. location must be #1 to #1000."));
      debugPort.print(F("location = "));
      debugPort.println(location);
    #endif

    return FPS_BAD_VALUE;
  }

  #ifdef FPS_DEBUG
    debugPort.println(F("Saving template.."));
  #endif

  uint8_t dataArray[3] = {0}; //create data array
  dataArray[2] = bufferId;  //highest byte
  dataArray[1] = ((location-1) >> 8) & 0xFFU; //high byte of location
  dataArray[0] = ((location-1) & 0xFFU); //low byte of location

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_STORETEMPLATE, dataArray, 3); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Storing template successful."));
        debugPort.print(F("Saved to #"));
        debugPort.println(location);
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Storing template failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//load the contents from a location on the library to one of the two
//template/character buffers

uint8_t R30X_FPS::loadTemplate (uint8_t bufferId, uint16_t location) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FPS_DEBUG
      debugPort.println(F("Loading template failed."));
      debugPort.println(F("Bad value. bufferId can only be 1 or 2."));
      debugPort.print(F("bufferId = "));
      debugPort.println(bufferId);
    #endif

    return FPS_BAD_VALUE;
  }

  if((location > 1000) || (location < 1)) { //if the value is not in range
    #ifdef FPS_DEBUG
      debugPort.println(F("Loading template failed."));
      debugPort.println(F("Bad value. location must be #1 to #1000."));
      debugPort.print(F("location = "));
      debugPort.println(location);
    #endif

    return FPS_BAD_VALUE;
  }

  uint8_t dataArray[3] = {0}; //create data array
  dataArray[2] = bufferId;  //highest byte
  dataArray[1] = ((location-1) >> 8) & 0xFFU; //high byte of location
  dataArray[0] = ((location-1) & 0xFFU); //low byte of location

  #ifdef FPS_DEBUG
    debugPort.println(F("Loading template.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_LOADTEMPLATE, dataArray, 3); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Loading template successful."));
        debugPort.print(F("Loaded #"));
        debugPort.print(location);
        debugPort.print(F(" to buffer "));
        debugPort.println(bufferId);
      #endif

      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Loading template failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//delete a set of templates saved in the flash memory

uint8_t R30X_FPS::deleteTemplate (uint16_t startLocation, uint16_t count) {
  if((startLocation > 1000) || (startLocation < 1)) { //if the value is not 1 or 2
    #ifdef FPS_DEBUG
      debugPort.println(F("Deleting template failed."));
      debugPort.println(F("Bad value. Start location must be #1 to #1000."));
      debugPort.print(F("startLocation = "));
      debugPort.println(startLocation);
    #endif

    return FPS_BAD_VALUE;
  }

  if((count + startLocation) > 1001) { //if the value is not in range
    #ifdef FPS_DEBUG
      debugPort.println(F("Deleting template failed."));
      debugPort.println(F("Bad value. Sum of startLocation and count can't be greater than 1001."));
      debugPort.print(F("startLocation + count = "));
      debugPort.println(startLocation + count);
    #endif

    return FPS_BAD_VALUE;
  }

  uint8_t dataArray[4] = {0}; //create data array
  dataArray[3] = ((startLocation-1) >> 8) & 0xFFU; //high byte of location
  dataArray[2] = ((startLocation-1) & 0xFFU); //low byte of location
  dataArray[1] = (count >> 8) & 0xFFU; //high byte of total no. of templates to delete
  dataArray[0] = (count & 0xFFU); //low byte of count

  #ifdef FPS_DEBUG
    debugPort.println(F("Deleting template.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_DELETETEMPLATE, dataArray, 4); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
     #ifdef FPS_DEBUG
        debugPort.println(F("Deleting template successful."));
        debugPort.print(F("From #"));
        debugPort.print(startLocation);
        debugPort.print(F(" to #"));
        debugPort.println(startLocation + count - 1);
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Deleting template failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//deletes all the templates stored in the library

uint8_t R30X_FPS::clearLibrary () {
  #ifdef FPS_DEBUG
    debugPort.println(F("Clearing library.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_CLEARLIBRARY); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Clearing library success."));
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("Clearing library failed."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//match the templates stored in the buffers and calculate a match score

uint8_t R30X_FPS::matchTemplates () {
  #ifdef FPS_DEBUG
    debugPort.println(F("Matching templates.."));
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_MATCHTEMPLATES); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FPS_DEBUG
        debugPort.println(F("Matching templates success."));
      #endif

      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FPS_DEBUG
        debugPort.println(F("The templates do no match."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//searches the contents of one of the two char buffers for a match on the
//fingerprint library throughout a range

uint8_t R30X_FPS::searchLibrary (uint8_t bufferId, uint16_t startLocation, uint16_t count) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FPS_DEBUG
      debugPort.println(F("Searching library failed."));
      debugPort.println(F("Bad value. bufferId can only be 1 or 2."));
      debugPort.print(F("bufferId = "));
      debugPort.println(bufferId);
    #endif
    return FPS_BAD_VALUE;
  }

  if((startLocation > 1000) || (startLocation < 1)) { //if not in range (0-999)
    #ifdef FPS_DEBUG
      debugPort.println(F("Searching library failed."));
      debugPort.println(F("Bad start ID"));
      debugPort.print(F("startId = #"));
      debugPort.println(startLocation);
    #endif
    return FPS_BAD_VALUE;
  }

  if((startLocation + count) > 1001) { //if range overflows
    #ifdef FPS_DEBUG
      debugPort.println(F("Searching library failed."));
      debugPort.println(F("startLocation + count can't be greater than 1001."));
      debugPort.print(F("startLocation = #"));
      debugPort.println(startLocation);
      debugPort.print(F("count = "));
      debugPort.println(count);
      debugPort.print(F("startLocation + count = "));
      debugPort.println(startLocation + count);
    #endif
    return FPS_BAD_VALUE;
  }

  uint8_t dataArray[5] = {0};
  dataArray[4] = bufferId;
  dataArray[3] = (startLocation >> 8) & 0xFFU;  //high byte
  dataArray[2] = (startLocation & 0xFFU); //low byte
  dataArray[1] = (count >> 8) & 0xFFU; //high byte
  dataArray[0] = (count & 0xFFU); //low byte

  #ifdef FPS_DEBUG
    debugPort.println(F("Starting searching library for buffer content."));
    debugPort.print(F("bufferId = "));
    debugPort.println(bufferId);
    debugPort.print(F("startLocation = #"));
    debugPort.println(startLocation);
    debugPort.print(F("count = "));
    debugPort.println(count);
    debugPort.print(F("startLocation + count = "));
    debugPort.println(startLocation + count);
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SEARCHLIBRARY, dataArray, 5); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == FPS_RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //add high byte and low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //add high byte and low byte
      
      #ifdef FPS_DEBUG
        debugPort.println(F("Buffer content found in library."));
        debugPort.print(F("fingerId = #"));
        debugPort.println(fingerId);
        debugPort.print(F("matchScore = "));
        debugPort.println(matchScore);
      #endif

      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      //fingerId = 0 doesn't mean the match was found at location 0
      //instead it means an error. check the confirmation code to determine the problem
      fingerId = 0;
      matchScore = 0;

      #ifdef FPS_DEBUG
        debugPort.println(F("Fingerprint not found."));
        debugPort.print(F("rxConfirmationCode = "));
        debugPort.println(rxConfirmationCode, HEX);
      #endif
      
      return rxConfirmationCode;
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//

//written by human, for humans.
