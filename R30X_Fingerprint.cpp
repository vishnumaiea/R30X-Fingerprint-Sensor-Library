
//=========================================================================//
//                                                                         //
//  ## R30X Fingerprint Sensor Library ##                                  //
//                                                                         //
//  Filename : R30X_Fingerprint.cpp                                        //
//  Description : CPP file for R30X_Fingerprint library for R30X series    //
//                fingerprint sensors.                                     //
//  Library version : 1.0                                                  //
//  Author : Vishnu M Aiea                                                 //
//  Src : https://github.com/vishnumaiea/R30X-Fingerprint-Sensor-Library   //
//  Author's website : https://www.vishnumaiea.in                          //
//  Initial release : IST 07:35 PM, 08-04-2019, Monday                     //
//  License : MIT                                                          //
//                                                                         //
//  Last modified : IST 11:44 PM, 08-04-2019, Monday                       //
//                                                                         //
//=========================================================================//

#include "R30X_Fingerprint.h"

//=========================================================================//
//constructor for SoftwareSerial interface

#if defined(__AVR__) || defined(ESP8266)
  R30X_Fingerprint::R30X_Fingerprint (SoftwareSerial *ss, uint32_t password, uint32_t address) {
    devicePassword = password; //these can be altered later
    deviceAddress = address;

    hwSerial = NULL;
    swSerial = ss;
    mySerial = swSerial;

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

    resetParameters(); //initialize and reset and all parameters
  }
#endif

//=========================================================================//
//constructor for hardware serial interface

R30X_Fingerprint::R30X_Fingerprint (HardwareSerial *hs, uint32_t password, uint32_t address) {
  #if defined(__AVR__) || defined(ESP8266)
    swSerial = NULL;
  #endif
  hwSerial = hs;
  mySerial = hwSerial;

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

void R30X_Fingerprint::begin (uint32_t baudrate) {
  delay(1000);  //one second delay to let the sensor 'boot up'

  deviceBaudrate = baudrate;  //save the new baudrate

  if (hwSerial) hwSerial->begin(baudrate);

  #if defined(__AVR__) || defined(ESP8266)
    if (swSerial) swSerial->begin(baudrate);
  #endif
}

//=========================================================================//
//reset tx and rx parameters

void R30X_Fingerprint::resetParameters (void) {
  deviceBaudrate = DEFAULT_BAUDRATE;  //UART speed
  securityLevel = DEFAULT_SECURITY_LEVEL;  //threshold level for fingerprint matching
  dataPacketLength = DEFAULT_RX_DATA_LENGTH;

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
//send a data packet to the FPS

uint8_t R30X_Fingerprint::sendPacket (uint8_t type, uint8_t command, uint8_t* data, uint16_t dataLength) {
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

  #ifdef FINGERPRINT_DEBUG
  Serial.print("Sent packet = ");
    Serial.print(startCode[1], HEX); //high byte is sent first
    Serial.print("-");
    Serial.print(startCode[0], HEX);
    Serial.print("-");
    Serial.print(deviceAddress[3], HEX); //high byte is sent first
    Serial.print("-");
    Serial.print(deviceAddress[2], HEX);
    Serial.print("-");
    Serial.print(deviceAddress[1], HEX);
    Serial.print("-");
    Serial.print(deviceAddress[0], HEX);
    Serial.print("-");
    Serial.print(txPacketType, HEX);
    Serial.print("-");
    Serial.print(txPacketLength[1], HEX); //high byte is sent first
    Serial.print("-");
    Serial.print(txPacketLength[0], HEX);
    Serial.print("-");
    Serial.print(txInstructionCode, HEX);
    Serial.print("-");

    for(int i=(txDataBufferLength-1); i>=0; i--) {
      Serial.print(txDataBuffer[i], HEX); //send high byte first
      Serial.print("-");
    }

    Serial.print(txPacketChecksum[1], HEX);
    Serial.print("-");
    Serial.print(txPacketChecksum[0], HEX);
    Serial.println();
    Serial.print("txInstructionCode = ");
    Serial.println(txInstructionCode, HEX);
    Serial.print("txDataBufferLength = ");
    Serial.println(txDataBufferLength, HEX);
    Serial.print("txPacketLengthL = ");
    Serial.println(txPacketLengthL);
    // Serial.print("rxPacketLength[] = ");
    // Serial.print(rxPacketLength[1], HEX);
    // Serial.print("-");
    // Serial.println(rxPacketLength[0], HEX);
  #endif

  return RX_OK;
}

//=========================================================================//
//send a data packet to the FPS

uint8_t R30X_Fingerprint::receivePacket (uint32_t timeout) {
  uint8_t* dataBuffer;
  if(dataPacketLength < 64) { //data buffer length should be at least 64 bytes
    dataBuffer = new uint8_t[64](); //conatains only the data
  }
  else {
    dataBuffer = new uint8_t[DEFAULT_RX_DATA_LENGTH]();
  }

  rxDataBuffer = dataBuffer;
  uint8_t serialBuffer[DEFAULT_SERIAL_BUFFER_LENGTH] = {0}; //serialBuffer will store high byte at the start of the array
  uint16_t serialBufferLength = 0;
  uint8_t byteBuffer = 0;

  #ifdef FINGERPRINT_DEBUG
    Serial.println();
    Serial.println("Reading response.");
  #endif

  while (timeout > 0) {
    if(Serial1.available()) {
      byteBuffer = Serial1.read();
      #ifdef FINGERPRINT_DEBUG
        // Serial.print("Response byte found = ");
        // Serial.println(byteBuffer, HEX);
      #endif
      serialBuffer[serialBufferLength] = byteBuffer;
      serialBufferLength++;
    }

    timeout--;
    delay(1);
  }

  if(serialBufferLength == 0) {
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Serial timed out.");
      Serial.println("This usually means the baud rate is not correct.");
    #endif
    return RX_TIMEOUT;
  }

  if(serialBufferLength < 10) {
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Received bad packet with length < 10");
    #endif
    return RX_BADPACKET;
  }

  uint16_t token = 0;

  while(true) {
    switch (token) {
      case 0: //test packet start codes
        if(serialBuffer[token] == startCode[1])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG //enable it to get debug info
            Serial.println("Error at 0 : Start Code");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }

      case 1:
        if(serialBuffer[token] == startCode[0])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 1 : Start Code");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }

      case 2: //test device address
        if(serialBuffer[token] == deviceAddress[3])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 2 : Device Address");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }
      
      case 3:
        if(serialBuffer[token] == deviceAddress[2])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 3 : Device Address");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }

      case 4:
        if(serialBuffer[token] == deviceAddress[1])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 4 : Device Address");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }
      
      case 5:
        if(serialBuffer[token] == deviceAddress[0])
          break;
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 5 : Device Address");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;
        }

      case 6: //test for valid packet type
        if((serialBuffer[token] == FPS_ID_COMMANDPACKET) || (serialBuffer[token] == FPS_ID_DATAPACKET) || (serialBuffer[token] == FPS_ID_ACKPACKET) || (serialBuffer[token] == FPS_ID_ENDDATAPACKET)) {
          rxPacketType = serialBuffer[token]; //store the packet ID to class variable
          break;
        }
        else {
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 6 : Unknown Response");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_WRONGRESPONSE;
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
         #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 7 : Unknown Response");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_WRONGRESPONSE;
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
            #ifdef FINGERPRINT_DEBUG
              Serial.println("Checksums match success.");
              Serial.print("Received = ");
              Serial.print(rxPacketChecksum[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketChecksum[0], HEX);
              Serial.print("Received L = " );
              Serial.println(rxPacketChecksumL, HEX);
              Serial.print("Calculated = ");
              Serial.print(byte(tempSum >> 8), HEX);
              Serial.print("-");
              Serial.println(byte(tempSum & 0xFFU), HEX);
              Serial.print("Calculated L = ");
              Serial.println(tempSum, HEX);
              Serial.print("Received packet = ");

              for(int i=0; i < serialBufferLength; i++) {
                Serial.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  Serial.print("-");
                }
              }
              Serial.println();
              Serial.print("Data stream = none");

              Serial.println();
              Serial.print("rxConfirmationCode = ");
              Serial.println(rxConfirmationCode, HEX);
              Serial.print("rxDataBufferLength = ");
              Serial.println(rxDataBufferLength, HEX);
              Serial.print("rxPacketLengthL = ");
              Serial.println(rxPacketLengthL);
              Serial.print("rxPacketLength[] = ");
              Serial.print(rxPacketLength[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketLength[0], HEX);
            #endif

            return RX_OK; //packet read success
          }

          else { //if the checksums do not match
            #ifdef FINGERPRINT_DEBUG
              Serial.println("Checksums match fail.");
              Serial.print("Received = ");
              Serial.print(rxPacketChecksum[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketChecksum[0], HEX);
              Serial.print("Received L = " );
              Serial.println(rxPacketChecksumL, HEX);
              Serial.print("Calculated = ");
              Serial.print(byte(tempSum >> 8), HEX);
              Serial.print("-");
              Serial.println(byte(tempSum & 0xFFU), HEX);
              Serial.print("Calculated L = ");
              Serial.println(tempSum, HEX);
              Serial.print("Received packet = ");

              for(int i=0; i < serialBufferLength; i++) {
                Serial.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  Serial.print("-");
                }
              }
              Serial.println();
              Serial.print("Data stream = none");
              
              Serial.println();
              Serial.print("rxConfirmationCode = ");
              Serial.println(rxConfirmationCode, HEX);
              Serial.print("rxDataBufferLength = ");
              Serial.println(rxDataBufferLength, HEX);
              Serial.print("rxPacketLengthL = ");
              Serial.println(rxPacketLengthL, HEX);
              Serial.print("rxPacketLength[] = ");
              Serial.print(rxPacketLength[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketLength[0], HEX);
            #endif

            return RX_BADPACKET;  //then that's an error
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
            #ifdef FINGERPRINT_DEBUG
              Serial.println("Checksums match success.");
              Serial.print("Received = ");
              Serial.print(rxPacketChecksum[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketChecksum[0], HEX);
              Serial.print("Received L = " );
              Serial.println(rxPacketChecksumL, HEX);
              Serial.print("Calculated = ");
              Serial.print(byte(tempSum >> 8), HEX);
              Serial.print("-");
              Serial.println(byte(tempSum & 0xFFU), HEX);
              Serial.print("Calculated L = ");
              Serial.println(tempSum, HEX);
              Serial.print("Received packet = ");

              for(int i=0; i < serialBufferLength; i++) {
                Serial.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  Serial.print("-");
                }
              }
              Serial.println();
              Serial.print("Data stream = ");

              for(int i=0; i < rxDataBufferLength; i++) {
                Serial.print(rxDataBuffer[(rxDataBufferLength-1) - i], HEX);
                if(i != (rxDataBufferLength - 1)) {
                  Serial.print("-");
                }
              }

              Serial.println();
              Serial.print("rxConfirmationCode = ");
              Serial.println(rxConfirmationCode, HEX);
              Serial.print("rxDataBufferLength = ");
              Serial.println(rxDataBufferLength, HEX);
              Serial.print("rxPacketLengthL = ");
              Serial.println(rxPacketLengthL, HEX);
              Serial.print("rxPacketLength[] = ");
              Serial.print(rxPacketLength[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketLength[0], HEX);
            #endif

            return RX_OK; //packet read success
          }

          else { //if the checksums do not match
            #ifdef FINGERPRINT_DEBUG
              Serial.println("Checksums match fail.");
              Serial.print("Received = ");
              Serial.print(rxPacketChecksum[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketChecksum[0], HEX);
              Serial.print("Received L = " );
              Serial.println(rxPacketChecksumL, HEX);
              Serial.print("Calculated = ");
              Serial.print(byte(tempSum >> 8), HEX);
              Serial.print("-");
              Serial.println(byte(tempSum & 0xFFU), HEX);
              Serial.print("Calculated L = ");
              Serial.println(tempSum, HEX);
              Serial.print("Received packet = ");

              for(int i=0; i < serialBufferLength; i++) {
                Serial.print(serialBuffer[i], HEX);
                if(i != (serialBufferLength - 1)) {
                  Serial.print("-");
                }
              }
              Serial.println();
              Serial.print("Data stream = ");

              for(int i=0; i < rxDataBufferLength; i++) {
                Serial.print(rxDataBuffer[(rxDataBufferLength-1) - i], HEX);
                if(i != (rxDataBufferLength - 1)) {
                  Serial.print("-");
                }
              }
              
              Serial.println();
              Serial.print("rxConfirmationCode = ");
              Serial.println(rxConfirmationCode, HEX);
              Serial.print("rxDataBufferLength = ");
              Serial.println(rxDataBufferLength, HEX);
              Serial.print("rxPacketLengthL = ");
              Serial.println(rxPacketLengthL, HEX);
              Serial.print("rxPacketLength[] = ");
              Serial.print(rxPacketLength[1], HEX);
              Serial.print("-");
              Serial.println(rxPacketLength[0], HEX);
            #endif

            return RX_BADPACKET;  //then that's an error
          }
          break;
        }

        //-------------------------------------------------------------------------//

        else { //if the checksum received is 0
          #ifdef FINGERPRINT_DEBUG
            Serial.println("Error at 12 : Checksum");
            Serial.print("Received packet = ");
            for(int i=0; i < serialBufferLength; i++) {
              Serial.print(serialBuffer[i], HEX);
              if(i != (serialBufferLength - 1)) {
                Serial.print("-");
              }
            }
            Serial.println();
          #endif

          return RX_BADPACKET;  //that too an error
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

uint8_t R30X_Fingerprint::verifyPassword (uint32_t password) {
  uint8_t passwordArray[4] = {0};
  passwordArray[0] = password & 0xFFU;
  passwordArray[1] = (password >> 8) & 0xFFU;
  passwordArray[2] = (password >> 16) & 0xFFU;
  passwordArray[3] = (password >> 24) & 0xFFU;

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_VERIFYPASSWORD, passwordArray, 4); //send the command and data
  uint8_t response = receivePacket(); //read response
  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) {
      devicePasswordL = password;
      devicePassword[0] = passwordArray[0]; //save the new password as array
      devicePassword[1] = passwordArray[1];
      devicePassword[2] = passwordArray[2];
      devicePassword[3] = passwordArray[3];

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Password is correct.");
        Serial.print("Current Password = ");
        Serial.println(devicePasswordL, HEX);
      #endif

      return FPS_RESP_OK; //password is correct
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Password is not correct.");
        Serial.print("Current Password = ");
        Serial.println(devicePasswordL, HEX);
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::setPassword (uint32_t password) {
  uint8_t passwordArray[4] = {0};
  passwordArray[0] = password & 0xFFU;
  passwordArray[1] = (password >> 8) & 0xFFU;
  passwordArray[2] = (password >> 16) & 0xFFU;
  passwordArray[3] = (password >> 24) & 0xFFU;

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETPASSWORD, passwordArray, 4); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confrim code will be saved when the response is received
      devicePasswordL = password; //save the new password (Long)
      devicePassword[0] = passwordArray[0]; //save the new password as array
      devicePassword[1] = passwordArray[1];
      devicePassword[2] = passwordArray[2];
      devicePassword[3] = passwordArray[3];

      #ifdef FINGERPRINT_DEBUG
        Serial.print("New password = ");
        Serial.println(devicePasswordL, HEX);
      #endif

      return FPS_RESP_OK; //password setting complete
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Setting password failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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
//address will be saved to the object.

uint8_t R30X_Fingerprint::setAddress (uint32_t address) {
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

  if(response == RX_OK) { //if the response packet is valid
    if((rxConfirmationCode == FPS_RESP_OK) || (rxConfirmationCode == 0x20U)) { //the confrim code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Setting address success.");
        Serial.print("New address = ");
        Serial.println(deviceAddressL, HEX);
      #endif

      return FPS_RESP_OK; //address setting complete
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Setting address failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::setBaudrate (uint32_t baud) {
  uint8_t baudNumber = baud / 9600; //check is the baudrate is a multiple of 9600
  uint8_t dataArray[2] = {0};

  if((baudNumber > 0) && (baudNumber < 13)) { //should be between 1 (9600bps) and 12 (115200bps)
    dataArray[0] = baudNumber;  //low byte
    dataArray[1] = 4; //the code for the system parameter number, 4 means baudrate

    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETSYSPARA, dataArray, 2); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == RX_OK) { //if the response packet is valid
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

        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting baudrate success.");
        #endif
        return FPS_RESP_OK; //baudrate setting complete
      }
      else {
        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting baudrate failed.");
          Serial.print("rxConfirmationCode = ");
          Serial.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Bad baudrate value.");
      Serial.println("Setting baudrate failed.");
    #endif
    return BAD_VALUE;
  }
}

//=========================================================================//
//change the baudrate and reinitialize the port. the new baudrate will be
//saved after successful execution

uint8_t R30X_Fingerprint::setSecurityLevel (uint8_t level) {
  uint8_t dataArray[2] = {0};

  if((level > 0) && (level < 6)) { //should be between 1 and 5
    dataArray[0] = level;  //low byte
    dataArray[1] = 5; //the code for the system parameter number, 5 means security level

    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SETSYSPARA, dataArray, 2); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting new security level success.");
          Serial.print("Old value = ");
          Serial.println(securityLevel, HEX);
          Serial.print("New value = ");
          Serial.println(level, HEX);
        #endif
        securityLevel = level;  //save new value
        return FPS_RESP_OK; //security level setting complete
      }
      else {
        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting security level failed.");
          Serial.print("Current value = ");
          Serial.println(securityLevel, HEX);
          Serial.print("rxConfirmationCode = ");
          Serial.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Bad security level value.");
      Serial.println("Setting security level failed.");
    #endif
    return BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//set the max length of data bytes that can be received from the module

uint8_t R30X_Fingerprint::setDataLength (uint16_t length) {
   #ifdef FINGERPRINT_DEBUG
    Serial.println("Setting new data length..");
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

    if(response == RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        dataPacketLength = length;  //save the new data length

        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting data length success.");
          Serial.print("dataPacketLength = ");
          Serial.println(dataPacketLength);
        #endif

        return FPS_RESP_OK; //length setting complete
      }
      else {
        #ifdef FINGERPRINT_DEBUG
          Serial.println("Setting data length failed.");
          Serial.print("rxConfirmationCode = ");
          Serial.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Bad data length value.");
      Serial.println("Setting data length failed.");
    #endif
    return BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//turns on or off the communication port

uint8_t R30X_Fingerprint::portControl (uint8_t value) {
  #ifdef FINGERPRINT_DEBUG
    if(value == 1)
      Serial.println("Turing port on..");
    else
      Serial.println("Turing port off..");
  #endif

  uint8_t dataArray[1] = {0};

  if((value == 0) || (value == 1)) { //should be either 1 or 0
    dataArray[0] = value;
    sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_PORTCONTROL, dataArray, 1); //send the command and data
    uint8_t response = receivePacket(); //read response

    if(response == RX_OK) { //if the response packet is valid
      if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
        #ifdef FINGERPRINT_DEBUG
          if(value == 1)
            Serial.println("Turing port on success.");
          else
            Serial.println("Turing port off success.");
        #endif
        return FPS_RESP_OK; //port setting complete
      }
      else {
        #ifdef FINGERPRINT_DEBUG
          Serial.println("Turning port on/off failed.");
          Serial.print("rxConfirmationCode = ");
          Serial.println(rxConfirmationCode, HEX);
        #endif
        return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
      }
    }
    else {
      return response; //return packet receive error code
    }
  }
  else {
    return BAD_VALUE; //the received parameter is invalid
  }
}

//=========================================================================//
//read system configuration

uint8_t R30X_Fingerprint::readSysPara() {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Reading system parameters..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_READSYSPARA); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
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

      deviceBaudrate = rxDataBuffer[0] * 9600;  //baudrate is retrieved as a number

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Reading system parameters success.");
        Serial.print("statusRegister = ");
        Serial.println(statusRegister);
        Serial.print("securityLevel = ");
        Serial.println(securityLevel);
        Serial.print("dataPacketLength = ");
        Serial.println(dataPacketLength);
        Serial.print("deviceBaudrate = ");
        Serial.println(deviceBaudrate);
      #endif

      return FPS_RESP_OK;
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Reading system parameters failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//returns the total template count

uint8_t R30X_Fingerprint::getTemplateCount() {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Reading template count..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_TEMPLATECOUNT); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      templateCount = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //high byte + low byte

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Reading template count success.");
        Serial.print("templateCount = ");
        Serial.println(templateCount);
      #endif

      return FPS_RESP_OK;
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Reading template count failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::captureAndRangeSearch (uint16_t captureTimeout, uint16_t startLocation, uint16_t count) {
  if(captureTimeout > 25500) { //if range overflows
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Capture and range search failed.");
      Serial.println("Bad capture timeout.");
      Serial.print("captureTimeout = ");
      Serial.println(captureTimeout);
    #endif
    return BAD_VALUE;
  }

  if((startLocation > 1000) || (startLocation < 1)) { //if not in range (0-999)
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Capture and range search failed.");
      Serial.println("Bad start ID");
      Serial.print("startId = #");
      Serial.println(startLocation);
    #endif

    return BAD_VALUE;
  }

  if((startLocation + count) > 1001) { //if range overflows
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Capture and range search failed.");
      Serial.println("startLocation + count can't be greater than 1001.");
      Serial.print("startLocation = #");
      Serial.println(startLocation);
      Serial.print("count = ");
      Serial.println(count);
      Serial.print("startLocation + count = ");
      Serial.println(startLocation + count);
    #endif
    return BAD_VALUE;
  }

  uint8_t dataArray[5] = {0}; //need 5 bytes here

  //generate the data array
  dataArray[4] = uint8_t(captureTimeout / 140);  //this byte is sent first
  dataArray[3] = ((startLocation-1) >> 8) & 0xFFU;  //high byte
  dataArray[2] = uint8_t((startLocation-1) & 0xFFU);  //low byte
  dataArray[1] = (count >> 8) & 0xFFU; //high byte
  dataArray[0] = uint8_t(count & 0xFFU); //low byte

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Starting capture and range search.");
    Serial.print("captureTimeout = ");
    Serial.println(captureTimeout);
    Serial.print("startLocation = #");
    Serial.println(startLocation);
    Serial.print("count = ");
    Serial.println(count);
    Serial.print("startLocation + count = ");
    Serial.println(startLocation + count);
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_GETANDRANGESEARCH, dataArray, 5); //send the command, there's no additional data
  uint8_t response = receivePacket(captureTimeout + 100); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //high byte + low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //data length will be 4 here

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Capture and range search success.");
        Serial.print("fingerId = #");
        Serial.println(fingerId);
        Serial.print("matchScore = ");
        Serial.println(matchScore);
      #endif

      return FPS_RESP_OK;
    }
    else {
      fingerId = 0;
      matchScore = 0;

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Fingerprint not found.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::captureAndFullSearch () {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Starting capture and full search.");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_GETANDFULLSEARCH); //send the command, there's no additional data
  uint8_t response = receivePacket(3000); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //high byte + low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //data length will be 4 here

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Capture and full search success.");
        Serial.print("fingerId = #");
        Serial.println(fingerId);
        Serial.print("matchScore = ");
        Serial.println(matchScore);
      #endif

      return FPS_RESP_OK;
    }
    else {
      fingerId = 0;
      matchScore = 0;

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Fingerprint not found.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
      #endif

      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//scan the fingerprint, generate an image and store it on the buffer

uint8_t R30X_Fingerprint::generateImage () {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Generating fingerprint image..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_GENIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Image saved to buffer successfully.");
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Generating fingerprint failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//get the image stored in the image buffer
//this is not completely implemented

uint8_t R30X_Fingerprint::downloadImage () {
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_DOWNLOADIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
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
//get the image stored in the image buffer
//this is not completely implemented

uint8_t R30X_Fingerprint::uploadImage (uint8_t* dataBuffer) {
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_UPLOADIMAGE, dataBuffer, 64); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
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
//generate character file from image stored in image buffer and store it on
//one of the two character buffers

uint8_t R30X_Fingerprint::generateCharacter (uint8_t bufferId) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Generating character file failed.");
      Serial.println("Bad value. bufferId can only be 1 or 2.");
      Serial.print("bufferId = ");
      Serial.println(bufferId);
    #endif

    return BAD_VALUE;
  }
  uint8_t dataBuffer[1] = {bufferId}; //create data array

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Generating character file..");
    Serial.print("Character bufferId = ");
    Serial.println(bufferId);
  #endif
  
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_UPLOADIMAGE, dataBuffer, 1); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Generating character file successful.");
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Generating character file failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);

        if(rxConfirmationCode == FPS_RESP_OVERDISORDERFAIL2) {
          Serial.println("Character file overly disordered.");
        }

        if(rxConfirmationCode == FPS_RESP_FEATUREFAIL) {
          Serial.println("Character file feature fail.");
        }

        if(rxConfirmationCode == FPS_RESP_IMAGEGENERATEFAIL) {
          Serial.println("Valid image not available.");
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
//combine the two character files and generate a template

uint8_t R30X_Fingerprint::generateTemplate () {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Generating template from char buffers..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_UPLOADIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Generating template success.");
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Generating template failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//combine the two character files and generate a template
//this is not completely implemented

uint8_t R30X_Fingerprint::downloadCharacter (uint8_t bufferId) {
  uint8_t dataBuffer[1] = {bufferId}; //create data array
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_UPLOADIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
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
//combine the two character files and generate a template
//this is not completely implemented

uint8_t R30X_Fingerprint::uploadCharacter (uint8_t bufferId, uint8_t* dataBuffer) {
  uint8_t dataArray[sizeof(dataBuffer)+1] = {0}; //create data array
  dataArray[sizeof(dataBuffer)];
  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_UPLOADIMAGE); //send the command, there's no additional data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
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

uint8_t R30X_Fingerprint::saveTemplate (uint8_t bufferId, uint16_t location) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Storing template failed.");
      Serial.println("Bad value. bufferId can only be 1 or 2.");
      Serial.print("bufferId = ");
      Serial.println(bufferId);
    #endif

    return BAD_VALUE;
  }

  if((location > 1000) || (location < 1)) { //if the value is not in range
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Generating template failed.");
      Serial.println("Bad value. location must be #1 to #1000.");
      Serial.print("location = ");
      Serial.println(location);
    #endif

    return BAD_VALUE;
  }

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Saving template..");
  #endif

  uint8_t dataArray[3] = {0}; //create data array
  dataArray[2] = bufferId;  //highest byte
  dataArray[1] = ((location-1) >> 8) & 0xFFU; //high byte of location
  dataArray[0] = ((location-1) & 0xFFU); //low byte of location

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_STORETEMPLATE, dataArray, 3); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Storing template successful.");
        Serial.print("Saved to #");
        Serial.println(location);
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Storing template failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::loadTemplate (uint8_t bufferId, uint16_t location) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Loading template failed.");
      Serial.println("Bad value. bufferId can only be 1 or 2.");
      Serial.print("bufferId = ");
      Serial.println(bufferId);
    #endif

    return BAD_VALUE;
  }

  if((location > 1000) || (location < 1)) { //if the value is not in range
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Loading template failed.");
      Serial.println("Bad value. location must be #1 to #1000.");
      Serial.print("location = ");
      Serial.println(location);
    #endif

    return BAD_VALUE;
  }

  uint8_t dataArray[3] = {0}; //create data array
  dataArray[2] = bufferId;  //highest byte
  dataArray[1] = ((location-1) >> 8) & 0xFFU; //high byte of location
  dataArray[0] = ((location-1) & 0xFFU); //low byte of location

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Loading template..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_LOADTEMPLATE, dataArray, 3); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Loading template successful.");
        Serial.print("Loaded #");
        Serial.print(location);
        Serial.print(" to buffer ");
        Serial.println(bufferId);
      #endif

      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Loading template failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
      #endif
      return rxConfirmationCode;  //setting was unsuccessful and so send confirmation code
    }
  }
  else {
    return response; //return packet receive error code
  }
}

//=========================================================================//
//delete templates saved in the library

uint8_t R30X_Fingerprint::deleteTemplate (uint16_t startLocation, uint16_t count) {
  if((startLocation > 1000) || (startLocation < 1)) { //if the value is not 1 or 2
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Deleting template failed.");
      Serial.println("Bad value. Start location must be #1 to #1000.");
      Serial.print("startLocation = ");
      Serial.println(startLocation);
    #endif

    return BAD_VALUE;
  }

  if((count + startLocation) > 1001) { //if the value is not in range
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Deleting template failed.");
      Serial.println("Bad value. Sum of startLocation and count can't be greater than 1001.");
      Serial.print("startLocation + count = ");
      Serial.println(startLocation + count);
    #endif

    return BAD_VALUE;
  }

  uint8_t dataArray[4] = {0}; //create data array
  dataArray[3] = ((startLocation-1) >> 8) & 0xFFU; //high byte of location
  dataArray[2] = ((startLocation-1) & 0xFFU); //low byte of location
  dataArray[1] = (count >> 8) & 0xFFU; //high byte of total no. of templates to delete
  dataArray[0] = (count & 0xFFU); //low byte of count

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Deleting template..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_DELETETEMPLATE, dataArray, 4); //send the command and data
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
     #ifdef FINGERPRINT_DEBUG
        Serial.println("Deleting template successful.");
        Serial.print("From #");
        Serial.print(startLocation);
        Serial.print(" to #");
        Serial.println(startLocation + count - 1);
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Deleting template failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::clearLibrary () {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Clearing library..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_CLEARLIBRARY); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Clearing library success.");
      #endif
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Clearing library failed.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::matchTemplates () {
  #ifdef FINGERPRINT_DEBUG
    Serial.println("Matching templates..");
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_MATCHTEMPLATES); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Matching templates success.");
      #endif

      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];
      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      #ifdef FINGERPRINT_DEBUG
        Serial.println("The templates do no match.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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

uint8_t R30X_Fingerprint::searchLibrary (uint8_t bufferId, uint16_t startLocation, uint16_t count) {
  if(!((bufferId > 0) && (bufferId < 3))) { //if the value is not 1 or 2
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Searching library failed.");
      Serial.println("Bad value. bufferId can only be 1 or 2.");
      Serial.print("bufferId = ");
      Serial.println(bufferId);
    #endif
    return BAD_VALUE;
  }

  if((startLocation > 1000) || (startLocation < 1)) { //if not in range (0-999)
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Searching library failed.");
      Serial.println("Bad start ID");
      Serial.print("startId = #");
      Serial.println(startLocation);
    #endif
    return BAD_VALUE;
  }

  if((startLocation + count) > 1001) { //if range overflows
    #ifdef FINGERPRINT_DEBUG
      Serial.println("Searching library failed.");
      Serial.println("startLocation + count can't be greater than 1001.");
      Serial.print("startLocation = #");
      Serial.println(startLocation);
      Serial.print("count = ");
      Serial.println(count);
      Serial.print("startLocation + count = ");
      Serial.println(startLocation + count);
    #endif
    return BAD_VALUE;
  }

  uint8_t dataArray[5] = {0};
  dataArray[4] = bufferId;
  dataArray[3] = (startLocation >> 8) & 0xFFU;  //high byte
  dataArray[2] = (startLocation & 0xFFU); //low byte
  dataArray[1] = (count >> 8) & 0xFFU; //high byte
  dataArray[0] = (count & 0xFFU); //low byte

  #ifdef FINGERPRINT_DEBUG
    Serial.println("Starting searching library for buffer content.");
    Serial.print("bufferId = ");
    Serial.println(bufferId);
    Serial.print("startLocation = #");
    Serial.println(startLocation);
    Serial.print("count = ");
    Serial.println(count);
    Serial.print("startLocation + count = ");
    Serial.println(startLocation + count);
  #endif

  sendPacket(FPS_ID_COMMANDPACKET, FPS_CMD_SEARCHLIBRARY, dataArray, 5); //send the command
  uint8_t response = receivePacket(); //read response

  if(response == RX_OK) { //if the response packet is valid
    if(rxConfirmationCode == FPS_RESP_OK) { //the confirm code will be saved when the response is received
      fingerId = uint16_t(rxDataBuffer[3] << 8) + rxDataBuffer[2];  //add high byte and low byte
      fingerId += 1;  //because IDs start from #1
      matchScore = uint16_t(rxDataBuffer[1] << 8) + rxDataBuffer[0];  //add high byte and low byte
      
      #ifdef FINGERPRINT_DEBUG
        Serial.println("Buffer content found in library.");
        Serial.print("fingerId = #");
        Serial.println(fingerId);
        Serial.print("matchScore = ");
        Serial.println(matchScore);
      #endif

      return FPS_RESP_OK; //just the confirmation code only
    }
    else {
      //fingerId = 0 doesn't mean the match was found at location 0
      //instead it means an error. check the confirmation code to determine the problem
      fingerId = 0;
      matchScore = 0;

      #ifdef FINGERPRINT_DEBUG
        Serial.println("Fingerprint not found.");
        Serial.print("rxConfirmationCode = ");
        Serial.println(rxConfirmationCode, HEX);
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