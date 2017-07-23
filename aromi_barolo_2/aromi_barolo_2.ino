
#define buttonPIN 3
#define pumpPIN_A 5
#define pumpPIN_B 4
#define fanPIN 9
#define ledPIN 6

#define pushDrop 1
#define dropTime 50

#define fanSpeed 120
       

#define pushMix 1000
#define mixTime 6000
bool pump_mode = false;

void setup() {

  pinMode(buttonPIN, INPUT_PULLUP);
  pinMode(pumpPIN_A, OUTPUT);
  pinMode(pumpPIN_B, OUTPUT);
  pinMode(fanPIN, OUTPUT);
  pinMode(ledPIN, OUTPUT);


  digitalWrite(ledPIN, LOW);
  digitalWrite(fanPIN, LOW);
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);
 
  while (!digitalRead(buttonPIN)) {
    pump_mode = true;
  }

  delay(1000);
  

}

bool primo = true;
bool premuto = false;
bool drop = false;

uint8_t mix = 0;
uint8_t count = 0;
uint8_t bigCount = 0;
uint8_t brightness = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

unsigned long lastDrop = 0;
unsigned long lastMix = 0;

unsigned long lastPush = 0;
unsigned long timeStep = 0;
unsigned long lastStep = 0;

unsigned long lastLed = 0;
unsigned long timeLed = 0;
bool ledState = LOW;

void loop() {

  if (pump_mode) {
    while (1) {
            
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

      // LED //
      timeLed = 1000 - 400 * bigCount;
      if ((millis() - lastLed) > timeLed) {
        digitalWrite(ledPIN, ledState);
        ledState = !ledState;
        lastLed = millis();
      }

      
    }
  }

  // PULSANTE //
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

  // FANS //
  if (premuto) {
    analogWrite(fanPIN, fanSpeed);
  }
  else {
    digitalWrite(fanPIN, LOW);
  }


  // POMPA GOCCIA //
  if ((count > pushDrop) && !drop && !mix) {
    bagna();
    lastDrop = millis();
    count = 0;
    drop = true;
  }
  if (((millis() - lastDrop) > dropTime) && drop) {
    ferma();
    drop = false;
  }


  // POMPA MESCOLA //
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

/*

  //Appena schiaccio il pulsante
  if ((!digitalRead(buttonPIN)) && primo) {
  primo = false;
  digitalWrite(fanPIN, HIGH);
  count++;
  st = millis();
  }

  //Mentre schiaccio il pulsante
  if (!digitalRead(buttonPIN)) {
  luce++;
  if (luce > 255) {
    luce = 255;
  }
  }
  //Se non schiaccio il pulsante
  else if (digitalRead(buttonPIN)) {
  primo = true;

  if (passo) {
    luce--;
    if (luce < 0) {
      luce = 0;
    }
    passo = false;
  }
  fi = millis();
  durata_button = (unsigned long)(fi - st);
  }

  if (durata_button < 250) {
  fade_time = 0;
  }
  else if (durata_button < 500) {
  fade_time = 5;
  }
  else if (durata_button < 1000) {
  fade_time = 10;
  }
  else if (durata_button < 2000) {
  fade_time = 13;
  }
  else {
  fade_time = 15;
  }

  if ((millis() - ps) > fade_time) {
  passo = true;
  ps = millis();
  }

  if (count > 3) {
  st_p = millis();
  count = 0;
  }

  if ((millis() - st_p) > 100) {
  ferma();
  }
  else {
  bagna();
  }

  //Serial.println(count);
  if (luce == 0) {
  digitalWrite(ledPIN, LOW);
  }
  else {
  analogWrite(ledPIN, luce);
  }
  }

  void bagna() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, HIGH);
  }

  void asciuga() {
  digitalWrite(pumpPIN_A, HIGH);
  digitalWrite(pumpPIN_B, LOW);
  }

  void ferma() {
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);
  }
*/
