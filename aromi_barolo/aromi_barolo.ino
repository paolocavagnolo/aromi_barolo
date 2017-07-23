#define buttonPIN 7
#define pumpPIN_A 5
#define pumpPIN_B 6
#define fanPIN 3
#define ledPIN 9

bool pump_mode = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPIN, INPUT_PULLUP);
  pinMode(pumpPIN_A, OUTPUT);
  pinMode(pumpPIN_B, OUTPUT);
  pinMode(fanPIN, OUTPUT);
  pinMode(ledPIN, OUTPUT);

  digitalWrite(ledPIN, LOW);
  digitalWrite(fanPIN, LOW);
  digitalWrite(pumpPIN_A, LOW);
  digitalWrite(pumpPIN_B, LOW);

  if (!digitalRead(buttonPIN)) {
    pump_mode = false;
  }
  else if (digitalRead(buttonPIN)) {
    pump_mode = false;
  }

}

bool primo = true;
long st = 0;
long st_p = 0;
long fi = 0;
bool pump = true;
long durata_button = 0;
long count = 0;
int luce = 255;
bool passo = true;
long ps = 0;

int fade_time = 15;

void loop() {
  // put your main code here, to run repeatedly:
  while(pump_mode) {
    if (!digitalRead(buttonPIN)) {
      bagna();
    }
    else if (digitalRead(buttonPIN)) {
      asciuga();
    }
  }

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

