//======================================================================================================================================
//======================================================================================================================================
// GENERAL
#include <ArduinoJson.h>
#include <Adafruit_VL53L0X.h>
#include <FreqCount.h>
#include <EEPROM.h>
String experimentDetector;
String check = "";
String serialReceivedData = "";
bool loopState = true;
bool freqStatus;

enum Experiment {
  NONE,
  MRUA,
  FF,
  MF,
  MD,
  KD,
  TMT
};

Experiment getExperiment(const String& str) {
    if (str == "MRUA") return MRUA;
    if (str == "FF") return FF;
    if (str == "MF") return MF;
    if (str == "MD") return MD;
    if (str == "KD") return KD;
    if (str == "TMT") return TMT;
    return NONE;
}
//======================================================================================================================================
// MRUA
const int trigPin = 9;
const int echoPin = 10;
unsigned long initialTime;
unsigned long currentTime;
unsigned long lastInitCalledTime = 0;
unsigned long lastPrintTime = 0;
long duration;
int distance;
int lastDistance = 0;
bool lastPrintState = true;
//======================================================================================================================================
// FREEFALL
const int bottomSensor = 8;            // Bottom sensor
const int topSensor = 9;               // Top sensor
const float distanceBetween = 0.60;    // Distance between both sensors in meters
const float localGravity = 9.78;
float calculatedGravity;
unsigned long bottomSensorCounter, currentTimeFF, timeInterval;
byte lastBottomSensorState, lastTopSensorState;
float speed;
float error;
//======================================================================================================================================
// MAGNETIC FIELD
const int sensorPin = A3;
const float sensitivity = 0.0014;  // Sensor sensitivity in volts/gauss
const float offsetVoltage = 2.5;  // Offset voltage in volts
float magneticFieldReference;
bool calibrationState = false;
int rawValue;
float outputVoltage;
float magneticField;
int frecuency = 400;
float magneticFieldSum;
const int measurements = 10;
float buzzTime;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
//======================================================================================================================================
// METAL DETECTOR
const int buzzPin = 12;
const int thirteenPin = 13;
int buff = 0;
int frq = 0;
int difference = 0;
int sens = 2; // sensibilidad
int autoBalance = 0;
bool AB = true;
//======================================================================================================================================
// KUNDT'S TUBE
char displayString[17] = "";
double frequency  = 123.456;          
double uncalfreq = 123.456;
float cal = 1.00000f;                 
float readcal = 0.000f;               
unsigned long count = 0;              
int divisor = 256;
//======================================================================================================================================
// TERMOMETER
int TERMISTOR = A0;
int val;
int temp;
//======================================================================================================================================
//======================================================================================================================================
void setup()
{
  // MRUA
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // FREE FALL
  pinMode(bottomSensor, INPUT);
  pinMode(topSensor, INPUT);
  PCICR |= (1 << PCIE0);    // Enable the PCMSK0 scan
  PCMSK0 |= (1 << PCINT0);  // Set pin D8 to trigger a state change
  PCMSK0 |= (1 << PCINT1);  // Set pin D9 to trigger a state change
  // MAGNETIC FIELD: Nothing
  // METAL DETECTOR
  pinMode (buzzPin, OUTPUT);
  pinMode (thirteenPin, OUTPUT);
  // KUNDT'S TUBE
  EEint();
  // TERMOMETER
  pinMode(TERMISTOR, INPUT);
  // GENERAL
  Serial.begin(9600);
  Serial.println("SIRIUS STARTED");
}
//======================================================================================================================================
//======================================================================================================================================
void loop()
{
  // Check the experience to proceed
  while (loopState == true){  
    if (Serial.available() > 0){ 
      check = Serial.readStringUntil('\n');
      check.trim();
      Experiment exp = getExperiment(check);
      switch (exp) {
        case MRUA:
            Serial.println("MRUA");
            loopState = false;
            experimentDetector = "MRUA";
            break;
        case FF:
            Serial.println("FF");
            loopState = false;
            experimentDetector = "FF";
            break;
        case MF:
            Serial.println("MF");
            loopState = false;
            experimentDetector = "MF";
            break;
        case MD:
            Serial.println("MD");
            loopState = false;
            experimentDetector = "MD";
            break;
        case KD:
            Serial.println("KD");
            loopState = false;
            experimentDetector = "KD";
            break;
        case TMT:
            Serial.println("TMT");
            loopState = false;
            experimentDetector = "TMT";
            break;
        default:
            // Handle unrecognized or NONE case
            Serial.println("None");
            break;
      }
    }
  }
  // The specific experience will be shown depending the check
  Experiment currentExp = getExperiment(experimentDetector);
    switch (currentExp) {
      case MRUA:
          while (experimentDetector == "MRUA") {
            executeOperation(readDistance);
          }
          break;
      case FF:
          while (experimentDetector == "FF") {
            executeOperation(calculateGravityAcceleration);
          }
          break;
      case MF:
          while (experimentDetector == "MF") {
            if (calibrationState == false){
              mmDistanceSensorCalibration();
              hallSensorCalibration();
              calibrationState = true;
            }
            executeOperation(readMagneticField);
          }
          break;
      case MD:
          while (experimentDetector == "MD") {
            if (freqStatus == false){
              FreqCount.begin(200);
              frq=FreqCount.read();
              buff=frq;
              freqStatus = true;      
            }
            executeOperation(detectMetal);
          }
          break;
      case KD:
          while (experimentDetector == "KD") {
            if (freqStatus == false){
              FreqCount.begin(1000);
              freqStatus = true;      
            }
            executeOperation(readFrecuencyLevels);
          }
          break;
      case TMT:
          while (experimentDetector == "TMT") {
            executeOperation(readTemperature);
          }
          break;
      default:
          break;
  }
}
//======================================================================================================================================
//======================================================================================================================================
// MRUA
void readDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2;
  if (distance < 2 || distance > 60){
    distance = 0;
  }
  if(distance != lastDistance){
    lastDistance = distance;
  }
  currentTime = millis() - initialTime;
  
  StaticJsonDocument<200> doc;
  JsonObject ultrasonic = doc.createNestedObject("MRUA");
  ultrasonic["time"] = currentTime;
  ultrasonic["distance"] = distance;
  serializeJson(doc, Serial);
  Serial.println();
}
//======================================================================================================================================
// FREEFALL
ISR(PCINT0_vect) {
  // Capture the current counter value in microseconds using the micros function
  currentTimeFF = micros();
  
  if(PINB & B00000001) {  // Check if pin 8 is HIGH
    if(lastBottomSensorState == 0) {                         
      lastBottomSensorState = 1;                            
      bottomSensorCounter = currentTimeFF;                      
    }
  } else if(lastBottomSensorState == 1) {                      
    lastBottomSensorState = 0;                              
  }

  if(PINB & B00000010) {   // Check if pin 9 is HIGH                                              
    if(lastTopSensorState == 0) {                                               
      lastTopSensorState = 1;                                                   
      timeInterval = currentTimeFF - bottomSensorCounter;                                    
    }
  } else if(lastTopSensorState == 1) {                                           
    lastTopSensorState = 0;                                                     
  }
}

void calculateGravityAcceleration() {
  speed = distanceBetween / (timeInterval / 1E6);
  calculatedGravity = (speed * speed) / distanceBetween;
  error = (abs(calculatedGravity - localGravity) / localGravity) * 100.0;

  StaticJsonDocument<200> doc;
  JsonObject gravity = doc.createNestedObject("FF");
  gravity["acceleration"] = calculatedGravity;
  gravity["error"] = error;
  serializeJson(doc, Serial);
  Serial.println();
}
//======================================================================================================================================
// MAGNETIC FIELD
void readMagneticField(){
  rawValue = analogRead(sensorPin);
  outputVoltage = rawValue * (5.0 / 1023.0) - offsetVoltage;
  buzzTime = (100000*(1/outputVoltage));
  magneticField = outputVoltage / sensitivity - magneticFieldReference;
  distance = 0;
  magneticField = round(magneticField * 100.0) / 100.0;

  distance = readMmDistance();
  distance = 0;

  StaticJsonDocument<200> doc;
  JsonObject magneticJson = doc.createNestedObject("MF");
  magneticJson["field"] = magneticField;
  magneticJson["distance"] = distance;
  serializeJson(doc, Serial);
  Serial.println();
  
  if (magneticField > 2){
    digitalWrite(buzzPin, HIGH);         
    delay(buzzTime);
    digitalWrite(buzzPin, LOW);
  }
}

double readMmDistance(){
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // Pass in 'true' to get debug data printout

  if (measure.RangeStatus != 4){  // Phase failures have incorrect data
    distance = measure.RangeMilliMeter;
  } 
  else {
    distance = 0;
  }
  return distance;
}

void mmDistanceSensorCalibration(){
  if (!lox.begin()) {
    Serial.println("ERROR: VL53L0X SENSOR");
  }
}

void hallSensorCalibration(){
  // Takes several initial measurements and calculates the average to use as a reference
  magneticFieldSum = 0;
  for(int i = 0; i < measurements; i++) {
    int rawValue = analogRead(sensorPin);
    float voltage = rawValue * (5.0 / 1023.0) - offsetVoltage;
    magneticFieldSum += voltage / sensitivity;
    delay(100);
  }
  magneticFieldReference = magneticFieldSum / measurements;
}
//======================================================================================================================================
// METAL DETECTOR
void detectMetal(){
  AB=true;
  frq = FreqCount.read();
  difference = buff-frq;

  if(difference>sens){               // Ferrous metal
    for(int i=0; i<10; i++){  // Generate tone
      digitalWrite(buzzPin,HIGH);
      delay(2);
      digitalWrite(buzzPin,LOW);
      delay(2);
    }
    delay(40-(constrain(difference*5,10,40)));
    AB=false;   // Reset Autobalance
    StaticJsonDocument<200> doc;
    JsonObject MDJson = doc.createNestedObject("MD");
    MDJson["isFerrous"] = "YES";
    serializeJson(doc, Serial);
  }
  
  else if(difference<-sens){        // Non-Ferrous metal
    difference =- difference;
    for(int i=0; i<20; i++){    // Generate tone
      digitalWrite(buzzPin, HIGH);
      delay(1);
      digitalWrite(buzzPin, LOW);
      delay(1);
    }
    delay(40-(constrain(difference*5,10,40))); 
    AB = false;   // Reset Autobalance
    StaticJsonDocument<200> doc;
    JsonObject MDJson = doc.createNestedObject("MD");
    MDJson["isFerrous"] = "NO";
    serializeJson(doc, Serial);
  }

  if (true){                     // Autobalance
    digitalWrite(thirteenPin,HIGH);
    if (AB && difference != 0){   
      if (autoBalance>1000){
        autoBalance = 0; 
        buff = frq;
      }
      autoBalance++;
    }
    else autoBalance = 0;
    delay(1);
  }
}
//======================================================================================================================================
// KUNDT'S TUBE
void readFrecuencyLevels(){
  if (FreqCount.available()){
      count = FreqCount.read();
      count = count * divisor;
      uncalfreq = count / 1000000.0;
      frequency = uncalfreq * cal;
  }
  updateFrecuencyValues();
}

void updateFrecuencyValues(){
  dtostrf(frequency, 7, 3, displayString);
  StaticJsonDocument<200> doc;
  JsonObject frecuencyJson = doc.createNestedObject("KD");
  frecuencyJson["frecuency"] = displayString;
  serializeJson(doc, Serial);
  Serial.println();
}

void EEint(){
  byte value = EEPROM.read(0);             
  if (value == 0x55) {                     
     EEPROM.get(1, readcal);               
     cal = readcal;                        
     delay(2000);
     }
  else {                                   // initialize EEPROM at first time use
    EEPROM.write(0, 0x55);                 // write flag
    EEPROM.put(1, cal);                     // write initial cal value
    delay(2000);      
  }
}
//======================================================================================================================================
// TERMOMETER
void readTemperature(){
  val = analogRead(TERMISTOR);
  temp = Thermister(val);
  StaticJsonDocument<200> doc;
  JsonObject TMTJson = doc.createNestedObject("TMT");
  TMTJson["temperature"] = temp;
  serializeJson(doc, Serial);
  Serial.println();
}

double Thermister(int RawADC) {
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;
  return Temp;
}
//======================================================================================================================================
// GENERAL
void executeOperation(void (*experimentFn)()){
  if (Serial.available() > 0) {
    serialReceivedData = Serial.readStringUntil('\n');
    serialReceivedData.trim();
    if (serialReceivedData == "INIT"){
      lastPrintState = true;
      initialTime = millis();      
      while (lastPrintState == true){ 
        if ((millis() - lastPrintTime) > 250) {
          experimentFn();
          lastPrintTime = millis();
        }
        if (Serial.available() > 0){
          serialReceivedData = Serial.readStringUntil('\n');
          serialReceivedData.trim();
          if (serialReceivedData == "PAUSE"){
            lastPrintState = false;
            break;
          } 
          else if (serialReceivedData == "ESC"){
            lastPrintState = false;
            experimentDetector = "null";
            loopState = true;
            break;
          }
        }    
      }
    }
    else if (serialReceivedData == "ESC"){
      freqStatus = false;
      experimentDetector = "null";
      loopState = true;
    } 
  }
}
//======================================================================================================================================
//======================================================================================================================================