#include <Keypad.h>             //library for keypad
#include <Wire.h>               //library for LCD
#include "RTClib.h"             //library for LCD
#include <LiquidCrystal_I2C.h>  //library for LCD
#include <SPI.h>                //library for RFID
#include <MFRC522.h>            //library for RFID
#include "DHT.h"

//-------------------Part that you reconfigurate of PIN-------------------------

#define RST_PIN  5  // From Pin chart : RST/Reset Mega Pin 5
#define SS_PIN  53  // From Pin chart : SPI SS Mega Pin 53
#define DHTPIN  2  // Digital pin connected to the DHT sensor
#define DHTTYPE  DHT11   // DHT 11 (DHT type)
#define WATER_PIN  A8 // Analog pin connected to water level sensor
#define LDR_PIN  A0 // Analog pin connected to LDR
#define LED_RED  8 // Digital pin connected to RED LED
#define LED_GREEN  11 // Digital pin connected to GREEN LED
#define LED_YELLOW  12 // Digital pin connected to YELLOW LED
#define BUZZER_PIN  10 // Digital pin connected to Buzzer
const byte interruptPin = 3; // Digital pin connected to interrupt (button)

//------------------------------------------------------------------------------

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
LiquidCrystal_I2C lcd(0x27, 16, 2); // Create LiquidCrystal_I2C size 2 rows and 16 columns LCD
RTC_DS3231 RTC;                     // Create RTC_DS3231 instance
DHT dht(DHTPIN, DHTTYPE);           // Create dht instance

int buzzer = BUZZER_PIN;

//-----------------Part that you reconfigurate of buzzer frequency--------------

int buzzer_freq = 10;  // configure frequency of buzzer when having problems
int buzzer_fo = 10000;  // configure frequency of buzzer when interrupt

//------------------------------------------------------------------------------

unsigned long t0,t1;
float humid,temp;
int LDR_val, water_level;

//-----------------Part that you reconfigurate of LCD showing time--------------

long interval = 5000; // Time of LCD monitoring (ms)

//------------------------------------------------------------------------------

int d_of_m[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'0','1','2','3'},
  {'4','5','6','7'},
  {'8','9','A','B'},
  {'C','D','E','F'}
};
char baseKey[13] = {'0','1','2','3','4','5','6','7','8','9','A','B','C'};
byte rowPins[ROWS] = {29, 27, 25, 23}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {31, 33, 35, 37}; //connect to the column pinouts of the keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// base variable value of range humidity, Temperature, LCR value, Water level (always in range)
float humid_LOW = -100;
float humid_HIGH = 200;
float temp_LOW = -100;
float temp_HIGH = 200;
int ldr_LOW = -100;
int ldr_HIGH = 5000;
int water_LOW = -100;
int water_HIGH = 5000;
//variable to tell time  in process
int hour1 ;
int minute1 ;
int day1 ;
int month1 ;
int year1 ;
int mintime_deliver;

//--------------------------------function change char to dec integer = return dec integer
int c2DEC(char c)
{
  int cha = -1;
  if ((c>='0') && (c<='9')){
    cha = c-'0';
  }
  if ((c>='A') && (c<='F')){
    cha = c+10-'A';
  }
  if ((c>='a') && (c<='f')){
    cha = c+10-'a';
  }
  return cha ;
}

void blink() {
  Serial.println("Open door");
  tone(buzzer,buzzer_fo,1000);
}

//--------------------------------function read UID of card = return UID string
String UID_RFID(byte *buffer, byte bufferSize) {  // create fucntion dump_byte_array to print UID in Serial Monitor and LCD
  String UID = "";
  //lcd.setCursor(0, 0); //set cursor of LCD to begin at column 1 and row 1 (0,0)
  //lcd.print("UID:"); //print "UID:" on LCD at row 1 column 1-4
  for (byte i = 0; i < bufferSize; i++) {                //use for loop to run every UID number (4 number) from mfrc522.uid.size
        if (buffer[i] < 0x10) {
          UID = UID + " 0";
        }
        else {
          UID = UID + " ";
        }
        UID = UID + String(buffer[i], HEX);
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");     //if UID number < 10 (0-9) print " 0" else print " " on Serial Monitor
        //lcd.print(buffer[i] < 0x10 ? " 0" : " ");        //if UID number < 10 (0-9) print " 0" else print " " on LCD
        Serial.print(buffer[i], HEX);                    //print UID number in HEX on Serial Monitor
        //lcd.print(buffer[i], HEX);                       //print UID number in HEX on LCD
  }
  UID.toUpperCase();
  UID = UID.substring(1);
  delay(500);  //delay 1 second of the display of UID after taked out RFID from detector
  return UID;
}

//-------------------------------function to print time 2 digits
void print2digits(int number){
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number, DEC);
}

//--------------------------------------------------------------------------------------------------------------------------
//-------------------------------------Start Program------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

void setup(){
  Serial.begin(9600);
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  lcd.begin();                                                  // Initialize LCD
  Wire.begin();                                                 // Initialize Wire
  RTC.begin();                                                  // Initialize RTC
  RTC.adjust(DateTime(F(__DATE__),F(__TIME__)));                // Adjust RTC     
  dht.begin();                                                  // Initialize DHT
  pinMode(LED_RED, OUTPUT);                                     // LED pin = OUTPUT pin
  pinMode(LED_GREEN, OUTPUT);                                   // LED pin = OUTPUT pin
  pinMode(LED_YELLOW, OUTPUT);                                  // LED pin = OUTPUT pin
  pinMode(interruptPin, INPUT_PULLUP);                          // Interrupt pin = INPUT PULLUP pin
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink , RISING);     //Attach interupt to program
}

void loop(){

  //-----------------------------------Read RFID if valid start process else retry tap RFID------------------
  lcd.setCursor(0, 0); //set cursor of LCD to begin at column 1 and row 1 (0,0)
  lcd.print("Tap RFID Card");
  String UID_code = "00 21 B3 56";

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));
  
  String UID;
  Serial.print(F("Card UID:"));                               //print "Card UID:" on Serial Monitor
  UID = UID_RFID(mfrc522.uid.uidByte, mfrc522.uid.size);     //use dump_byte_array function that byte *buffer = mfrc522.uid.uidByte and  byte bufferSize = mfrc522.uid.size
  Serial.println();

  //-------------------------------- CARD CORRECTED   ------>    Start process
  if (UID == UID_code){
    Serial.println("UID-Valid");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Valid Card");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    
    //--------------------------------Information of Button to choose Type of Medical Supply------------------
    lcd.print("Select Type");
    Serial.println("Type of Medical Supply");
    Serial.println(" 1: Organs");
    Serial.println(" 2: Blood");
    Serial.println(" 3: Morphine");
    Serial.println(" 4: Light-sensitive vaccines");
    Serial.println(" 5: Liquid Oxygen");

    //Use RTC to find time at the moment and storing in variables
    DateTime now = RTC.now();
    int hour0 = now.hour();
    int minute0 = now.minute();
    int day0 = now.day();
    int month0 = now.month();
    int year0 = now.year();

    //-------------------------------loop to setting process
    while (1){
      char key = kpd.getKey();
      if(key){
        Serial.println("-----key detected");
        if (key=='0' || key=='1' || key=='2' || key=='3' || key=='4'){
          if (key == '0'){
            // Base data of variable range of Organs
            mintime_deliver = 270;
            humid_LOW = 0;
            humid_HIGH = 100;
            temp_LOW = 2;
            temp_HIGH = 100;
            ldr_LOW = 0;
            ldr_HIGH = 120;
            water_LOW = 0;
            water_HIGH = 20;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Organs");
            Serial.println(" Organs");
          }
          // Base data of variable range of Organs
          if (key == '1'){
            mintime_deliver = 240;
            //humid_LOW = 0;
            //humid_HIGH = 100;
            temp_LOW = 2;
            temp_HIGH = 10;
            //ldr_LOW = 0;
            //ldr_HIGH = 100;
            water_LOW = 0;
            water_HIGH = 50;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Blood");
            Serial.println(" Blood");
          }
          // Base data of variable range of Organs
          if (key == '2'){
            mintime_deliver = 240;
            humid_LOW = 0;
            humid_HIGH = 100;
            temp_LOW = 0;
            temp_HIGH = 100;
            ldr_LOW = 0;
            ldr_HIGH = 100;
            water_LOW = 0;
            water_HIGH = 200;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Morphine");
            Serial.println(" Morphine");
          }
          // Base data of variable range of Organs
          if (key == '3'){
            mintime_deliver = 240;
            humid_LOW = 0;
            humid_HIGH = 100;
            temp_LOW = 0;
            temp_HIGH = 100;
            ldr_LOW = 0;
            ldr_HIGH = 100;
            water_LOW = 0;
            water_HIGH = 200;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Light-sensitive");
            lcd.setCursor(1, 0);
            lcd.print("vaccines");
            Serial.println(" Light-sensitive vaccines");
          }
          // Base data of variable range of Organs
          if (key == '4'){
            mintime_deliver = 240;
            humid_LOW = 0;
            humid_HIGH = 100;
            temp_LOW = 0;
            temp_HIGH = 100;
            ldr_LOW = 0;
            ldr_HIGH = 100;
            water_LOW = 0;
            water_HIGH = 200;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Liquid Oxygen");
            Serial.println(" Liquid Oxygen");
          }
          
          //Template to insert new Supply
          /*if (key == '5'){
            mintime_deliver = 240;
            humid_LOW = 0;
            humid_HIGH = 100;
            temp_LOW = 0;
            temp_HIGH = 100;
            ldr_LOW = 0;
            ldr_HIGH = 100;
            water_LOW = 0;
            water_HIGH = 200;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("New Supply");
            Serial.println("New Supply");
          }*/

          
          delay(1500);
          break;
        }
        else{
          Serial.print("Retry");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Retry");
          delay(500);
        }
      }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start");
    delay(1000);
    lcd.clear();
    
    //---------------------------------------loop to check problem, warn, display
    while (1){
      humid = dht.readHumidity();     //Read humidity from sensor
      //Serial.print("Humid = ");
      //Serial.println(humid);
      temp = dht.readTemperature();   //Read Temperature from sensor
      //Serial.print("Temp = ");
      //Serial.println(temp);
      if (isnan(humid) || isnan(temp)) {
        Serial.println(F("Failed to read from DHT sensor"));
      }
      LDR_val = analogRead(LDR_PIN);   //Read LDR value
      //Serial.print("LDR value = ");
      //Serial.println(LDR_val);
      water_level = analogRead(WATER_PIN);    //Read Water level from sensor
      //Serial.print("Water = ");
      //Serial.println(water_level);
      
      //  Condition to check that variable in range or not
      if (!((humid_LOW < humid) && (humid < humid_HIGH)) || !((temp_LOW < temp) && (temp < temp_HIGH)) || 
      !((ldr_LOW < LDR_val) && (LDR_val < ldr_HIGH)) || !((water_LOW < water_level) && (water_level < water_HIGH))) {
        //Serial.println("Problem detect");
        //turn on buzzer to warning
        tone(buzzer, buzzer_freq);
        if (!((humid_LOW < humid) && (humid < humid_HIGH)) || !((temp_LOW < temp) && (temp < temp_HIGH))) {
          digitalWrite(LED_RED,HIGH);   //Turn ON RED LED
        }
        else {
          digitalWrite(LED_RED,LOW); //Turn OFF RED LED
        }
        if (!((ldr_LOW < LDR_val) && (LDR_val < ldr_HIGH))) {
          digitalWrite(LED_YELLOW,HIGH);  //Turn ON YELLOW LED
        }
        else {
          digitalWrite(LED_YELLOW,LOW);     //Turn OFF YELLOW LED
        }
        if (!((water_LOW < water_level) && ( water_level< water_HIGH))) {
          digitalWrite(LED_GREEN,HIGH);  //Turn ON GREEN LED
        }
        else {
          digitalWrite(LED_GREEN,LOW); //Turn OFF GREEN LED
        }
      }
      else {
        //turn off all LEDs and buzzer
        noTone(buzzer);
        digitalWrite(LED_RED,LOW);   
        digitalWrite(LED_GREEN,LOW);
        digitalWrite(LED_YELLOW,LOW);
      }
      
      bool c = true;
      static int lcdState = LOW;
        char key = kpd.getKey();
        if(key) {

          //LCD display from button that push
                    
          if ( key=='0' || key=='1' || key=='2' || key=='3' || key=='C' || key=='F') {
            Serial.print("key detected :  ");
            if (key =='0') {
              float humid = dht.readHumidity();
              if (lcdState == LOW) {
                t0 = millis();
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("HUMIDITY:");
                lcd.setCursor(0, 1);
                lcd.print(humid);
                Serial.println("Humidity");
              }
            }
            if (key =='1') {
              float temp = dht.readTemperature();
              if (lcdState == LOW) {
                t0 = millis();
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("TEMPERATURE:");
                lcd.setCursor(0, 1);
                lcd.print(temp);
                Serial.println("Temperature");
              }
            }
            if (key =='2') {
              water_level = analogRead(WATER_PIN);
              if (lcdState == LOW) {
                t0 = millis();
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("WATER LEVEL:");
                lcd.setCursor(0, 1);
                lcd.print(water_level);
                Serial.println("Water level");
              }
            }
            if (key =='3') {
              LDR_val = analogRead(LDR_PIN);
              if (lcdState == LOW) {
                t0 = millis();
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("LDR VALUE:");
                lcd.setCursor(0, 1);
                lcd.print(LDR_val);
                Serial.println("LDR value");
              }
            }
            
            //----------------------------Button to display deadline
            if (key == 'C') {
                day1 = day0;
                month1 = month0;
                year1 = year0;
                minute1 = minute0 +( mintime_deliver % 60);
                hour1 = hour0;
                if (minute0 + mintime_deliver >= 60) {
                  hour1 = (hour0 + (mintime_deliver / 60)) % 24;
                  if (minute1 >=60) {
                    hour1 = hour1 + 1;
                    minute1 = minute1 - 60; 
                  }

                  if (hour0 + (mintime_deliver / 60) >= 24) {
                    day1 = day0 + mintime_deliver / (24 * 60);
                    if (day1 > d_of_m[month0]) {
                      month1 = month1 + 1;
                      day1 = day1 - d_of_m[month0];
                    }
                    if (month1 == 13) {
                      month1 = 1;
                      year1 = year1+1;
                    }
                  }
                }
                
                lcd.clear();
                lcd.setCursor(0, 0);
                print2digits(hour1);
                lcd.print(':');
                print2digits(minute1);
                lcd.setCursor(0, 1);
                print2digits(day1);
                lcd.print('/');
                print2digits(month1);
                lcd.print('/');
                lcd.print(year1, DEC);
                Serial.println("Deadline");

     
            }

            //-------------------------Button to stop the process
            if (key == 'F') {
              Serial.println("Stop process");
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Stop");
              delay(1500);
              lcd.clear();
              noTone(buzzer);
              digitalWrite(LED_RED,LOW);
              digitalWrite(LED_GREEN,LOW);
              digitalWrite(LED_YELLOW,LOW);
              exit(0);
            } 
          }                  
        }
        // Condition to make LCD display time (interval,ms) and do not use delay (Concept of Multitasking using millis())
        else {
          t1 = millis();
          if (t1 - t0 > interval) {
            lcd.clear();
          }
        }
    }
  }
  //--------------Retry to tap RFID card until valid
  else {
    Serial.println("UID-Invalid");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Card");
    delay(1000);
    lcd.clear();
    lcd.print("Retry");
    delay(500);
    lcd.clear();
  }  
  delay(500); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
