#include <DHT.h>
#include <Ultrasonic.h>
#include <ESP32Servo.h>


//-------------------INICIO PINAGEM INPUT-------------------------

// Sensor de Presenca
#define motionSensorPin   35

// Potenciometro
#define potenciometerPin  15 

// Sensor de Iluminacao(LDR)
#define photoSensorPin    2
const float RESISTOR_FIXO = 1000;
const float GAMMA = 0.7;
const float RL10 = 50;

// Sensor de Temperatura e Humidade
#define DHTPIN            45
#define DHTTYPE        DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensor Ultrasonico
#define pino_trigger 4
#define pino_echo 5
Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Botao de Interrupcao
#define buttonPin  23
volatile bool buttonState = false;
volatile unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;
void IRAM_ATTR handleButtonISR() {
  unsigned long now = millis();
  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    // le a borda
    if (digitalRead(buttonPin) == LOW) {
      // alterna estado lógico (isso apenas atualiza flag; o envio ao Cloud acontece no loop)
      buttonState = !buttonState;
    }
    lastDebounceTime = now;
  }
}

//-------------------FIM PINAGEM INPUT-------------------------

//-----------------INICIO PINAGEM OUTPUT-----------------------

// Servo
#define servoPin   33
Servo servo;

//buzzer
#define buzzerPin   1

//------------------FIM PINAGEM OUTPUT------------------------

// ----------------- SETUP -------------------
void setup() {

  //--------------INPUT--------------//
  pinMode(motionSensorPin,  INPUT);
  pinMode(photoSensorPin,   INPUT);
  pinMode(potenciometerPin, INPUT);
  dht.begin();
  
  //-----------INPUT-PULLUP-----------//
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonISR, FALLING);

  //--------------OUTPUT--------------//
  pinMode(buzzerPin, OUTPUT);
  servo.attach(servoPin);

}

// ----------------- LOOP -------------------
void loop() {
  // put your main code here, to run repeatedly:

  // Obter Dados Sensor de Iluminacao(LDR)
  // @param int pin
  int fotoValue = analogRead(photoSensorPin);
  float fotoVoltage = fotoValue * 3.3 / 4095.0;
  if (fotoVoltage >= 3.29) fotoVoltage = 3.29;
  float fotoResistance = RESISTOR_FIXO * fotoVoltage / (3.3 - fotoVoltage);
  if (fotoResistance <= 0) fotoResistance = 1;
  float lux = pow((RL10 * 1000 * pow(10, GAMMA)) / fotoResistance, 1.0 / GAMMA);

  // Obter Dados Sensor Ultrasonico
  // @param int unitDistanceMeasurement
  float distanciaCM = ultrasonic.read(CM);

  // Obter Dados Sensor Temperatura e Humidade
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // Obter Dados Sensor Presenca
  // @param int pin
  int motionSensorState = digitalRead(motionSensorPin);
  
  // Obter Dados do Potenciometro
  // @param int pin
  int potenciometerValue = analogRead(potenciometerPin);

  //-----------------------------------------------------
  
  // Saida do Servo
  // Rotaciona ate um determinado angulo
  // @param int angle
  servo.write(90);

  // Saida do Buzzer
  // ligar buzzer
  // @param int pin, int frequence   
  tone(buzzerPin, 100);
  

}
