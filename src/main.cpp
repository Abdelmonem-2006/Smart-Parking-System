#define BLYNK_TEMPLATE_ID "TMPL2JrWYosa2"
#define BLYNK_TEMPLATE_NAME "Smart System"
#define BLYNK_AUTH_TOKEN "C7lb344AgI---FR1btucjAeTWQfY-nLB"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

const char *SSID = "Abdelmonem";
const char *PASS = "5278ghad";



#define LED1 2
#define LED2 4
#define DHT_PIN 15
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

BlynkTimer timer;

bool setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

void SendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    digitalWrite(LED1, HIGH);
  } else {
    digitalWrite(LED1, LOW);
  }
}
BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  if (pinValue == 0) {
    digitalWrite(LED2, HIGH);
  } else {
    digitalWrite(LED2, LOW);
  }
}


void setup() {
  pinMode(LED1, OUTPUT);
  
  pinMode(LED2, OUTPUT);

  dht.begin();
  

  Serial.begin(115200);

  setup_wifi();

  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PASS);

  timer.setInterval(2000, SendSensorData);
  
}

void loop() {
  Blynk.run();
  timer.run();
}