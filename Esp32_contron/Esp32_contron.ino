#include <WiFi.h>           // เปลี่ยนจาก ESP8266WiFi.h เป็น WiFi.h
#include <PubSubClient.h>

// ------------------ WiFi ------------------
const char* ssid = "Grissana";
const char* password = "00000000";

// ------------------ MQTT ------------------
const char* mqtt_server = "broker.netpie.io";
// const char* mqtt_server = "broker.mqtt-dashboard.com"; // ใช้ได้ถ้าไม่ใช้ NETPIE
const int mqtt_port = 1883;
const char* mqtt_client = "12a2c927-a8a7-4199-a744-2e3136b3b62a";
const char* mqtt_username = "sfV5UX4RL9PQUpXiRhQehhMaaJqwLsEu";
const char* mqtt_password = "KkDfYhbH24E29CnqN8jmb7YAtTNv4pDU";

WiFiClient espClient;
PubSubClient client(espClient);

int LED1mcu2;
char msg[100];

// ------------------ MQTT Reconnect ------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      client.subscribe("LED1mcu2status");
      client.subscribe("@msg/sw1MCU1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds...");
      delay(5000);
    }
  }
}

// ------------------ MQTT Callback ------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(msg);

  if (String(topic) == "@msg/sw1MCU1") {
    if (msg == "1") {
      digitalWrite(2, HIGH); // GPIO2 แทน D0 ของ ESP8266
      Serial.println("Turn on the LED1");
      LED1mcu2 = 1;
    } else if (msg == "0") {
      digitalWrite(2, LOW);
      Serial.println("Turn off the LED1");
      LED1mcu2 = 0;
    }
  }
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // GPIO2 แทน D0

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ------------------ Loop ------------------
void loop() {
  if (!client.connected()) reconnect();

  String data = "{\"data\":{\"LED1mcu2status\":" + String(LED1mcu2) + "}}";
  data.toCharArray(msg, data.length() + 1);
  Serial.println(msg);
  client.publish("@shadow/data/update", msg);

  delay(1000);
  client.loop();
}
