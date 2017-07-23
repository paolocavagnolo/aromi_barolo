
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

void setup() {
  pinMode(buttonPIN, INPUT_PULLUP);
  pinMode(pumpPIN_A, OUTPUT);
  pinMode(pumpPIN_B, OUTPUT);
  pinMode(fanPIN, OUTPUT);

  digitalWrite(ledPIN, LOW);
  digitalWrite(fanPIN, LOW);
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);

  if (!digitalRead(buttonPIN)) {
    delay(5000);
    if (!digitalRead(buttonPIN)) {
      stato_emergenza(false);
    }
    else {
      pump_mode = true;
    }
  }

  delay(1000);

  //emergency
  if (EEPROM.read(ADDR) == 5) {
    pinMode(ledPIN, OUTPUT);
    digitalWrite(ledPIN, HIGH);
    while (1) {}
  }

  //bagna il jeans
  while (!bagna_jeans()) {}
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

void loop() {
  //MODO POMPA
  if (pump_mode) {
    while (1) {
      modo_pompa();
    }
  }

  //PULSANTE
  leggi_pulsante();

  //FANS
  aziona_fans();

  //POMPA GOCCIA
  goccia();

  //POMPA BAGNA
  bagna_jeans();

  //POMPA MESCOLA
  mescola();

  //LED FADE
  leds();

}

//MODO POMPA
void modo_pompa() {
  // PULSANTE //
  if ((!digitalRead(buttonPIN)) && primo) {
    lastDebounceTime = millis();
    primo = false;
    count++;
    //Serial.println(count);
  }
  if (((millis() - lastDebounceTime) > debounceDelay) && !primo) {
    if (!digitalRead(buttonPIN)) {
      premuto = true;
    }
    else {
      premuto = false;
      primo = true;
      lastPush = (unsigned long)(millis() - lastDebounceTime);
    }
  }

  if (count % 3 == 0) {
    ferma();
    bigCount = 0;
  }
  else if (count % 3 == 1) {
    asciuga();
    bigCount = 1;
  }
  else if (count % 3 == 2) {
    bagna();
    bigCount = 2;
  }

  timeLed = 1000 - 400 * bigCount;
  if ((millis() - lastLed) > timeLed) {
    digitalWrite(ledPIN, ledState);
    ledState = !ledState;
    lastLed = millis();
  }
}

//PULSANTE
void leggi_pulsante() {
  if ((!digitalRead(buttonPIN)) && primo) {
    lastDebounceTime = millis();
    primo = false;
    count++;
    bigCount++;
    //Serial.println(count);
  }
  if (((millis() - lastDebounceTime) > debounceDelay) && !primo) {
    if (!digitalRead(buttonPIN)) {
      premuto = true;
    }
    else {
      premuto = false;
      primo = true;
      lastPush = (unsigned long)(millis() - lastDebounceTime);
    }
  }
}

//FANS
void aziona_fans() {
  if (premuto && !fanState) {
    analogWrite(fanPIN, fanSpeed);
    lastFan = millis();
    fanState = true;
  }

  if (((millis() - lastFan) > fanTime) && fanState) {
    digitalWrite(fanPIN, LOW);
    fanState = false;
  }
}

//POMPA GOCCIA
void goccia() {
  if ((count > pushDrop) && !drop && !mix && !sens) {
    bagna();
    lastDrop = millis();
    count = 0;
    drop = true;
  }
  if (((millis() - lastDrop) > dropTime) && drop) {
    ferma();
    drop = false;
  }
}

//POMPA BAGNA JEANS
bool bagna_jeans() {
  if ((analogRead(A5) <= 400)) {
    //tutto secco
    bagna();
    if (firstSens) {
      lastSens = millis();
      emergency_count++;
    }
    sens = true;
    firstSens = false;
    return false;
  }
  else if (!drop && ((millis() - lastSens) > sensTime)) {
    //bagnato
    ferma();
    emergencyTime = millis();
    sens = false;
    firstSens = true;
    return true;
  }
  if (emergency_count >= emergency_count_max) {
    ferma();
    stato_emergenza(true);
    sens = false;
  }
  //RESET EMERGENCY COUNT
  if ((millis() - emergencyTime) > 7200000UL) {
    emergency_count = 0;
  }
}

//POMPA MESCOLA
void mescola() {
  if ((bigCount > pushMix) && (mix == 0)) {
    asciuga();
    lastMix = millis();
    bigCount = 0;
    mix = 1;
  }
  if (((millis() - lastMix) > mixTime) && (mix == 2)) {
    ferma();
    mix = 0;
  }
  else if (((millis() - lastMix) > mixTime / 2) && (mix == 1)) {
    bagna();
    mix = 2;
  }
}

//LED FADE
void leds() {
  // LED FADE IN //
  if (premuto) {
    if (brightness > 254) {
      brightness = 255;
    }
    else {
      brightness++;
    }
  }
  // LED FADE OUT //
  else {
    if (brightness < 1) {
      brightness = 0;
    }
    else {
      if ((millis() - lastStep) > ff(lastPush)) {
        brightness--;
        lastStep = millis();
      }
    }
  }
  analogWrite(ledPIN, brightness);
}



//ALTRO
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

