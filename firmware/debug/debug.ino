#include <EEPROM.h>
#define ADDR 10

#include <avr/io.h>
#include <avr/wdt.h>

#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

#define buttonPIN 3
#define pumpPIN_A 5
#define pumpPIN_B 4
#define fanPIN 9
#define ledPIN 6

#define pushDrop 1
#define dropTime 50
#define emergency_count_max 20
#define sensTime 1000

#define fanSpeed 130
#define fanTime 12000

#define pushMix 1000
#define mixTime 6000
bool pump_mode = false;

void stato_emergenza(bool stato);
void bagna();
void asciuga();
void ferma();
uint8_t ff(unsigned long in);

void setup() {

  pinMode(buttonPIN, INPUT_PULLUP);
  pinMode(pumpPIN_A, OUTPUT);
  pinMode(pumpPIN_B, OUTPUT);
  pinMode(fanPIN, OUTPUT);

  digitalWrite(ledPIN, LOW);
  digitalWrite(fanPIN, LOW);
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);

  Serial.begin(9600);

  Serial.println("DEBUG MODE:");
  Serial.println("\t V - ventola ON al max");
  Serial.println("\t v - ventola OFF");
  Serial.println("\t P - pompa bagna");
  Serial.println("\t p - pompa asciuga");
  Serial.println("\t A - leggi sensore");
  Serial.println("\t a - ferma lettura");




}

bool primo = true;
bool premuto = false;

int emergency_count = 0;


uint8_t mix = 0;
uint8_t count = 0;
uint8_t bigCount = 0;
uint8_t brightness = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long lastDrop = 0;
bool drop = false;

unsigned long lastSens = 0;
bool sens = false;
bool firstSens = true;

unsigned long lastLed = 0;
bool ledState = LOW;

unsigned long lastFan = 0;
bool fanState = LOW;

unsigned long lastMix = 0;
unsigned long lastPush = 0;
unsigned long timeStep = 0;
unsigned long lastStep = 0;
unsigned long timeLed = 0;
unsigned long emergencyTime = 0;

bool leggi = false;

void loop() {

  if (leggi) {
    Serial.println(analogRead(A5));
    delay(100);
  }

  
  if (Serial.available() > 0) {
    char in = Serial.read();
    switch (in) {
      case 'P':
        bagna();
        break;
      case 'p':
        asciuga();
        break;
      case 'f':
        ferma();
        break;
      case 'V':
        digitalWrite(fanPIN, HIGH);
        break;
      case 'v':
        digitalWrite(fanPIN, LOW);
        break;
      case 'A':
        leggi = true;
        break;
      case 'a':
        leggi = false;
        break;
      default:
        Serial.println("cosa?");
        break;
    }
  }


}

void bagna() {
  digitalWrite(pumpPIN_A, HIGH);
  digitalWrite(pumpPIN_B, LOW);
}

void asciuga() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, HIGH);
}

void ferma() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);
}

uint8_t ff(unsigned long in) {
  if (in > 2000) {
    return 10;
  }
  else {
    return map(in, 0, 2000, 0, 10);
  }
}

void stato_emergenza(bool stato) {

  if (stato) {
    EEPROM.write(ADDR, 5);
    Reset_AVR();
  }
  else {
    EEPROM.write(ADDR, 255);
  }

}

