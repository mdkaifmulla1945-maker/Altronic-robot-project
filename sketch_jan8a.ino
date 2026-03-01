#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// ---------- PIN SETUP ----------
#define IR_LEFT 35
#define IR_RIGHT 34

// Motor A (Right Motor)
#define ENA 25
#define IN1 32
#define IN2 33

// Motor B (Left Motor)
#define ENB 12
#define IN3 26
#define IN4 27

// Servo pins
#define LOWER_SERVO_PIN 21  // Base / downward
#define UPPER_SERVO_PIN 4   // RFID arm / upward

//ultrasonic
#define TRIG_PIN 14
#define ECHO_PIN 13
#define OBSTACLE_DISTANCE 12  // cm threshold
#define CHECK_DISTANCE 12     // cm for secondary scanning


// DC Buzzer
#define BUZZER_PIN 2

// ---------- RFID ----------
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);
//MFRC522::MIFARE_Key keyA;

// ---------- WiFi ----------
const char* ssid = "vivo T1";
const char* password = "charlie1234";
String scriptURL = "https://script.google.com/macros/s/AKfycbz8gmabWaHtjmswZQpPlmEx51EqSS3dW4Y_rxssCGn4e_TQm6Oc_fKWm_6P1Qj2l4evDg/exec";
// kaif wifi password and only if wifi is connected then the bot get start!!!!!!
unsigned long lastScanTime = 0;
const unsigned long scanDelay = 100;

MFRC522::MIFARE_Key keyA;

//speed
int maxSpeed = 120;
int slowSpeed = 80;
int mSpeed = 105;
int junctionCount = 0;
bool junctionLocked = false;

Servo lowerServo;
Servo upperServo;

int LOWER_HOME = 190;
int LOWER_ALIGN = 10;
int UPPER_HOME = 190;
int UPPER_ALIGN = 50;
const int STEPS = 10;
const int STEPS_DELAY = 2;

//obstacle
bool obstacleActive = false;  // starts false, only activates after junction 2
bool obstacleBusy = false;

enum BypassSide { NONE,
                  RIGHT_BYPASS,
                  LEFT_BYPASS };
BypassSide lastBypass = NONE;
// ---------- BUZZER VARIABLES ----------
unsigned long buzzerStart = 0;
unsigned long buzzerDuration = 0;
bool buzzerActive = false;
bool rfidBusy = false;

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lowerServo.attach(LOWER_SERVO_PIN);
  upperServo.attach(UPPER_SERVO_PIN);
  lowerServo.write(LOWER_HOME);
  upperServo.write(UPPER_HOME);
  delay(2000);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  SPI.begin();
  mfrc522.PCD_Init();
  for (int i = 0; i < 6; i++) keyA.keyByte[i] = 0xFF;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  Serial.println("System Ready");
}

// ---------- MAIN LOOP ----------
void loop() {

  int L = digitalRead(IR_LEFT);
  int R = digitalRead(IR_RIGHT);

  if (L == 0 && R == 0) {
    junctionLocked = false;
  }

  // LINE FOLLOW
  if (junctionCount <= 8 && !junctionLocked) {
    if (L == 0 && R == 0) forward();
    else if (L == 1 && R == 0) slightRight();
    else if (L == 0 && R == 1) slightLeft();
  }

  // JUNCTION DETECT
  if (L == 1 && R == 1 && !junctionLocked) {
    junctionLocked = true;

    if (junctionCount == 0) {
      forward();
      delay(600);
      stopBot();
      turnRight90();
      junctionCount++;
    }

    else if (junctionCount == 1) {
      stopBot();
      servoActionStage1();
      servoBack();
      reverseBot();
      delay(400);
      turn180();
      junctionCount++;
    }

    else if (junctionCount == 2) {
      forward();
      delay(500);
      turnRight90();
      forward();
      delay(400);
      obstacleActive = true;
      junctionCount++;
    }
     else if (junctionCount == 3) {
      forward();
      delay(100);
      stopBot();
      
      obstacleActive = false;
      junctionCount++;
    }
  }

  // OBSTACLE
  if (obstacleActive) {
    obstacleCheck();
  }
} 
// ---------- BASIC MOTOR FUNCTIONS ----------

void forward() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed - 10);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void forwardSlow() {
  analogWrite(ENA, mSpeed);
  analogWrite(ENB, mSpeed - 15);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void reverseBot() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopBot() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void slightLeft() {
  analogWrite(ENA, slowSpeed);
  analogWrite(ENB, maxSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void slightRight() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, slowSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void turnRight90() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);  // Right turn → left motor forward, right motor reverse
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(1135);  // adjust if needed
}

//ONLY FOR OBSTACLE
void turnRightOB90() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);  // Right turn → left motor forward, right motor reverse
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(1210);  // adjust if needed
}
void turnLeftOB90() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);  // Right turn → left motor forward, right motor reverse
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(1195);  // adjust if needed
}
void turnLeftOBOB90() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);  // Right turn → left motor forward, right motor reverse
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(1165);  // adjust if needed
}

void turn180() {
  analogWrite(ENA, maxSpeed);
  analogWrite(ENB, maxSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2950);  // full 180 rotation (adjust for your bot)
}



// ---------- SERVO FUNCTIONS  A----------

void smoothMove(int lowerStart, int lowerEnd,
                int upperStart, int upperEnd) {
  for (int i = 0; i <= STEPS; i++) {
    float t = (float)i / STEPS;
    int lowerPos = lowerStart + t * (lowerEnd - lowerStart);
    int upperPos = upperStart + t * (upperEnd - upperStart);
    lowerServo.write(lowerPos);
    upperServo.write(upperPos);
    delay(STEPS_DELAY);
  }
}

void servoActionStage1() {
   rfidBusy = true; // Move both servos together to scan position
  smoothMove(
    LOWER_HOME, LOWER_ALIGN,
    UPPER_HOME, UPPER_ALIGN);
  delay(400);
  // Wait for total 6s, repeatedly scanning RFID
  unsigned long startTime = millis();
  while (millis() - startTime < 9000) {
    scanRFID();  // keeps checking for new card
    delay(50);   // tiny delay so SPI bus can work
  }              // Hold for RFID scan  // Hold for RFID scan
}

void servoBack() {
   rfidBusy = false;
  delay(300);
  smoothMove(
    LOWER_ALIGN, LOWER_HOME,
    UPPER_ALIGN, UPPER_HOME);
}

/// OBSTACLE AVOIDENCE
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}
// ---------------- JUNCTION  I  009----------------
bool isAllBlack() {
  return digitalRead(IR_LEFT) == 1 && digitalRead(IR_RIGHT) == 1;
}

bool obstacleCheck() {
  if (obstacleBusy) return true;
  obstacleBusy = true;

  long frontDist = readDistanceCM();
  if (frontDist <= 0 || frontDist > OBSTACLE_DISTANCE) {
    obstacleBusy = false;
    return false;
  }

  // --- OBSTACLE DETECTED ---   F
  stopBot();
  delay(200);
  reverseBot();
  delay(400);
  stopBot();  // --- FIRST MANEUVER: TURN RIGHT ---
  turnRightOB90();
  forwardSlow();
  delay(2880);

  // --- SECOND MANEUVER: TURN LEFT AND CHECK ---
  turnLeftOB90();
  long leftCheck1 = readDistanceCM();
  if (leftCheck1 > CHECK_DISTANCE) {
    forwardSlow();
    delay(5080);  // move past bypass
  }
  // --- THIRD MANEUVER: TURN LEFT AGAIN AND CHECK ---
  turnLeftOB90();
  long leftCheck2 = readDistanceCM();
  if (leftCheck2 > CHECK_DISTANCE) {
    forwardSlow();
    delay(1000); // move past second bypass check
  }
  // --- OBSTACLE PROCESS COMPLETE ---
  
  junctionLocked = false;  // allow future junctions
  obstacleBusy = false;
  return true;
}


// ---------- RFID FUNCTION K ----------
void scanRFID() {

  if (millis() - lastScanTime < scanDelay) return;

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  lastScanTime = millis();
  // program starts here   KEEP THE CARD AT 3 CM FROM READER
  byte block = 4;
  byte buffer[18];
  byte size = sizeof(buffer);

  if (mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        block,
        &keyA,
        &(mfrc522.uid))
      != MFRC522::STATUS_OK) {
    beepError();
    return;
  }

  if (mfrc522.MIFARE_Read(block, buffer, &size) != MFRC522::STATUS_OK) {
    beepError();
    return;
  }

  String rawData = "";
  for (int i = 0; i < 16; i++) {
    if (buffer[i] > 31 && buffer[i] < 126) rawData += (char)buffer[i];
  }

  rawData.trim();
  Serial.println("RFID: " + rawData);

  int b1 = rawData.indexOf('|');
  int b2 = rawData.indexOf('|', b1 + 1);
  int b3 = rawData.indexOf('|', b2 + 1);

  if (b1 < 0 || b2 < 0 || b3 < 0) {
    beepError();
    return;
  }

  beepCardRead();
  delay(100);

  String product = rawData.substring(0, 4);
  String location = rawData.substring(4, b1);
  String mfg = rawData.substring(b1 + 1, b2);
  String exp = rawData.substring(b2 + 1, b3);
  String count = rawData.substring(b3 + 1);

  String json = "{";
  json += "\"product\":\"" + product + "\",";
  json += "\"location\":\"" + location + "\",";
  json += "\"mfg\":\"" + mfg + "\",";
  json += "\"exp\":\"" + exp + "\",";
  json += "\"count\":\"" + count + "\"}";

  HTTPClient http;
  http.begin(scriptURL);
  http.addHeader("Content-Type", "application/json");
  http.POST(json);
  http.end();

  beepJsonSent();  // TIME IS 5.05 SEC TO JOSAN TO SERVER TO GOOGLE SHEET
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  mfrc522.PCD_Init();
}
// buzzzer function
void beepCardRead() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);  // 1 sec beep
  digitalWrite(BUZZER_PIN, LOW);
}

void beepError() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(4000);  // 4 sec beep
  digitalWrite(BUZZER_PIN, LOW);
}

void beepJsonSent() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);  // 1 sec beep
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);  // short gap
  }
}
