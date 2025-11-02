#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ------------------ WiFi ------------------
const char* ssid = "Grissana";
const char* password = "00000000";

// ------------------ MQTT ------------------
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_client = "12a2c927-a8a7-4199-a744-2e3136b3b62a";
const char* mqtt_username = "sfV5UX4RL9PQUpXiRhQehhMaaJqwLsEu";
const char* mqtt_password = "KkDfYhbH24E29CnqN8jmb7YAtTNv4pDU";

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------ GPIO ------------------
#define LED_PIN 13
#define RELAY_PIN 4
#define SW_PIN 33

int LED1mcu2 = 0;
int relayState = 0;
bool netpieConnected = false;

// ------------------ ฟังก์ชันแสดงบนจอ OLED ------------------
void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // บรรทัดที่ 1: WiFi (ขนาดเล็ก)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi: ");
  if (WiFi.status() == WL_CONNECTED)
    display.println(ssid);
  else
    display.println("Disconnected");

  // บรรทัดที่ 2: NETPIE (ขนาด 1)
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("NETPIE: ");
  display.println(netpieConnected ? "Connected" : "Disconnected");

  // บรรทัดที่ 3: LED (ขนาดใหญ่)
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print("Servo:");
  display.setCursor(80, 30);
  display.println((LED1mcu2 == 1) ? "ON" : "OFF");

  // บรรทัดที่ 4: Relay (ขนาดใหญ่)
  display.setTextSize(2);
  display.setCursor(0, 50);
  display.print("Relay:");
  display.setCursor(80, 50);
  display.println((relayState == 1) ? "ON" : "OFF");

  display.display();
}

// ------------------ MQTT Reconnect ------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      client.subscribe("@msg/sw1MCU1");
      netpieConnected = true;
      updateOLED();
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds...");
      netpieConnected = false;
      updateOLED();
      delay(5000);
    }
  }
}

// ------------------ MQTT Callback ------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  msg.replace("\"", "");
  Serial.print("Message arrived ["); Serial.print(topic); Serial.print("]: "); Serial.println(msg);

  if (String(topic) == "@msg/sw1MCU1") {
    if (msg == "on") {
      digitalWrite(LED_PIN, HIGH);
      LED1mcu2 = 1;
    } else if (msg == "off") {
      digitalWrite(LED_PIN, LOW);
      LED1mcu2 = 0;
    }
    updateOLED();
  }
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SW_PIN, INPUT_PULLUP);

  // OLED เริ่มต้น
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  updateOLED();

  // เชื่อม WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    updateOLED();
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  updateOLED();
}

// ------------------ Loop ------------------
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  static int lastSwitchState = HIGH;
  int switchState = digitalRead(SW_PIN);
  if (switchState == LOW && lastSwitchState == HIGH) {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState);
    Serial.print("Relay changed: ");
    Serial.println(relayState ? "ON" : "OFF");

    client.publish("@msg/relay_status", (relayState == 1) ? "on" : "off");

    updateOLED();
    delay(300);
  }
  lastSwitchState = switchState;

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 1000) {
    client.publish("@msg/status_sw", (LED1mcu2 == 1) ? "on" : "off");
    lastSend = millis();
  }

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    updateOLED();
    lastUpdate = millis();
  }
}
