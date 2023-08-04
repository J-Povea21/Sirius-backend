#include <ArduinoJson.h>

const int trigPin = 9;
const int echoPin = 10;
const int buttonPin = 8;

String data = "";
unsigned long initialTime;
unsigned long currentTime;
unsigned long lastInitCalledTime = 0;
unsigned long lastPrintTime = 0;
long duration;
int distance;
int lastDistance = 0;
bool lastPrintState = true;

void setup() 
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() 
{
  if (Serial.available() > 0) 
  {
    data = Serial.readStringUntil('\n'); // Leer los datos entrantes y almacenarlos en 'data'
    data.trim(); // Remover espacios en blanco o saltos de línea
    
    if (data == "INIT")
    { 
      lastPrintState = true;
      initialTime = millis();      
      while (lastPrintState == true)
      { 
        if ((millis() - lastPrintTime) > 250) 
          {
            read_distance();
            lastPrintTime = millis();
          }
        if (Serial.available() > 0) // Verificar si hay datos disponibles
        {
          data = Serial.readStringUntil('\n'); // Leer los datos entrantes y almacenarlos en 'data'
          data.trim(); // Remover espacios en blanco o saltos de línea
          if (data == "PAUSE") {
            lastPrintState = false;
            break; }
        }    
      }
    } 
  }
}

void read_distance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2;

  if (distance < 2 || distance > 50)
  {
    return;
  }

  if(distance != lastDistance)
  {
    lastDistance = distance;
  }

  currentTime = millis() - initialTime;
  StaticJsonDocument<200> doc;
  
  JsonObject ultrasonic = doc.createNestedObject("ULTRASONIC");
  ultrasonic["time"] = currentTime;
  ultrasonic["distance"] = distance;
  serializeJson(doc, Serial);
  Serial.write('\n');
}