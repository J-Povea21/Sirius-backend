//======================================================================================================================================
//======================================================================================================================================
// GENERAL
#include <ArduinoJson.h>
#include <Adafruit_VL53L0X.h>
#include <FreqCount.h>
String experimentDetector, check = "", serialReceivedData = ""; // Checking controllers
bool freqStatus, loopState = true, calibrationState = false;    // Status controllers
const int numMeasurements = 120;    // Calibration measurements number

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
// DELAY CONTROLLER
const int delayMRUA = 250, delayFF = 10, delayMF = 250, delayMD = 250, delayKD = 10, delayTMT = 250;
//======================================================================================================================================
// MRUA
const int trigPin = 10, echoPin = 11;
unsigned long initialTime, duration, lastInitCalledTime = 0, lastPrintTime = 0;
double currentTime, distance, lastDistance = 0.0, lastMRUATime = 0.0, lastSpeed = 0.0, aceleration = 0.0;
float distanceMF;
bool lastPrintState = true;
//======================================================================================================================================
// FREEFALL
const int bottomSensor = 8, topSensor = 9;             
const float distanceBetween = 0.50, localGravity = 9.78;
double speed, error, calculatedGravity;
unsigned long topSensorCounter, currentTimeFF, timeInterval;
byte lastBottomSensorState, lastTopSensorState;
//======================================================================================================================================
// MAGNETIC FIELD
const int sensorPin = A0;
const float sensitivity = 0.0014, offsetVoltage = 2.5;
int rawValue, frecuency = 400;
float magneticFieldReference, outputVoltage, magneticField, magneticFieldSum;
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
//======================================================================================================================================
// METAL DETECTOR
const int buzzPin = 12, thirteenPin = 13;
int buff = 0, frq = 0, difference = 0, sens = 2, autoBalance = 0;
bool AB = true;
//======================================================================================================================================
// KUNDT'S TUBE
const int soundSensor = A0;
const unsigned long interval = 1000000;
int currentValue, nodeValue = 0, lastValue = 0, frecuencySum = 0;           
unsigned long currentTimeKD = micros(), frecuencyKD = 0, lastTime = 0;
//======================================================================================================================================
// THERMOMETER
const int termistor = A0;
int val, temp;
//======================================================================================================================================
//======================================================================================================================================
void setup()
{
  // MRUA
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // FREE FALL
  pinMode(bottomSensor, INPUT);     // Fotorresistor sensor Dpin 
  pinMode(topSensor, INPUT);        // Fotorresistor sensor Dpin 
  PCICR |= (1 << PCIE0);            // Enable the PCMSK0 scan
  PCMSK0 |= (1 << PCINT0);          // Set pin 8 to trigger a state change
  PCMSK0 |= (1 << PCINT1);          // Set pin 9 to trigger a state change
  // MAGNETIC FIELD: Nothing
  // METAL DETECTOR
  pinMode (buzzPin, OUTPUT);        // Buzzer Dpin
  pinMode (thirteenPin, OUTPUT);    // MD Dpin
  // KUNDT'S TUBE
  lastTime = micros();              // Inicial relative time
  // THERMOMETER
  pinMode(termistor, INPUT);        // Thermometer's Apin
  // GENERAL
  Serial.begin(9600);
  Serial.println("SIRIUS STARTED"); // Signal SIRIUS system inicialization
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
          executeOperation(readDistance, delayMRUA);
        }
        break;
    case FF:
        while (experimentDetector == "FF") {
          executeOperation(calculateGravityAcceleration, delayFF);
        }
        break;
    case MF:
        while (experimentDetector == "MF") {
          if (calibrationState == false){
            mmDistanceSensorCalibration();
            hallSensorCalibration();
            calibrationState = true;
          }
          executeOperation(readMagneticField, delayMF);
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
          executeOperation(detectMetal, delayMD);
        }
        break;
    case KD:
        while (experimentDetector == "KD") {
          if (calibrationState == false){
            soundSensorCalibration();
            calibrationState = true;      
          }
          executeOperation(readFrecuencyLevels, delayKD);
        }
        break;
    case TMT:
        while (experimentDetector == "TMT") {
          executeOperation(readTemperature, delayTMT);
        }
        break;
    default:
        break;
  }
}
//======================================================================================================================================
//======================================================================================================================================
// MRUA
void readDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

  if (distance > 65.0 || distance < 2.0){
    distance = 0.0;
  } else if (abs(distance - lastDistance) < 0.30){
    distance = lastDistance;
  }

  currentTime = millis() - initialTime;
  currentTime /= 1000.0; // ms to seg
  speed = (distance - lastDistance) / (currentTime - lastMRUATime); // Speed in cm/s
  aceleration = (speed - lastSpeed) / (currentTime - lastMRUATime); // Acceleration in cm/s^2

  lastTime = currentTime;
  lastSpeed = speed;
  if(distance != lastDistance){
    lastDistance = distance;
  }

  StaticJsonDocument<200> doc;
  JsonObject ultrasonic = doc.createNestedObject("MRUA");
  ultrasonic["time"] = currentTime;
  ultrasonic["distance"] = distance;
  ultrasonic["speed"] = speed;
  ultrasonic["aceleration"] = aceleration;
  serializeJson(doc, Serial);
  Serial.println();
}
//======================================================================================================================================
// FREEFALL
ISR(PCINT0_vect){
  // Save the current counter value
  currentTimeFF = micros();
  
  if(PINB & B00000001) {  // Check if D8 is HIGH
    if(lastBottomSensorState == 0) {                         
      lastBottomSensorState = 1;
      timeInterval = currentTimeFF - topSensorCounter;

      calculatedGravity = 2 * (0.5 - 0.938 * (timeInterval / 1E6)) / ((timeInterval / 1E6) * (timeInterval / 1E6));
      error = (abs(calculatedGravity - localGravity) / localGravity) * 100.0;
      StaticJsonDocument<200> doc;
      JsonObject gravity = doc.createNestedObject("FF");
      gravity["acceleration"] = calculatedGravity;
      gravity["error"] = error;
      serializeJson(doc, Serial);
      Serial.println();                                                    
    }
  } else if(lastBottomSensorState == 1) {                      
    lastBottomSensorState = 0;                              
  }

  if(PINB & B00000010) {   // Check if D9 is HIGH                                             
    if(lastTopSensorState == 0) {                                               
      lastTopSensorState = 1;                                                   
      topSensorCounter = currentTimeFF;               
    }
  } else if(lastTopSensorState == 1) {                                           
    lastTopSensorState = 0;                                                     
  }
}

void calculateGravityAcceleration() {
  asm volatile ("nop"); // Thanks to ISR function, the main FF function do not need a operation 
}
//======================================================================================================================================
// MAGNETIC FIELD
void readMagneticField(){
  rawValue = analogRead(sensorPin);
  outputVoltage = rawValue * (5.0 / 1023.0) - offsetVoltage;
  magneticField = outputVoltage / sensitivity - magneticFieldReference;
  magneticField = abs(round(magneticField * 100.0) / 100.0);

  distance = readMmDistance();
  currentTime = millis() - initialTime;
  currentTime /= 1000.0;

  StaticJsonDocument<200> doc;
  JsonObject magneticJson = doc.createNestedObject("MF");
  magneticJson["field"] = magneticField;
  magneticJson["distance"] = distance;
  magneticJson["time"] = currentTime;
  serializeJson(doc, Serial);
  Serial.println();
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
  for(int i=0; i<numMeasurements; i++) {
    int rawValue = analogRead(sensorPin);
    float voltage = rawValue * (5.0 / 1023.0) - offsetVoltage;
    magneticFieldSum += voltage / sensitivity;
    delay(10); // Small pause between readings
  }
  magneticFieldReference = magneticFieldSum / numMeasurements;
}
//======================================================================================================================================
// METAL DETECTOR
void detectMetal(){
  AB=true;
  frq = FreqCount.read();
  difference = buff - frq;

  if(difference > sens){               // Ferrous metal
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
  
  else if(difference <- sens){        // Non-Ferrous metal
    difference =- difference;
    for(int i=0; i<20; i++){    // Generate tone
      digitalWrite(buzzPin, HIGH);
      delay(1);
      digitalWrite(buzzPin, LOW);
      delay(1);
    }
    delay(40-(constrain(difference * 5, 10, 40))); 
    AB = false;   // Reset Autobalance
    StaticJsonDocument<200> doc;
    JsonObject MDJson = doc.createNestedObject("MD");
    MDJson["isFerrous"] = "NO";
    serializeJson(doc, Serial);
  }

  if (true){                     // Autobalance
    digitalWrite(thirteenPin, HIGH);
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
void soundSensorCalibration(){
  // Takes several initial measurements and calculates the average to use as a reference
  for(int i=0; i<numMeasurements; i++) {
    frecuencySum += analogRead(soundSensor);
    delay(10); // Small pause between readings
  }
  nodeValue = frecuencySum / numMeasurements; // Average value
}

void readFrecuencyLevels(){
  currentValue = analogRead(soundSensor);
  currentTimeKD = micros();

  // Detect a node crossing
  if((currentValue < nodeValue && lastValue >= nodeValue) || (lastValue < nodeValue && currentValue >= nodeValue)) {
    frecuencyKD++;
  }
  lastValue = currentValue;

  if(currentTimeKD - lastTime >= interval) {
    frecuencyKD /= 2; // Divide by 2 since two node crossings are detected
    StaticJsonDocument<200> doc;
    JsonObject frecuencyJson = doc.createNestedObject("KD");
    frecuencyJson["frecuency"] = frecuencyKD;
    serializeJson(doc, Serial);
    Serial.println();

    frecuencyKD = 0;
    lastTime = currentTimeKD;
  }
}
//======================================================================================================================================
// THERMOMETER
void readTemperature(){
  val = analogRead(termistor);
  temp = thermister(val);
  StaticJsonDocument<200> doc;
  JsonObject TMTJson = doc.createNestedObject("TMT");
  TMTJson["temperature"] = temp;
  serializeJson(doc, Serial);
  Serial.println();
}

double thermister(int RawADC) {
  double Temp;
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp - 273.15;
  return Temp;
}
//======================================================================================================================================
// GENERAL
void executeOperation(void (*experimentFn)(), int delayTime){
  if (Serial.available() > 0) {
    serialReceivedData = Serial.readStringUntil('\n');
    serialReceivedData.trim();
    if (serialReceivedData == "INIT"){
      lastPrintState = true;
      initialTime = millis();      
      while (lastPrintState == true){ 
        if ((millis() - lastPrintTime) > delayTime) {
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
            calibrationState = false;
            lastPrintState = false;
            experimentDetector = "null";
            loopState = true;
            break;
          }
        }    
      }
    }
    else if (serialReceivedData == "ESC"){
      calibrationState = false;
      freqStatus = false;
      experimentDetector = "null";
      loopState = true;
    } 
  }
}
//======================================================================================================================================
//======================================================================================================================================
