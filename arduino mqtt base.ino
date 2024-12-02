#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = //"WIFI이름";
const char* password = //"WIFI비밀번호";
const char* mqtt_server = "192.168.0.42";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int value = 0;

void setup() {
  //for esp8266 value must be 115200
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//subscribe하고 있는 토픽에 데이터가 들어올때의 콜백
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//MQTT브로커와 연결이 안되었을 경우의 반복값
void reconnect() {
  while (!client.connected()) {
    Serial.print("Wait for MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.publish("fArduino", "연결됨");
      //subscribe하는 topic
      client.subscribe("fRasberry"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}