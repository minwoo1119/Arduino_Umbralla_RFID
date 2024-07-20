/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

Servo mg995_1; // 1번 카드에 해당하는 모터
Servo mg995_2;
const int servoPin1 = 6; // 1번 모터 핀 6번으로 세팅
const int servoPin2 = 7; // 2번 모터 핀 7번으로

#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

// 1번 카드 번호 저장하고 있는 변수
byte card1[4] = {0x7D,0x1B,0x6F,0x6B};
// 2번 카드 번호 저장하고 있는 변수
byte card2[4] = {0x0D,0x9B,0xCB,0x6B};

bool isFirstTagging1 = true; // 처음 태깅하는건지 - 1번 카드에 해당하는 모터
bool isFirstTagging2 = true; // 처음 태깅하는건지 - 2번 카드에 해당하는 모터

void setup() { 
  Serial.begin(9600);
  mg995_1.attach(servoPin1);
  mg995_2.attach(servoPin2);
  SPI.begin(); // SPI 초기화
  rfid.PCD_Init(); // MFRC522 초기화

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

 // 새로운 카드가 읽혔을 때
  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // NUID 읽어서 저장
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    // Serial.println(F("The NUID tag is:"));
    // Serial.print(F("In hex: "));
    // printHex(rfid.uid.uidByte, rfid.uid.size);
    // Serial.println();
  }
  else { // 기존에 읽었던 카드를 읽었을 때
    Serial.println(F("Card read previously."));
  };
  
  // 1번 카드 찍혔을 때 실행하는 부분
  if (rfid.uid.uidByte[0] == card1[0] && 
    rfid.uid.uidByte[1] == card1[1] && 
    rfid.uid.uidByte[2] == card1[2] && 
    rfid.uid.uidByte[3] == card1[3] ) {
    Serial.println("Card1 Detected.");
    if(isFirstTagging1 == true){
      CloseHook();
      isFirstTagging1 = false;
    }else{
      OpenHook();
      isFirstTagging1 = true;
    }
  }

  // 2번 카드 찍혔을 때 실행하는 부분
  if (rfid.uid.uidByte[0] == card2[0] && 
  rfid.uid.uidByte[1] == card2[1] && 
  rfid.uid.uidByte[2] == card2[2] && 
  rfid.uid.uidByte[3] == card2[3] ) {
    Serial.println("Card2 Detected.");
    if(isFirstTagging2 == true){
      CloseHook2();
      isFirstTagging2 = false;
    }else{
      OpenHook2();
      isFirstTagging2 = true;
    }
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void InitSavedInfo(){
  for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = 0x0;
    }
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) { // 16진수로 출력하는 함수 -> 참고로 놔둠
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}


void OpenHook(){ // 1번 모터 여는 함수
  for(int i=0;i<90;i+=10){
    mg995_1.write(i);
    delay(15);
  }
}

void CloseHook(){ // 1번 모터 닫는 함수
  for(int i=90;i>0;i-=10){
    mg995_1.write(i);
    delay(15);
  }
}

void OpenHook2(){ // 2번 모터 여는 함수
  for(int i=0;i<90;i+=10){
    mg995_2.write(i);
    delay(15);
  }
}

void CloseHook2(){ // 2번 모터 닫는 함수
  for(int i=90;i>0;i-=10){
    mg995_2.write(i);
    delay(15);
  }
}

