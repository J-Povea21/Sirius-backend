//======================================================================================================================================
//======================================================================================================================================
// GENERAL
#include <ArduinoJson.h>
#include <Adafruit_VL53L0X.h>
#include <FreqCount.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
String experimentDetector, check = "", serialReceivedData = "";   // Checking controllers
bool freqStatus, loopState = true, startupState = false;          // Status controllers
const int numMeasurements = 120;                                  // Calibration measurements number
LiquidCrystal_I2C lcd(0x27, 20, 4);                               // 20x4 LCD configuration

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
const int delayMRUA = 250, delayFF = 10, delayMF = 250, delayMD = 250, delayKD = 0, delayTMT = 250;
const long intervalTone = 10;
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
int attemptCounter;
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
const int buzzPin = 12;
unsigned long previousTimeTone = 0;
int baseFrecuency = 0, frecuencyMD = 0, sensivityMD = 2, difference = 0,  autoBalance = 0, toneCount = 0;
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
  // KUNDT'S TUBE
  lastTime = micros();              // Inicial relative time
  // THERMOMETER
  pinMode(termistor, INPUT);        // Thermometer's Apin
  // GENERAL
  systemInitConfig();               // SIRIUS system initialization config
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
            attemptCounter = 0;
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
          if (startupState == false){
            lcdShow(6, 1, "Uniform", true);
            lcdShow(0, 2, "Rectilinear Movement", false);
            startupState = true;
          }
          executeOperation(readDistance, delayMRUA);
        }
        break;
    case FF:
        while (experimentDetector == "FF") {
          if (startupState == false){
            lcdShow(6, 1, "Freefall", true);
            startupState = true;
          }
          executeOperation(calculateGravityAcceleration, delayFF);
        }
        break;
    case MF:
        while (experimentDetector == "MF") {
          if (startupState == false){
            mmDistanceSensorCalibration();
            hallSensorCalibration();
            lcdShow(3, 1, "Magnetic Field", true);
            startupState = true;
          }
          executeOperation(readMagneticField, delayMF);
        }
        break;
    case MD:
        while (experimentDetector == "MD") {
          if (startupState == false){
            setupMD();
            lcdShow(3, 1, "Metal Detector", true);
            freqStatus = true;
          }
          executeOperation(detectMetal, delayMD);
        }
        break;
    case KD:
        while (experimentDetector == "KD") {
          if (startupState == false){
            soundSensorCalibration();
            lcdShow(4, 1, "Kundts Tube", true);
            startupState = true;
          }
          executeOperation(readFrecuencyLevels, delayKD);
        }
        break;
    case TMT:
        while (experimentDetector == "TMT") {
          if (startupState == false){
            lcdShow(5, 1, "Termometer", true);
            startupState = true;
          }
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
      attemptCounter += 1;

      calculatedGravity = 2 * (0.5 - 0.938 * (timeInterval / 1E6)) / ((timeInterval / 1E6) * (timeInterval / 1E6));
      error = (abs(calculatedGravity - localGravity) / localGravity) * 100.0;
      StaticJsonDocument<200> doc;
      JsonObject gravity = doc.createNestedObject("FF");
      gravity["acceleration"] = calculatedGravity;
      gravity["error"] = error;
      gravity["attempt"] = attemptCounter;
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
  } else {
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
void setupMD(){
  FreqCount.begin(200);
  frecuencyMD = FreqCount.read();
  baseFrecuency = frecuencyMD;

  for(int i=0; i<5; i++){
    generateTone(buzzPin, 2, 10);
    delay(20);
    frecuencyMD = FreqCount.read();
    if(frecuencyMD != baseFrecuency){
      baseFrecuency = frecuencyMD;
      i = 0;
    }
  }
  generateTone(buzzPin, 1, 20);
}

void detectMetal(){
  frecuencyMD = FreqCount.read();
  difference = baseFrecuency - frecuencyMD;
  // Ferrous metal
  if(difference > sensivityMD){
    generateTone(buzzPin, 2, 10);
    delay(40-(constrain(difference*5,10,40)));
    StaticJsonDocument<200> doc;
    JsonObject MDJson = doc.createNestedObject("MD");
    MDJson["isFerrous"] = "1";
    serializeJson(doc, Serial);
    Serial.println();
  }
  // Non-Ferrous metal
  else if(difference <- sensivityMD){
    difference = -difference;
    generateTone(buzzPin, 1, 20);
    delay(40-(constrain(difference * 5, 10, 40)));
    StaticJsonDocument<200> doc;
    JsonObject MDJson = doc.createNestedObject("MD");
    MDJson["isFerrous"] = "0";
    serializeJson(doc, Serial);
    Serial.println();
  }
  frecuencyMD = 0;
}

void generateTone(int pin, int duration, int repetitions){
  for(int i=0; i<repetitions; i++) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
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
    frecuencyJson["amplitude"] = currentValue;
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
  currentTime = millis() - initialTime;
  currentTime /= 1000.0; // ms to seg

  StaticJsonDocument<200> doc;
  JsonObject TMTJson = doc.createNestedObject("TMT");
  TMTJson["temperature"] = temp;
  TMTJson["time"] = currentTime;
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
          } else if (serialReceivedData == "ESC"){
            startupState = false;
            lastPrintState = false;
            experimentDetector = "null";
            loopState = true;
            lcdStartupConfig();
            break;
          }
        }    
      }
    } else if (serialReceivedData == "ESC"){
      startupState = false;
      experimentDetector = "null";
      loopState = true;
      lcdStartupConfig();
    } 
  }
}

void systemInitConfig(){
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  Serial.println("SIRIUS STARTED");           // Software signal SIRIUS system inicialization
  lcdShow(3, 1, "SIRIUS STARTED", true);      // LCD signal SIRIUS system inicialization
  delay(4000);
  lcdStartupConfig();
}

void lcdStartupConfig(){
  lcdShow(6, 0, "Waiting", true);
  lcdShow(9, 1, "for", false);
  lcdShow(5, 2, "Experience", false);
}

void lcdShow(int x, int y, String text, bool clearState){
  if (clearState){ lcd.clear(); }
  lcd.setCursor(x, y);
  lcd.print(text);
}
//======================================================================================================================================
