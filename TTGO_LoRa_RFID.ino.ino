#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <U8g2lib.h>
#include <MFRC522.h>


#define LORA_SCK     5    
#define LORA_MISO    19   
#define LORA_MOSI    27 
#define LORA_SS      18  
#define LORA_RST     14   
#define LORA_DI0     26  
#define LORA_BAND    868E6

#define RFID_SDA 5 
#define RFID_SCK 18 
#define RFID_MOSI 23
#define RFID_MISO 19
#define RFID_RST 27

String uidString;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
MFRC522 mfrc522(RFID_SDA, RFID_RST);  // Create MFRC522 instance
int current_spi = -1; // -1 - NOT STARTED   0 - RFID   1 - LORA

void setup() {
  u8g2.begin();         // Initialise graphics
  Serial.begin(115200);   // Initialise serial port
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  
}

void loop() {
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB12_tr); // choose font
  u8g2.drawStr(0,25,"Scan Tag/Card:");  // write internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
  delay(1000);  
  bool card_present = RFID_check();
  if (card_present) LORA_send();
}

void spi_select(int which) {
     if (which == current_spi) return;
     SPI.end();
     
     switch(which) {
        case 0:
          SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI);
          mfrc522.PCD_Init();   
        break;
        case 1:
          SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_SS);
          LoRa.setPins(LORA_SS,LORA_RST,LORA_DI0);
        break;
     }

     current_spi = which;
}

int RFID_check() {
  spi_select(0);
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
    return false;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
     Serial.print("My Card UID:");
    printDec(mfrc522.uid.uidByte, mfrc522.uid.size);

    uidString = String(mfrc522.uid.uidByte[0])+" "+String(mfrc522.uid.uidByte[1])+" "+String(mfrc522.uid.uidByte[2])+ " "+String(mfrc522.uid.uidByte[3]);
    
    printUID();

     mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  return true;
}

void LORA_send() {
  spi_select(1);
  
  Serial.println("LoRa Sender Test"); 
  
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("init ok");
 
  delay(1500);

  // send packet
  LoRa.beginPacket();
  LoRa.print (uidString);
  LoRa.endPacket();

  Serial.println("Sent UID");
  delay(2000);                       // wait for a second
 }
  void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void printUID()
  {
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB12_tr); // choose font
  u8g2.setCursor(0, 45);
  u8g2.print(uidString);  // write UID to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
  }
