#include <SPI.h>
#include <Wire.h>

#include <U8g2lib.h> //U8g2lib

#include <MFRC522.h>
#include "Mifare_Write_RC522.h"  

#include "wordlistEnglish.h"
#include "sha256.h"
#include "gf256.h"
#include "Share.h"


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display


/**Pins**/
#define RST_PIN         3          // Pins for RC522
#define SS_PIN          2          // Pins for RC522
#define buzzer          4
#define outputA_dial    10 
#define outputB_dial    11
#define buttonPin       12         //the number of the pushbutton pin
/********/

#define waitTime        500       //Time between tapping and untapping (lack of better word)

#define rep(i,x,n) for(int i=x;i<n;i++)


/*
 * TODO : Check OLD Mnemonic generation
 * TODO : Remove all comments about name and Password
 * TODO : add xcordinate 1,2,3,4 and not other for PCD_authentication
 */


void setup() 
{
  Serial.begin(9600);
  SPI.begin();                                                  // Init SPI bus
  MFRC522 mfrc522(SS_PIN, RST_PIN);  
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  //pinMode(buttonPin, INPUT_PULLUP);pinMode(outputA_dial, INPUT_PULLUP);pinMode(outputB_dial, INPUT_PULLUP);
  //digitalWrite(buttonPin, HIGH);//digitalWrite(outputA_dial, HIGH);digitalWrite(outputB_dial, HIGH);
  pinMode(buttonPin, INPUT_PULLUP);
  u8g2.begin(buttonPin, outputA_dial, outputB_dial, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);

  /**Specific for this UTD design**/
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(buzzer, OUTPUT);

  /***Starting Logo***/
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_shylock_nbp_tf);
  u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nCYPHEROCK");
  u8g2.sendBuffer();
  delay(2000);
  
starting:

        
while(1)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_shylock_nbp_tf);
  String menuList="Secure Wallet\nRecover Wallet\nCheck Health";
  int x=u8g2.userInterfaceSelectionList("Menu", 1, menuList.c_str()); //Displays the main menu on oled
  x=x-1; //u8g2 returns 1 to n. converting x from 0 to n-1
  Serial.print("Choice of User - "); Serial.println(x);
  

  if(x==0)
  {
    //Goes into this if condition if secure wallet is selected
      String phraseList[]={" OLD "," NEW "};
      u8g2.setFontRefHeightAll();                                                                                       //Adds extra space between fonts. Useful before userInterfaceMessage
      uint8_t walletType=u8g2.userInterfaceMessage("Choose\nWallet Instance", NULL, NULL, conctString(phraseList, 2));
      u8g2.setFontRefHeightText();                                                                                      //Sets font space to default
      walletType-=1; //0 if OLD. 1 if NEW.
      
      String walletName="a987";//choiceLetters("\nEnter Wallet\nName", 37, 2, 8);
      Serial.print("Wallet Name : ");Serial.println(walletName);
      
      String walletPassword="aaaaaa";//choiceLetters("\nEnter\nPassword :", 37, 6, 10);
      Serial.print("Wallet Password : ");Serial.println(walletPassword);

      String mnemonicList="12\n18\n24";
      int numberOfMnemonics=u8g2.userInterfaceMessage("Choose Number \nof Mnemonics", NULL, NULL, mnemonicList.c_str()/*conctString(mnemonicList, 3)*/);
      
      Serial.print("numberOfMnemonics : ");Serial.println(numberOfMnemonics);
      numberOfMnemonics=(numberOfMnemonics+1)*6;    //Convert mnemonics from 0,1,2 to 12,18,24 
      
      uint8_t mnemonic[35];

      if(walletType==0)
      {
        String temp="";      //Store string of binary converted mnemonics taken from user

        for(int i=0;i<numberOfMnemonics;i++)
        {
             String tempName;
             String tempList[100];
             int len=0,start=123456;
             while(1)
             {  
                  tempName=choiceTwoChars("\nEnter\nMnemonic");
                  rep(i,0,2048)
                  {
                      if(tempName[0]==ENGLISH[i][0] && tempName[1]==ENGLISH[i][1]){
                          start=min(start,i);
                          tempList[len]=ENGLISH[i];
                          len++;
                      }
                  }
                  if(len!=0)
                      break;
             }
             
             int number;
             number=u8g2.userInterfaceSelectionList("Mnemonics", 1, conctString(tempList, len));
             Serial.print("NUMBER : ");
             Serial.println(number);
             number+=start;
             number--;     
             Serial.println(number);
        
        	  temp+=toBinary(number,11);
            Serial.print(number);
            Serial.print(" : " + ENGLISH[number] + " : ");
            Serial.println(temp);
        }    
                
        int numberOfBytes=((numberOfMnemonics*11)/8)+1;
        
        rep(i,0,numberOfBytes)
        {
          uint8_t tempNumber=0; //store temporarily 8 bits from 'temp' string
          rep(j,0,8)
          {	
            tempNumber<<=1;
            tempNumber |= (temp[i*8+j]=='1');
          }
          Serial.print("Temp Number : ");
          Serial.println(tempNumber);
          mnemonic[i]=tempNumber;
        }
        
        Serial.print("17 : ");
        Serial.println(mnemonic[16]);
      }
      else
      {
        //Goes here if NEW Wallet was selected. Generate random Mnemonics.
        /*TODO :
         * Display Mnemonics. Code for displaying them is below. Need to convert them into a single string with \n in between menmonics.
         */
        Serial.println("reached new");
        
        uint8_t *randomMnemonics=createRandomArray(numberOfMnemonics); //Random Array is generated but not tested.
        /*for(int i=0;i<numberOfMnemonics;i++){
            toString(randomMnemonics[i])
            Serial.println(toString(randomMnemonics[i]));
        }*/

        Serial.print("numberOfMnemonics : ");Serial.println(numberOfMnemonics);

        //rep(i,0,35)
        //    mnemonic[i]=(uint8_t)randomMnemonics[i];
        uint8_t temp[]={231,222,143,43,64,62,17,8,199,110,11,212,31,114,215,136,174,128,19,120,1,240,253,21,5,126,227,121,67,69,2,34,11,31,235};

        for(int i=0;i<35;i++)
            mnemonic[i]=temp[i];
        
        for(int i=0;i<35;i++)
            Serial.println(mnemonic[i]);
        String newMnemonicString="";
        for(int i=0;i<35;i++){
            newMnemonicString+=decimalToBinaryString((int)mnemonic[i]);
            Serial.print(mnemonic[i]);Serial.print(" : ");Serial.println(decimalToBinaryString(mnemonic[i]));
        }
        
        
        Serial.println(newMnemonicString);

        int index=0;
        String out_str="";
        for(int i=0;i<numberOfMnemonics*11;i++)
        {
            if(i%11==0 && i!=0)
            {
                Serial.println(ENGLISH[index]);
                out_str+=ENGLISH[index];
                out_str+="\n";
                index=newMnemonicString[i]=='1';
            }
            else
            {
                index<<=1;
                index|=(newMnemonicString[i]=='1');
            }
        }
        out_str+=ENGLISH[index];
        out_str+="\nGenerate Shares";

        int choice=0;
        while(choice!=numberOfMnemonics+1)
        {
            choice=u8g2.userInterfaceSelectionList("Genrated Mnemonics", 1, out_str.c_str()); 
        }
      }

      u8g2.clearBuffer();
      u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nGenerating\nShares...");
      u8g2.sendBuffer();
        
    	uint8_t *hash = new uint8_t[SHA256_BLOCK_SIZE];

      SHA256_CTX ctx;
      sha256_init(&ctx);
      sha256_update(&ctx, mnemonic, 34);
      sha256_final(&ctx, hash);

      /*Display(1,0,0,"HASH",1000);

      for(int i=0;i<32;i++){
          Display(1,0,0,toString(int(hash[i])),1000);
          Serial.println((int)(hash[i]));
      }*/
        
      if(numberOfMnemonics==12)
      {
          //mnemonic[16]&=11110000;
          for(int i=0;i<17;i++)
            mnemonic[i+17]=hash[i];
          mnemonic[34]=12;
      }
      else if(numberOfMnemonics==18)
      {
          //mnemonic[24]&=11111100;
          for(int i=0;i<9;i++)
              mnemonic[i+25]=hash[i];
          mnemonic[34]=18;
      }
      else
      {
          mnemonic[33]=hash[0];
          mnemonic[34]=24;
      }

      Serial.print("17 : ");
      Serial.println(mnemonic[16]);
      
      Share share[4];
      
      rep(i,0,4)
      {
        share[i].setWalletName(walletName);
        share[i].setPasswordAsString(walletPassword);
        //share[i].setChecksum(60);
        share[i].setWalletSize(numberOfMnemonics);
      }

      
      for(int i=0;i<35;i++)
      {
        Serial.println("Data for each share : "+toString(i+1));

        uint8_t c=mnemonic[i];
        uint8_t arr[4];
        uint8_t p=60;   //TODO : generate randomly
        uint8_t m=divide(subtract(c,p),subtract(0,1));
        arr[0]=add(multiply(m,1),c);
        arr[1]=add(multiply(m,2),c);
        arr[2]=add(multiply(m,3),c);
        arr[3]=add(multiply(m,4),c);


        for(int j=0;j<4;j++)
        {
            Serial.print((int)arr[j]);
            Serial.print(" ");
        }
        Serial.println();
        Serial.print(c);
        Serial.print(" : ");
        uint8_t mm=divide(subtract(arr[0],arr[1]),subtract(1,2));
        uint8_t temp=multiply(mm,1);
        temp=add(temp,arr[0]);  
        Serial.print(temp);
        //Serial.print((int)getSecret(1,arr[0],2,arr[2]));
        Serial.println();
  
          for(int j=0;j<4;j++){
              share[j].setShareByIndex(arr[j],i);
              share[j].setXcor(j+1);
          }
          delay(50);
        }

        /*TODO :
         * Add multiple wallets
         */
    
        //MFRC522 mfrc522(SS_PIN, RST_PIN);
        Serial.println("Reached MFRC");
        MFRC522::MIFARE_Key key = {keyByte: {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
        byte cardPID[4][4] = {0, 0, 0, 0,
                              0, 0, 0, 0,
                              0, 0, 0, 0,
                              0, 0, 0, 0}; //Store UID. UID of Mifare classic can be of 4, 7, 10 bytes. 4 bytes is used here.
                              
        byte walletNo = 1; /*(walletAvail(mfrc522, key) + 1)%4;*/

        /*
         * Choose wallet Number to enter the mnemonics 
         * Multiple wallet Implementation
         */
        u8g2.clearBuffer();
        u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\nTap a card to see\navailable wallets");
        u8g2.sendBuffer();
        Serial.println("Called Read from Card to return Name");

        /**Buzzer**/
        /*tone(buzzer, 1000); delay(60); noTone(buzzer);
        delay(waitTime); //Delay for effect
        tone(buzzer, 1000); delay(60); noTone(buzzer);
        delay(60); 
        tone(buzzer, 1000); delay(60); noTone(buzzer);*/
        /************/
    
      String walletNameList[4]={"","","",""};
      String walletPasswordList[4]={"","","",""};

      Share tempShare;
      while(1)
      {
          for(int i=0;i<4;i++)
          {
              walletNameList[i]="";
              walletPasswordList[i]="";
          }
          bool flag=1;
          Serial.println("Read Names : ");
          
          String walletData=tempShare.readCardReturnName(mfrc522);   
          Serial.print("walletData : ");Serial.println(walletData);

          for(int i=0;i<168;i+=42)
          {
            for(int j=0;j<10;j++)
            { 
                walletNameList[i/42]+=walletData[i+j];
                Serial.print(walletData[i+j]); 
            }
            Serial.println();
            for(int j=10;j<42;j++)
            {
                walletPasswordList[i/42]+=walletData[i+j];
                Serial.print(walletData[i+j]);
            }
            Serial.println();
          }
          
          for(int i=1;i<4;i++)
            if(walletNameList[i]==walletNameList[0] && walletNameList[0]!="UNALLOTED "){
                flag=0;
                Serial.print("Breaking :");Serial.println(i);
            }
            if(flag)
              break;
           delay(50);
           
      }
      for(int i=0;i<4;i++)
        Serial.println(walletNameList[i]);

      String temp="";
      for(int i=0;i<4;i++){
          temp+=walletNameList[i];
          if(i!=3)
             temp+="\n";
      }
    
      int walletNumber=u8g2.userInterfaceSelectionList("Select Wallet", 1, temp.c_str()/*conctString(walletNameList, 4)*/); //Displays available wallet
      //walletNumber-=1;

      Serial.print("Wallet Number : ");Serial.println(walletNumber);
      //Serial.println(walletNameList[x]);
        
        /**Writing shares to 4 cards**/
        rep(i,0,4)
        {

          Serial.println("Reached Card");
          String t="TAP CARD\n"+toString(i+1)+"   OF  4";
          u8g2.clearBuffer();
          u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), t.c_str());
          u8g2.sendBuffer();
  
          byte *tempShare=share[i].getShare();
          
          //share[i].shareToCard(mfrc522,i);


          Serial.print("Wallet Number : ");Serial.println(walletNumber);
          while(1)
          {
            
            int countDamage=0;
            Serial.println("Card detection");
            if(detectNewCard(mfrc522))  //detectNewCard also checks if same card is tapped again. i is card number.
            { 
              while(1)
              {
                  if(writeCard(mfrc522, key, tempShare, walletNumber))
                  {
                    Serial.println("Written");
                    //tone(buzzer, 1000); delay(60); 
                    //noTone(buzzer); //One beep
                    break;
                  }
                  else
                  {
                      Serial.println("Failed Writing");
                      countDamage++;
                      //u8g2.clearBuffer();
                      //u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\nError");
                      //u8g2.sendBuffer();
                      if(countDamage>=15){
                          Serial.println("Card Damaged ");
                          break;
                      }
                      //tone(buzzer, 1000); delay(45); noTone(buzzer); //One beep
                      //delay(45);
                      //tone(buzzer, 1000); delay(45); noTone(buzzer); //2nd beep
                      //delay(45);
                      //tone(buzzer, 1000); delay(45); noTone(buzzer); //3rd beep
                      //delay(45);
                      //tone(buzzer, 1000); delay(70); noTone(buzzer); //4th beep*/
                      continue;
                      
                  }
              }
              if(countDamage>=15){
                  countDamage=0;
                  continue;
              }
              

              byte buff[80];
              if(readCard(mfrc522, key,buff , walletNumber))
              {
                  Serial.println("Card is Read");
                  Serial.println("Data Read -:");
                  for(int i=0; i<80; i++)
                  {
                      if((i)%16 == 0)
                      Serial.println();
                    //Serial.print(buff[i], HEX); Serial.print(", ");     
                    Serial.print(buff[i]); Serial.print(", ");     
                  } 
                  Serial.println();
                  Serial.println("___________________________________");
              }   
              Serial.println(); Serial.println("Whole memory structure -:");
              mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); //Prints the whole memory structure
              break;
            }
            delay(50);   
          }
          share[i].show();        
          delay(2000); //wait time 2 sec bw card tap 
          /*tone(buzzer, 1000); delay(60); noTone(buzzer); //1st beep
          delay(60);
          tone(buzzer, 1000); delay(60); noTone(buzzer);*/
        }
        //incrWalletAvail(mfrc522, key);
    }
  else if(x==1)
  {
    //Goes here if Recover wallet is selected
    Share share;
    byte cardPID[4][4] = {0, 0, 0, 0,
                          0, 0, 0, 0,
                          0, 0, 0, 0,
                          0, 0, 0, 0}; //Store UID. UID of Mifare classic can be of 4, 7, 10 bytes. 4 bytes is used here.
    
    u8g2.clearBuffer();
    u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\nTap a card to see\navailable wallets");
    u8g2.sendBuffer();
    Serial.println("Called Read from Card to return Name");

    Serial.println("Reached Recover");
    /**Buzzer**/
      /*tone(buzzer, 1000); delay(60); noTone(buzzer);
      delay(waitTime); //Delay for effect
      tone(buzzer, 1000); delay(60); noTone(buzzer);
      delay(60); 
      tone(buzzer, 1000); delay(60); noTone(buzzer);*/
    /************/
    
    String walletNameList[4]={"","","",""};
    String walletPasswordList[4]={"","","",""};

    while(1)
    {
        Serial.println("Read Names");
        for(int i=0;i<4;i++)
        {
            walletNameList[i]="";
            walletPasswordList[i]="";
        }
        bool flag=1;
        Serial.println("Read Names : ");
        String walletData=share.readCardReturnName(mfrc522);   
        Serial.print("walletData : ");Serial.println(walletData);

        for(int i=0;i<168;i+=42)
        {
          for(int j=0;j<10;j++)
          {
              walletNameList[i/42]+=walletData[i+j];
              Serial.print(walletData[i+j]); 
          }
          Serial.println();
          for(int j=10;j<42;j++)
          {
              walletPasswordList[i/42]+=walletData[i+j];
              Serial.print(walletData[i+j]);
          }
          Serial.println();
        }

        for(int i=1;i<4;i++)
            if(walletNameList[i]==walletNameList[0] && walletNameList[0]!="UNALLOTED "){
                flag=0;
                Serial.print("Breaking :");Serial.println(i);
            }
         if(flag)
            break;
    }


    Serial.println();Serial.println("Names of Wallets:");
    for(int i=0;i<4;i++)
      Serial.println(walletNameList[i]);

    String temp="";
    for(int i=0;i<4;i++){
        temp+=walletNameList[i];
        if(i!=3)
           temp+="\n";
    }
    
    int walletNumber=u8g2.userInterfaceSelectionList("Select Wallet", 1, temp.c_str()/*conctString(walletNameList, 4)*/); //Displays available wallet
    Serial.print("Wallet Selected : "); Serial.println(walletNumber);
    
    //walletNumber;
    
    Serial.println(walletNameList[walletNumber-1]);

    while(1)
    {
        String sharePassword=choiceLetters("\nEnter Password", 37, 6, 10); //Asks for Password.

        /*uint8_t *hash = new uint8_t[SHA256_BLOCK_SIZE];

        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, sharePassword, 34);
        sha256_final(&ctx, hash);*/

        int len=sharePassword.length();


        Serial.println("passwords : ");
        Serial.println(sharePassword);
        Serial.println(walletPasswordList[walletNumber-1]);

        for(int i=len;i<32;i++)
            sharePassword+=" ";
            
        bool flag=1;
        
        for(int i=0;i<32;i++)
            if(sharePassword[i]!=walletPasswordList[walletNumber-1][i])
                flag=0;
        if(flag)
            break;
    }    
    
    
    uint8_t mnemonicsSecret[35]; 

    /**Storing first card**/
    u8g2.clearBuffer();
    u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nTap 1 of 2 Cards");
    u8g2.sendBuffer();
    
    uint8_t *tempWalletShare;
    
    while(1)
    {
          tempWalletShare=share.readFromCard(mfrc522,walletNumber, cardPID, 0);
          Serial.print("XCOR : ");Serial.println(tempWalletShare[77]);
          if(tempWalletShare[77]>=1 && tempWalletShare[77]<=4)
              break;
    }
  
    u8g2.clearBuffer();
    u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nTap 2 of 2 cards");
    u8g2.sendBuffer();  
    
    /**Buzzer**/
      /*tone(buzzer, 1000); delay(60); noTone(buzzer); //1st Beep
      delay(waitTime); //Delay for effect
      tone(buzzer, 1000); delay(60); noTone(buzzer); //2nd Beep
      delay(60);
      tone(buzzer, 1000); delay(60); noTone(buzzer); //3rd Beep*/
    /************/
    uint8_t walletShare1[35];
    for(int i=0;i<35;i++)
      walletShare1[i]=tempWalletShare[i];
    
    uint8_t walletxcor1=share.getXcor();
    uint8_t walletSize=share.getSize();
    
    for(int i=0;i<35;i++)
    {
      Serial.print(walletxcor1);
      Serial.print(",");
      Serial.print(walletShare1[i]);
      Serial.println(" : ");
    }

    delay(2000);
    /**Storing 2nd Card**/
    /*u8g2.clearBuffer();
    u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nTap 2 of 2 cards");
    u8g2.sendBuffer();*/

    Share share2;
    uint8_t *walletShare2;    
        
    while(1)
    {
        walletShare2=share2.readFromCard(mfrc522,walletNumber, cardPID, 0);
        Serial.print("XCOR : ");Serial.println(share2.getXcor());
        if(share2.getXcor()>=1 && share2.getXcor()<=4)
            break;
    }

//    u8g2.clearBuffer();
//    u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\n\nDONE");
//    u8g2.sendBuffer();
    
    /**Buzzer**/
      /*tone(buzzer, 1000); delay(60); noTone(buzzer); //1st Beep
      delay(waitTime); //Delay for effect
      tone(buzzer, 1000); delay(60); noTone(buzzer); //2nd Beep
      delay(60);
      tone(buzzer, 1000); delay(60); noTone(buzzer); //3rd Beep*/
    /************/
    uint8_t walletxcor2=share2.getXcor();
    
    /**Calculation of mnemonics**/
    for(int i=0;i<35;i++)
    {
      Serial.print(walletxcor1);
      Serial.print(",");
      Serial.print(walletShare1[i]);
      Serial.print(" : ");
      Serial.print(walletxcor2);
      Serial.print(",");
      Serial.println(walletShare2[i]);
    }
    
    String tempMnemonics="";
    for(int i=0;i<35;i++)
    {
      uint8_t mm=divide(subtract(walletShare2[i],walletShare1[i]),subtract(walletxcor1,walletxcor2));
      uint8_t temp=multiply(mm,walletxcor1);
      temp=add(temp,walletShare1[i]);  
      //Serial.print(temp);
      mnemonicsSecret[i]=temp;//getSecret(walletxcor1,walletShare1[i],walletxcor2,walletShare2[i]);
      Serial.print("Mnemonic : ");Serial.print(i);Serial.print(" : ");Serial.println(mnemonicsSecret[i]);
      tempMnemonics+=decimalToBinaryString(mnemonicsSecret[i]);
      Serial.println(mnemonicsSecret[i]);
    
      if(i==15 || i==16)
      {
          Serial.print("- -  : ");
          Serial.println(mnemonicsSecret[i]);
          Serial.println(decimalToBinaryString(mnemonicsSecret[i]));
      }
    }
    
    int countX=0;
    for(int i=0;i<tempMnemonics.length();i++)
    {
        if(i%11==0){
          Serial.println();
          countX++;
          Serial.println(countX);
          Serial.print(tempMnemonics[i]);
        }
        else
          Serial.print(tempMnemonics[i]);
    }
    Serial.println();
    Serial.println(tempMnemonics);
    
    Serial.println();
    Serial.print("Wallet Size : ");
    Serial.println(walletSize);

    /**Converting mnemonics into a string with \n in between which can then be displayed**/
    String out_mnemonics="";
    for(int i=0;i<walletSize-1;i++) //Splitted this for loop into 2. For creating the string out_mnemonics.
    {
        int x=0;
        for(int j=0;j<11;j++)
        {
            x<<=1;
            x|=((tempMnemonics[(i*11)+j])=='1');
        }
        Serial.print(x);
        Serial.print("  ");
        Serial.println(ENGLISH[x]);
        out_mnemonics+=ENGLISH[x]+"\n";
        //Display(2,0,0,ENGLISH[x],50);
    }
    {
      int x=0;
      for(int j=0;j<11;j++)
      {
          x<<=1;
          x|=((tempMnemonics[((walletSize-1)*11)+j])=='1');
      }
      Serial.print(x);
      Serial.print("  ");
      Serial.println(ENGLISH[x]);
      out_mnemonics+=ENGLISH[x];
      out_mnemonics+="\n<Go To Menu>"; //Last Option
    }
    uint8_t choice = 0;
    while(choice != walletSize+1)
    {
      choice = u8g2.userInterfaceSelectionList("Your Mnemonics", choice, out_mnemonics.c_str()); //Keeps showing mnemonics until user selects last option
    }
  }
  else
  {
    //Check Health should be here
        u8g2.clearBuffer();
        u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\nTap a card to see\navailable wallets");
        u8g2.sendBuffer();

        
       
  }
}
  //goto starting;
}

void loop()
{
    
}

/*TODO : Make header file for these utility functions*/

/*************Utility Functions*************/
String decimalToBinaryString(int x)
{
    String temp="";
    while(x)
    {
        temp+=char(48+x%2);
        x/=2;
    }
    int len=temp.length();
    for(int i=len;i<8;i++)
        temp+="0";
    
    for(int i=0;i<4;i++)
    {
        char c=temp[i];
        temp[i]=temp[7-i];
        temp[7-i]=c;
    }
    return temp;
}

uint8_t getSecret(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
    uint8_t m=divide(subtract(y2,y1),subtract(x2,x1));
    uint8_t temp=multiply(m,x1);
    temp=add(temp,y1);  
    return temp;
}


String toBinary(int number,int reqLen)
{
	String temp="";
	while(number)
	{
		temp+=char(48+(number%2));
		number/=2;
	}
	int len=temp.length();

  for(int i=len;i<reqLen;i++)
    temp+="0";
  
	for(int i=0;i<(reqLen-1)/2;i++)
	{
		char c=temp[i];
		temp[i]=temp[reqLen-i-1];
		temp[reqLen-i-1]=c;
	}
	return temp;
}

uint8_t *createRandomArray(int Size)
{
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_TRNG;   // TRNG clock on
    TRNG->CTRLA.reg = TRNG_CTRLA_ENABLE;
    uint8_t arr[35], REPS = 1000;

    for(int j=0; j<35; j++)
    {
      for (int i = 0; i < REPS; i++) {
        while ((TRNG->INTFLAG.reg & TRNG_INTFLAG_DATARDY) == 0) ; // 84 APB cycles
        arr[j] = TRNG->DATA.reg;
      }
    }
    return arr;
}

String convert(String list[],int Size,int index)
{
    String temp="";
    rep(i,0,Size)
    {
        if(i==index)
            temp+=">";
        else 
            temp+=" ";
        temp+=list[i];
        temp+="\n\n";
    }
    return temp;
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

const char * conctString(String *list, uint8_t len)
{
  String temp = "";
  for(uint8_t i=0; i<len-1; i++)
  {
    temp += list[i] + "\n";
  }
  temp += list[len-1];

  return temp.c_str();
}
/**************End of Utility Functions**************/


uint8_t *generateShare(uint8_t c)
{
    uint8_t arr[4];
    for(int i=1;i<=4;i++)
    {
        uint8_t p=60;   //TODO : generate randomly
        arr[0]=p;
        arr[1]=2*p-c;
        arr[2]=3*p-2*c;
        arr[3]=4*p-3*c;
    }
    return arr;
}

/**Function to input 2 char of mnemonics**/
String choiceTwoChars(String msg){
  //User can select letters from this list
  uint8_t choice=0;
  const char *c[] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"};
  String input = ""; //input is stored here
  rep(i, 0, 2)
  {
    u8g2.userInterfaceInputValue(msg.c_str(), input.c_str(), &choice, 0, 25, 4, "", c);     
    Serial.println(c[choice]);
  
    input += c[choice];
  }

  u8g2.setFontRefHeightAll();
  choice = u8g2.userInterfaceMessage("Your Input:", input.c_str(), NULL, " OK \n Cancel "); //Returns: 1 to n for if one of the buttons had been selected.
  u8g2.setFontRefHeightText();

  if(choice == 1)
    return input;
  else
    choiceTwoChars(msg);
}


/**Function to input any string(password or wallet name)**/
String choiceLetters(String msg, uint8_t inputOpt, uint8_t minChar, uint8_t maxChar){
  //User can select letters from this list
  uint8_t choice=0;
  const char *c[] = {"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z",
                     "0","1","2","3","4","5","6","7","8","9", "<GO>"}; //Space has been removed
  String input = ""; //input is stored here 
  while(1)
  {
    if(input.length() > maxChar)
    {
      u8g2.clearBuffer();
      u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "Max Character\nLimit\nReached");
      u8g2.sendBuffer();
      delay(2000);
      break;
    }
    
    //Warning is showed when only one more character can be entered
    if(input.length() > maxChar-1)
    {
      u8g2.clearBuffer();
      u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "1 Character\nLeft");
      u8g2.sendBuffer();
      delay(2000);
    }
    
    u8g2.userInterfaceInputValue(msg.c_str(), input.c_str(), &choice, 0, inputOpt-1, 4, "", c);     
    Serial.println(c[choice]);
  
    //if OK
    if(choice == 36)
    {
      if(input.length() < minChar)
      {
        u8g2.clearBuffer();
        u8g2.DrawUTF8Lines(0, u8g2.getAscent(), u8g2.getDisplayWidth(), u8g2.getAscent() - u8g2.getDescent(), "\nInput is too short.\n Renter"); //See u8glib_text
        u8g2.sendBuffer();
        delay(2000);
        choice=0;
        input="";
      }
      else
      {
        break;
      }
    }
    else
    {
      input += c[choice];
    }
  }

  u8g2.setFontRefHeightAll();
  choice = u8g2.userInterfaceMessage("Your Input", input.c_str(), NULL, " OK \n Cancel "); //Returns: 1 to n for if one of the buttons had been selected.
  u8g2.setFontRefHeightText();

  if(choice == 1)
    return input;
  else
    choiceLetters(msg, inputOpt, minChar, maxChar);
}
