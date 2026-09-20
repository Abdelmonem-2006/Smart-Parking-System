#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char *SSID = "Elrefay";
const char *PASS = "5278ghad";

#define LED1 2
#define LED2 4
#define DHT_PIN 5
#define DHT_TYPE DHT11

// MQTT Broker info
const char *MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

// MQTT Topics
const char* topic_pub_temp = "esp32/dht/temp";
const char* topic_pub_hum  = "esp32/dht/humidity";
const char* topic_led1     = "esp32/led1";
const char* topic_led2     = "esp32/led2";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

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

void callback(char topic[], byte message[], int messageLength)
{
  String msg = "";
  for (int i = 0; i < messageLength; i++) {
    msg += (char)message[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == topic_led1) {
    if (msg == "ON")       digitalWrite(LED1, HIGH);
    else if (msg == "OFF") digitalWrite(LED1, LOW);
  }
  else if (String(topic) == topic_led2) {
    if (msg == "ON")       digitalWrite(LED2, HIGH);
    else if (msg == "OFF") digitalWrite(LED2, LOW);
  }
}

void MQT_Reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT Connection......");

    String clientId = "ESP32_Reconnect_Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str()))
    {
      Serial.println("Connected");
      client.subscribe(topic_led1);
      client.subscribe(topic_led2);
      client.publish("esp32/status", "Hello From ESP32");
    }
    else
    {
      Serial.print("failed, rc= ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, LOW);

  Serial.begin(115200);
  dht.begin();

  if (setup_wifi())
  {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
  }
}

void loop() {
  if (!client.connected())
  {
    MQT_Reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // إرسال البيانات إلى الـ Broker
    client.publish(topic_pub_temp, String(t).c_str());
    client.publish(topic_pub_hum, String(h).c_str());

    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" °C | Humidity: ");
    Serial.print(h);
    Serial.println(" %");
  }
}