#define BLYNK_TEMPLATE_ID "TMPL2JrWYosa2"
#define BLYNK_TEMPLATE_NAME "Smart System"
#define BLYNK_AUTH_TOKEN "C7lb344AgI---FR1btucjAeTWQfY-nLB"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define TRIG1 13
#define ECHO1 12

#define TRIG2 14
#define ECHO2 27

#define TRIG3 26
#define ECHO3 25

#define TRIG4 4
#define ECHO4 34

#define LED_FULL 19
#define SERVO_PIN 15
#define BUZZER_PIN 23

Servo barrierGate;
BlynkTimer timer;

bool slot1Occupied = false;
bool slot2Occupied = false;
bool slot3Occupied = false;

int availableSlots = 3;

const int THRESHOLD_DISTANCE = 15;
const int GATE_THRESHOLD = 20;

bool gateOpen = false;

unsigned long lastGateDetection = 0;

const unsigned long NO_OBJECT_TIME = 5000;

const int GATE_CLOSED = 0;
const int GATE_OPEN = 90;

void checkParkingSlots();
void checkGateSensor();
void updateBlynk();
void updateLED();
void openGate();
void closeGate();
void alertParkingFull();
long getDistance(int trigPin, int echoPin);

void setup() {

  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  pinMode(TRIG4, OUTPUT);
  pinMode(ECHO4, INPUT);

  pinMode(LED_FULL, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  barrierGate.attach(SERVO_PIN);
  barrierGate.write(GATE_CLOSED);

  Serial.println("Connecting to Blynk...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  timer.setInterval(1000L, updateBlynk);

  Serial.println("Smart Parking System Ready");
}

void loop() {

  Blynk.run();
  timer.run();

  checkParkingSlots();
  updateLED();
  checkGateSensor();

  delay(100);
}

long getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration =
    pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
    return 0;

  long distance =
    duration * 0.034 / 2;

  return distance;
}

void checkParkingSlots() {

  long distance1 =
    getDistance(TRIG1, ECHO1);

  long distance2 =
    getDistance(TRIG2, ECHO2);

  long distance3 =
    getDistance(TRIG3, ECHO3);

  slot1Occupied =
    distance1 > 0 &&
    distance1 < THRESHOLD_DISTANCE;

  slot2Occupied =
    distance2 > 0 &&
    distance2 < THRESHOLD_DISTANCE;

  slot3Occupied =
    distance3 > 0 &&
    distance3 < THRESHOLD_DISTANCE;

  availableSlots = 0;

  if (!slot1Occupied)
    availableSlots++;

  if (!slot2Occupied)
    availableSlots++;

  if (!slot3Occupied)
    availableSlots++;
}

void checkGateSensor() {

  long gateDistance =
    getDistance(TRIG4, ECHO4);

  bool objectDetected =
    gateDistance > 0 &&
    gateDistance < GATE_THRESHOLD;

  if (objectDetected) {

    lastGateDetection = millis();

    Serial.print("Gate Object Detected: ");
    Serial.print(gateDistance);
    Serial.println(" cm");

    if (!gateOpen) {

      if (availableSlots > 0) {

        Serial.println("Opening Gate");

        openGate();

        digitalWrite(
          BUZZER_PIN,
          HIGH
        );

        delay(200);

        digitalWrite(
          BUZZER_PIN,
          LOW
        );

      } else {

        Serial.println("Parking FULL");

        alertParkingFull();
      }
    }
  }

  if (gateOpen &&
      !objectDetected &&
      millis() - lastGateDetection >= NO_OBJECT_TIME) {

    Serial.println("Closing Gate");

    closeGate();
  }
}

void openGate() {

  if (!gateOpen) {

    barrierGate.write(GATE_OPEN);

    gateOpen = true;

    lastGateDetection = millis();
  }
}

void closeGate() {

  if (gateOpen) {

    barrierGate.write(GATE_CLOSED);

    gateOpen = false;
  }
}

void updateLED() {

  digitalWrite(
    LED_FULL,
    availableSlots == 0
  );
}

void alertParkingFull() {

  static unsigned long lastAlert = 0;

  if (millis() - lastAlert >= 2000) {

    Serial.println("PARKING FULL");

    for (int i = 0; i < 2; i++) {

      digitalWrite(
        LED_FULL,
        HIGH
      );

      digitalWrite(
        BUZZER_PIN,
        HIGH
      );

      delay(200);

      digitalWrite(
        LED_FULL,
        LOW
      );

      digitalWrite(
        BUZZER_PIN,
        LOW
      );

      delay(200);
    }

    digitalWrite(
      LED_FULL,
      HIGH
    );

    lastAlert = millis();
  }
}

void updateBlynk() {

  int slot1Status = slot1Occupied ? 0 : 1;
  int slot2Status = slot2Occupied ? 0 : 1;
  int slot3Status = slot3Occupied ? 0 : 1;

  Blynk.virtualWrite(V0, slot1Status);
  Blynk.virtualWrite(V1, slot2Status);
  Blynk.virtualWrite(V2, slot3Status);
  Blynk.virtualWrite(V3, availableSlots);

  if (availableSlots == 0) {

    Blynk.virtualWrite(
      V5,
      "PARKING FULL - ENTRY DENIED"
    );

  } else {

    String status =
      "AVAILABLE SLOTS: " +
      String(availableSlots) +
      "/3";

    Blynk.virtualWrite(
      V5,
      status
    );
  }
}