#include <IRremote.h>
#include <IRremoteInt.h>
#include <BitVoicer11.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>//get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash

/*
 */

#define NETWORKID         100  //the same on all nodes that talk to each other (range up to 255)
#define CONTROLLERNODEID  2    //must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define SPELLNODEID       5
#define FREQUENCY         RF69_915MHZ
#define ENCRYPTKEY        "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW        //uncomment only for RFM69HW! Leave out if you have RFM69W!
//#define ENABLE_ATC        //comment out this line to disable AUTO TRANSMISSION CONTROL
#define LED               9 // Moteinos have LEDs on D9
#define SERIAL_BAUD       115200

char payload[25];         //buffer for spell sent to controller
byte snedSize=0;
boolean requestACK = false;
RFM69 radio;

//#define PHONE_PIN A0
//int pinPhone = 0;
int pinLeft = 4;
int pinCenter = 7;
int pinRight = 8;
int pinLED = 9;
int recvPin = 5;        //PIN receiving signal from IR
unsigned long startTime;
String currentSpell;
boolean wasSpellSpoken = false;
boolean wasWandWaved = false;
//int phoneSpell = 0;

IRrecv irrecv(recvPin);
decode_results results;

BitVoicerSerial bitVoicer = BitVoicerSerial();
byte dataType = BV_STR;

//----------------------------------------------------------------------
// the setup function runs once when you press reset or power the board
//----------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("moteinoBVWithIR");
  
  radio.initialize(FREQUENCY,SPELLNODEID,NETWORKID);
  radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(ENCRYPTKEY);
  Serial.println("radio ready to transmit\n");

  irrecv.enableIRIn();   //enable the IR sensor
  
  // initialize output pins.
  pinMode(pinLeft, OUTPUT);
  pinMode(pinCenter, OUTPUT);
  pinMode(pinRight, OUTPUT);
  pinMode(pinLED, OUTPUT);

  digitalWrite(pinLeft, HIGH);   
  digitalWrite(pinCenter, HIGH);   
  digitalWrite(pinRight, HIGH);   
  delay(500);
  digitalWrite(pinLeft, LOW);   
  digitalWrite(pinCenter, LOW);   
  digitalWrite(pinRight, LOW);   
}

//----------------------------------------------------------------------
// the loop function runs over and over again forever
//----------------------------------------------------------------------
void loop() {
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
    if (results.value == 0x219E10EF) {
      wasWandWaved = true;
      digitalWrite(pinLED, HIGH);
      startTime = millis();
    }    
    // Receive the next value
    irrecv.resume(); 
  } else {
    if ((startTime > 0) && (millis() - startTime > 4000))
      resetTimer();     
  }

  // read the state of the phonePin value:
  //phoneSpell = analogRead(pinPhone);

  //if (phoneSpell == 1) {
  //  invokeSpell("lefton");
  //  resetTimer();     
  //} else if (phoneSpell == 2) {
  //  invokeSpell("center");
  //  resetTimer();     
  //} else if (phoneSpell == 3) {
  //  invokeSpell("right");
  //  resetTimer();     
  //}

  if (wasSpellSpoken && wasWandWaved) {
    invokeSpell(currentSpell);
    resetTimer();     
  }
}

//----------------------------------------------------------------------
// this function resets the timer and settings when the timer expires
//----------------------------------------------------------------------
void resetTimer() {
  digitalWrite(pinLED, LOW);
  wasSpellSpoken = false;
  wasWandWaved = false;
  startTime = 0;
}

//----------------------------------------------------------------------
// this function is fired whenever a spell is spoken
//----------------------------------------------------------------------
void serialEvent() {
  bitVoicer.getData();

  currentSpell = bitVoicer.strData;

  if (currentSpell == "") 
    return;
  
  digitalWrite(pinLED, HIGH);
  wasSpellSpoken = true;
  startTime = millis();
}

//----------------------------------------------------------------------
// this function executes the spoken spell
//----------------------------------------------------------------------
void invokeSpell(String strSpell) {
  //send spell to controller
  char cmd[25];
  strSpell.toCharArray(cmd, 25);
  
  Serial.println("Sending: " + strSpell);

  radio.send(CONTROLLERNODEID, cmd, strSpell.length());

  if (strSpell == "$w") 
    leftOnSpell();
  else if (strSpell == "$k") 
    leftOffSpell();
  else if (strSpell == "$B") 
    centerSpell();
  else if (strSpell == "right") 
    rightSpell();
}

//----------------------------------------------------------------------
void leftOnSpell() {
    Serial.println("left on spell");
    digitalWrite(pinLeft, HIGH);   
}

//----------------------------------------------------------------------
void leftOffSpell() {
    Serial.println("left off spell");
    digitalWrite(pinLeft, LOW);   
}

//----------------------------------------------------------------------
void centerSpell() {
    Serial.println("center spell");
    digitalWrite(pinCenter, HIGH); 
    delay(3000);                   
    digitalWrite(pinCenter, LOW);  
}

//----------------------------------------------------------------------
void rightSpell() {
    Serial.println("right spell");
    digitalWrite(pinRight, HIGH);  
    delay(3000);                   
    digitalWrite(pinRight, LOW);   
}

