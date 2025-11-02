#include <WiFi.h>           // เปลี่ยนจาก ESP8266WiFi.h เป็น WiFi.h
#include <PubSubClient.h>
#include <DHT.h>

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

char msg[100];

// ------------------ DHT ------------------
#define DHTTYPE DHT11
#define DHTPIN 4   // ESP32 ใช้หมายเลข GPIO จริง (D4 บน ESP8266 = GPIO2)  
DHT dht(DHTPIN, DHTTYPE, 15);  // 15 = delay interval ของ DHT

unsigned long int humid, humid_old;
unsigned long int temp, temp_old;
unsigned long int humid_max = 100, temp_max = 50;

// ------------------ MQTT Reconnect ------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_client, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      client.subscribe("@msg/Tempstatus");
      client.subscribe("@msg/Humidstatus");
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
}

// ------------------ Read DHT ------------------
void project() {
  humid = dht.readHumidity();
  temp = dht.readTemperature();

  if (humid <= humid_max) humid_old = humid;
  if (humid > humid_max) humid = humid_old;

  Serial.print("humid=");
  Serial.println(humid);

  if (temp <= temp_max) temp_old = temp;
  if (temp > temp_max) temp = temp_old;

  Serial.print("Temp=");
  Serial.println(temp);
  Serial.println("---------------------------");
}

// ------------------ Setup ------------------
void setup() {
  Serial.begin(115200);
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

  dht.begin();
}

// ------------------ Loop ------------------
void loop() {
  if (!client.connected()) reconnect();

  project();  // อ่านค่า DHT

  String data = String(temp) + "," + String(humid);
  data.toCharArray(msg, (data.length() + 1));
  client.publish("@msg/update", msg);  // ส่งไป Node-RED

  data = "{\"data\":{\"Tempstatus\":" + String(temp) + ",\"Humidstatus\":" + String(humid) + "}}";
  data.toCharArray(msg, (data.length() + 1));
  client.publish("@shadow/data/update", msg);  // ส่งไป NETPIE Widget

  delay(1000);
  client.loop();
}
