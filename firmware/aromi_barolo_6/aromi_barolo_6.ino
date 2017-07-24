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

#define sensorPIN A5
#define WET_TRESHOLD 400
#define IST 20

#define pushDrop 4
#define dropTime 50

#define fanSpeed 130
#define fanTime 1000

#define retroTime 1000

#define emergencyTime 40000

#define fakeMAX 5000
#define brightOK 255
#define brightKO 30
#define SMOOTH 15

int smooth = 0;

bool pump_mode = false;

bool primo = true;
bool premuto = false;

uint8_t count = 0;
int tempCount = 0;
long bigCount = 0;

int brightness = 0;
int brightnessFake = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long lastDrop = 0;
bool drop = false;

unsigned long lastStart = 0;

unsigned long lastLed = 0;
bool ledState = LOW;

unsigned long lastFan = 0;
bool fanState = LOW;

unsigned long lastPush = 0;
unsigned long timeStep = 0;
unsigned long lastStep = 0;
unsigned long timeLed = 0;

bool jeansWet = false;
bool pumpStart = true;

bool ledRise = true;

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

  //emergency
  if (EEPROM.read(ADDR) == 5) {
    pinMode(ledPIN, OUTPUT);
    digitalWrite(ledPIN, HIGH);
    while (1) {}
  }

  //turn andrera
  long retroStart = millis();
  while ((millis() - retroStart) < retroTime) {
    asciuga();
  }

  //bagna il jeans
  while (!jeansWet) {
    check_jeans();
    bagna();
  }
  ferma();
}

//// LLLLLLOOOOOOOOOOOOOOOOPPPPPPPP

void loop() {
  //MODO POMPA
  /*
    Se all'accensione tengo premuto il pulsante per più
    di 1 secondo ma meno di 5 entro nella modalità
    PUMP_MODE

    In cui posso gestire la pompa riempiendo e svuotando
    interagendo solamente con il pulsante
  */
  if (pump_mode) {
    while (1) {
      modo_pompa();
    }
  }

  //PULSANTE
  /*
    legge lo stato del pulsante, gestisce il debounce,
    aggiorna 2 contatori:

    count++
    bigCount++

    ed una variabile booleana:

    premuto (true=schiacciato, false=non-schiacciato)

    ad ogni pulsata
  */
  leggi_pulsante();


  //JEANS
  /*
    Controlla lo stato del jeans e aggiorna una
    variabile booleana

    jeansWet (true=bagnato, false=asciutto)

  */
  check_jeans();


  //ROUTINE
  if (!count && jeansWet) {
    //Pulsante NON premuto + jeans bagnato
    ferma();

    leds_ready();

  }
  else if (count && jeansWet) {
    //Pulsante premuto + jeans bagnato
    ferma();

    leds_on();

    //FANS
    /*
      Se premuto == true allora aziona le ventole
      per un tempo pari a == fanTime

      poi count a 0

    */
    aziona_fans();



  }
  else if (!count && !jeansWet) {
    //Pulsante NON premuto + jeans asciutto
    ferma();

    leds_ko();

  }
  else if (count && !jeansWet) {
    //Pulsante premuto + jeans asciutto
    leds_charge();

    /*
      Bagno il jeans fino al segnale asciutto
    */
    bagna();

  }


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
    count = 0;

  }
}

//CHECK JEANS
void check_jeans() {
  if (analogRead(sensorPIN) <= (WET_TRESHOLD + IST)) {
    jeansWet = false;
  }
  else if (analogRead(sensorPIN) > (WET_TRESHOLD - IST)) {
    jeansWet = true;
  }
}



//LED FADE
void leds_ready() {
  //sali
  if (ledRise) {
    if (brightnessFake > fakeMAX) {
      brightness = brightOK;
      brightnessFake = fakeMAX;
      ledRise = false;
    }
    else if (brightnessFake >= brightOK) {
      brightness = brightOK;
      brightnessFake++;
    }
    else {
      if (brightnessFake > brightOK * 0.75) {
        smooth = SMOOTH;
      }
      else {
        smooth = SMOOTH * 2;
      }
      if ((millis() - lastStep) > smooth) {
        lastStep = millis();
        brightness++;
        brightnessFake++;
      }
    }
  }

  //stai su

  //scendi
  else {
    if (brightnessFake < 1) {
      brightness = 0;
      brightnessFake = 0;
      ledRise = true;
    }
    else if (brightnessFake < (fakeMAX - brightOK)) {
      brightness = 0;
      brightnessFake--;
    }
    else {
      if (brightnessFake > brightOK * 0.75) {
        smooth = SMOOTH;
      }
      else {
        smooth = SMOOTH * 2;
      }
      if ((millis() - lastStep) > smooth) {
        lastStep = millis();
        brightness--;
        brightnessFake--;
      }
    }
  }

  //stai giu

  analogWrite(ledPIN, brightness);
}

void leds_on() {
  // LED FADE IN //
  if (premuto) {
    if (brightness >= brightOK) {
      brightness = brightOK;
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
      if ((millis() - (lastPush)) > fanTime) {
        brightness--;
      }
    }
  }
  analogWrite(ledPIN, brightness);
}


void leds_ko() {

  //sali
  if (ledRise) {
    if (brightnessFake > fakeMAX) {
      brightness = brightKO;
      brightnessFake = fakeMAX;
      ledRise = false;
    }
    else if (brightnessFake >= brightKO) {
      brightness = brightKO;
      brightnessFake++;
    }
    else {
      if (brightnessFake > brightKO * 0.75) {
        smooth = SMOOTH;
      }
      else {
        smooth = SMOOTH * 2;
      }
      if ((millis() - lastStep) > smooth) {
        lastStep = millis();
        brightness++;
        brightnessFake++;
      }
    }
  }

  //stai su

  //scendi
  else {
    if (brightnessFake < 1) {
      brightness = 0;
      brightnessFake = 0;
      ledRise = true;
    }
    else if (brightnessFake < (fakeMAX - brightKO)) {
      brightness = 0;
      brightnessFake--;
    }
    else {
      if (brightnessFake > brightKO * 0.75) {
        smooth = SMOOTH;
      }
      else {
        smooth = SMOOTH * 2;
      }
      if ((millis() - lastStep) > smooth) {
        lastStep = millis();
        brightness--;
        brightnessFake--;
      }
    }
  }

  //stai giu

  analogWrite(ledPIN, brightness);
}

void leds_charge() {
  if (ledRise) {
    if (brightness >= brightOK) {
      brightness = brightOK;
      ledRise = false;
    }
  }
  else {
    brightness = 0;
    ledRise = true;
  }

  analogWrite(ledPIN, brightness);
}



//ALTRO
void bagna() {
  if (pumpStart) {

    digitalWrite(pumpPIN_A, HIGH);
    digitalWrite(pumpPIN_B, LOW);

    lastStart = millis();

    pumpStart = false;
  }

  if ((millis() - lastStart) > emergencyTime) {
    ferma();
    stato_emergenza(true);
  }

}

void asciuga() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, HIGH);
}

void ferma() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);
  pumpStart = true;
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
