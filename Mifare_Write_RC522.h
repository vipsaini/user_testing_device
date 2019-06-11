/*#include <RobotIRremote.h>
#include <RobotIRremoteTools.h>
#include <RobotIRremoteInt.h>*/

#ifndef MIFARE_WRITE_RC522_H_INCLUDED
#define MIFARE_WRITE_RC522_H_INCLUDED

#include <SPI.h>
#include <MFRC522.h>

//Calculates Starting Address
byte startAddr(byte walletNo)
{
	byte blocksAvail = 0;
    byte blocksNeeded = (walletNo-1)*5;
    byte startingBlock = 4;

    while(blocksAvail < blocksNeeded )
    {
        if((startingBlock+1)%4 != 0)
        {
            blocksAvail++;
        }
        startingBlock++;
    }
    if((startingBlock+1)%4 == 0)
    {
    	startingBlock++;
    }
    return startingBlock;
}

//Return true if new card is present
bool detectNewCard(MFRC522 &mfrc522, byte cardPID[4][4], uint8_t cardNo)
{
    // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  
  Serial.println("Stored PID");

  //Check if one of the four cards have been tapped before
  /*for(int i=0; i<4; i++)
  {
      if(mfrc522.uid.uidByte[0]==cardPID[i][0] &&
         mfrc522.uid.uidByte[1]==cardPID[i][1] &&
         mfrc522.uid.uidByte[2]==cardPID[i][2] &&
         mfrc522.uid.uidByte[3]==cardPID[i][3])
      {
        Serial.println("Same UID Detected. Returning false");
        return false;
      }
   }*/
    Serial.println();
  Serial.println("UID of current Card");
  for(int j=0; j<4; j++)
  {
    cardPID[cardNo][j]=mfrc522.uid.uidByte[j];
    Serial.print(mfrc522.uid.uidByte[j]); Serial.print(" ");
  }
  Serial.println();
  return true;
}

//Overloading the function. This does not check for existing card.
bool detectNewCard(MFRC522 &mfrc522)
{
    // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  
  return true;
}

//Function to write to a block in card
bool writeToBlock(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key, byte *buffer, byte blockAdd)
{

    MFRC522::StatusCode status;
    //Authenticate
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAdd, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Write block
    status = mfrc522.MIFARE_Write(blockAdd, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    return true;
}

bool writeCard(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key, byte *data, byte walletNo)
{

    byte blockAdd = startAddr(walletNo); //Block number from where wallet will be stored
    byte blocksWritten = 0;

    while(blocksWritten < 5)
    {
        if((blockAdd+1)%4 != 0)
        {
            if(!writeToBlock(mfrc522, key, &data[blocksWritten*16], blockAdd))
                return false;
            blocksWritten++;
        }
        blockAdd++;
    }

    return true;
}


bool readCard(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key, byte *data, byte walletNo)
{
    MFRC522::StatusCode status;
    byte blockAdd = startAddr(walletNo); //Block number from where wallet will be stored
    byte blocksRead = 0;
    byte bytesRead = 0;
    byte buffer[18]; //Data read is first stored here
    byte len = sizeof(buffer);

    while(blocksRead < 5)
    {
        if((blockAdd+1)%4 != 0)
        {
            //Authenticate
            status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAdd, &key, &(mfrc522.uid));
            if (status != MFRC522::STATUS_OK) {
                Serial.print(F("PCD_Authenticate() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return false;
            }

            //Read in buffer[]
            status = mfrc522.MIFARE_Read(blockAdd, buffer, &len);
            if (status != MFRC522::STATUS_OK) {
                Serial.print(F("Reading failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
                return false;
            }

            //Copy to data[]
            for(int i=0; i<16; i++)
            {
                data[bytesRead] = buffer[i];
                bytesRead++;
            }

            blocksRead++;
        }
        blockAdd++;
    }

    return true;
}

//Not Tested
byte walletAvail(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key)
{
  //No of wallet stored in 1st byte of block 1
  MFRC522::StatusCode status;
  byte blockAdd = 1; //Block number from where wallet will be stored
  byte buffer[18]; //Data read is first stored here
  byte len = sizeof(buffer);

  //Authenticate
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAdd, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return 0;
  }

  //Read in buffer[]
  status = mfrc522.MIFARE_Read(blockAdd, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return 0;
  }

  return buffer[0];
}

//Not Tested
bool incrWalletAvail(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key)
{
  MFRC522::StatusCode status;
  byte blockAdd = 1; //Block number from where available wallet will be stored
  byte blocksRead = 0;
  byte bytesRead = 0;
  byte buffer[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Data read is first stored here
  byte len = sizeof(buffer);

  /**Imp line**/
  buffer[0]=(walletAvail(mfrc522, key)+1)%4;

  if(!writeToBlock(mfrc522, key, buffer, blockAdd))
    return false;

   return true;
}
#endif // MIFARE_WRITE_RC522_H_INCLUDED
