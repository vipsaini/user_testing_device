#include<stdint.h>

class Share{
    uint8_t share[35];
    uint8_t password[32];
    uint8_t walletName[10];
    uint8_t xcor;
    uint8_t walletSize;
    uint8_t checksum;
    uint8_t SHARE[80];

    public:
    Share(){
      for(int i=0;i<80;i++)
          SHARE[i]=0;
          for(int i=0;i<32;i++)
            this->password[i]=0;
        //walletName={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0};
    }

    void setPasswordAsString(String passwordString)
    {
        for(int i=0;i<32;i++)
            this->password[i]=char(32);
        int len=passwordString.length();
        for(int i=0;i<len;i++)
            this->password[i]=passwordString[i];
    }
    void setPassword(uint8_t passwordHash[])
    {
        for(int i=0;i<32;i++)
            this->password[i]=passwordHash[i];
    } 
    
    void setWalletName(String name)
    {
        int len=name.length();
        for(int i=0;i<len;i++)
            walletName[i]=uint8_t(name[i]);
        for(int i=len;i<10;i++)
            walletName[i]=uint8_t(32);
    }

    void setShareByIndex(uint8_t shareAtIndex,int index)
    {
        share[index]=shareAtIndex;
    }

    void setXcor(uint8_t xcor)
    {
        this->xcor=xcor;
    }

    void setshare(int size,uint8_t shares[])
    {
        for(int i=0;i<34;i++)
            share[i]=shares[i];
        share[34]=uint8_t(size);
    }

    void setChecksum(uint8_t checksum)
    {
        this->checksum=checksum;
    }

    void setWalletSize(uint8_t Size)
    {
        this->walletSize=Size;
    }

    String getPassword()
    {
        String temp="";
        for(int i=45;i<77;i++)
            temp+=char(this->SHARE[i]);
        return temp;
    }
    
    uint8_t* getShare()
    {
        int index=0;
        for(int i=0;i<35;i++)
          SHARE[i]=share[i];
        index+=35;
        for(int i=0;i<10;i++)
            SHARE[index+i]=walletName[i];
        index+=10;
        for(int i=0;i<32;i++)
            SHARE[index+i]=password[i];
        index+=32;
        SHARE[index]=xcor;
        index++;
        SHARE[index]=walletSize;
        return SHARE;
    }

    uint8_t getSize()
    {
        return this->SHARE[78];
    }
    uint8_t getXcor()
    {
        return this->SHARE[77];
    }
    void show(){

        Serial.print("Share : ");
        for(int i=0;i<35;i++){
            Serial.print(this->SHARE[i]);
            Serial.print(" ");
        }
        Serial.println();
        
        Serial.print("WalletName : ");
        for(int i=0;i<10;i++)
          Serial.print(char(this->SHARE[35+i]));

        Serial.println();

        Serial.println("Password Hash");
        for(int i=0;i<32;i++)
            Serial.print(char(this->SHARE[i+45]));

        Serial.println();
        
        Serial.print("Xcor : ");
        Serial.println(this->SHARE[77]);

        
        Serial.println();

        Serial.print("Size : ");
        Serial.println(this->SHARE[78]);

        //delay(5000);

    }

    void shareToCard(MFRC522 &mfrc522,int walletNo)
    {
        //this->SHARE=this->getShare();

        //MFRC522 mfrc522(2, 3);
        //SPI.begin();
        //mfrc522.PCD_Init(); 

        
        uint8_t *tempShare=this->getShare();
        for(int i=0;i<80;i++)
            this->SHARE[i]=tempShare[i];

        MFRC522::MIFARE_Key key = {keyByte: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

        while(1)
        {
            if(detectNewCard(mfrc522))
            {
                if(writeCard(mfrc522, key, this->SHARE, walletNo))
                {
                  Serial.println("Written");
                }
                else{
                  Serial.println("Failed Writing");
                }
            } 
        }
    }

    bool comparePassword(uint8_t sharePassword[])
    {
        for(int i=0;i<32;i++)
            if(sharePassword[i]!=this->SHARE[45+i])
                return 0;
        return 1;
    }
    
    byte *readFromCard(MFRC522 mfrc522, byte walletNo, byte cardPID[4][4], uint8_t cardNo)
    {
        MFRC522::MIFARE_Key key = {keyByte: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
        
        byte buff[80]; //Data read will be stored in this array
        
        Serial.println("Trying to read Mifare Card");//create mfrc instance
        
        while(1)
        {
              if(detectNewCard(mfrc522))
              {
                if(readCard(mfrc522, key, this->SHARE, walletNo))
                {
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                    if((i)%16 == 0)
                      Serial.println();
                    Serial.print(this->SHARE[i]); Serial.print(", ");     
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                  }      
                  Serial.println();
                  Serial.println("___________________________________");
                }
                Serial.println(); Serial.println("Whole memory structure -:");
                mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); //Prints the whole memory structure

                //for()
                return this->SHARE;
                break;
              }
        }
    }

    String readCardReturnName(MFRC522 mfrc522)
    {
        String walletData="";
        String walletNameList[4];
        
        MFRC522::MIFARE_Key key = {keyByte: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

        //byte walletNo = 4; //Wallet No can be 1,2,3,4
        byte buff[80]; //Data read will be stored in this array
        
        Serial.println("RUNNING");//create mfrc instance

        String temp="";
        
        uint32_t currentCardUID=0;
        while(1)
        {
              if(detectNewCard(mfrc522))
              {
                if(readCard(mfrc522, key, buff, 1))
                {
                  for(int k=0;k<80;k++)
                      this->SHARE[k]=buff[k];
                  Serial.println("Card is Read");
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                    if((i)%16 == 0)
                      Serial.println();
                    Serial.print(this->SHARE[i]); Serial.print(", ");     
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                  }      
                  Serial.println();
                  Serial.println("___________________________________");
                }
                walletData+=this->getWalletName();
                walletData+=this->getPassword();
                
                if(readCard(mfrc522, key, buff, 2))
                {
                  for(int k=0;k<80;k++)
                      this->SHARE[k]=buff[k];
                  Serial.println("Card is Read");
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                    if((i)%16 == 0)
                      Serial.println();
                    Serial.print(this->SHARE[i]); Serial.print(", ");     
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                  }      
                  Serial.println();
                  Serial.println("___________________________________");
                }
                walletData+=this->getWalletName();
                walletData+=this->getPassword();
                                
                if(readCard(mfrc522, key, buff, 3))
                {
                  for(int k=0;k<80;k++)
                      this->SHARE[k]=buff[k];
                  Serial.println("Card is Read");
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                    if((i)%16 == 0)
                      Serial.println();
                    Serial.print(this->share[i]); Serial.print(", ");     
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                  }      
                  Serial.println();
                  Serial.println("___________________________________");
                }
                walletData+=this->getWalletName();
                walletData+=this->getPassword();
                

                if(readCard(mfrc522, key, buff, 4))
                {
                  for(int k=0;k<80;k++)
                      this->SHARE[k]=buff[k];
                  Serial.println("Card is Read");
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                    if((i)%16 == 0)
                      Serial.println();
                    Serial.print(this->SHARE[i]); Serial.print(", ");     
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                  }      
                  Serial.println();
                  Serial.println("___________________________________");
                }
                walletData+=this->getWalletName();
                walletData+=this->getPassword();

                
                Serial.println(); Serial.println("Whole memory structure -:");
                mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); //Prints the whole memory structure

                //for()
                return walletData;
                break;
              }
        }

    }

    String getWalletName(){
        String name="";
        for(int i=35;i<45;i++)
            name+=char(this->SHARE[i]);
        return name;
    }

    uint8_t returnShareByIndex(int index)
    {
         return this->share[index];
    }
    
    String toString(int x)
    {
        String temp="";
        while(x)
        {
            temp+=char(48+x%10);
            x/=10;
        }
        int len=temp.length();
        for(int i=0;i<len/2;i++)
        {
            char c=temp[i];
            temp[i]=temp[len-i-1];
            temp[len-i-1]=c;
        }
        return temp;
    }
};
