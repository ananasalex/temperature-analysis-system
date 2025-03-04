#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
//#include <TermoRes3950.h>
#include <IRremote.hpp> 
//#include <microDS18B20.h>
//MicroDS18B20<4> sensor;
#define DECODE_NEC          // Includes Apple and Onkyo. To enable all protocols , just comment/disable this line.
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Настройки Wi-Fi
#include <iostream>
using namespace std;
// const char* ssid = "SmartHome";
// const char* password = "ipu75energy";
const char* ssid = "Keenetic-1540";
const char* password = "TkJJxhKK";
//const char* ssid = "l40_1";
//const char* password = "s072A21T";
IPAddress local_IP(10,101,0,130);
// Настройки MQTT-сервера
const char* mqtt_server = "10.101.0.5"; // pi mob  ThingsBoard
//const char* mqtt_server = "193.232.208.60"; // pi mob  ThingsBoard
const char* mqtt_server = "192.168.138.75"; // pi mob teplocentr
const char* access_token = "arseniy8"; // токен устройства на ThingsBoard

// Настройки NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Сервер NTP, смещение по UTC (0), обновление раз в 60 сек

String message;
int Int_message;
float temp = 0;
int bright = 0;
double resist;
double ratio;
unsigned long previousMillis = 0;
unsigned long interval = 5000;

WiFiClient espClient;
PubSubClient client(espClient);

String device_id = "ESP32-008";

void setup_wifi() {
  delay(100);
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

void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  //char a='a';
  
  message = messageTemp;
  
}



void setup() {
  delay(2000);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  sensors.begin();
  //pinMode(34, INPUT); // Инициализация пина для температуры
  //pinMode(35, INPUT);  // Инициализация пина для освещенности
  //pinMode(4, INPUT);
  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  client.setCallback(callback);
  timeClient.begin();
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Попробуйте подключиться с идентификатором клиента
    if (client.connect(device_id.c_str(), access_token, "")) {
      Serial.println("connected");
      client.subscribe("test2");
      // Подпишитесь, если необходимо
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Подождите 5 секунд перед повторной попыткой
      delay(5000);
    }
  }
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  unsigned long currentMillis = millis();  // Получаем текущее время
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis; // Сохраняем текущее время как время последнего выполнения кода
    /*temp = analogRead(34);
    ratio = double(temp)/double(4095);
    Serial.print("ratio = ");
    Serial.println(ratio);
    resist = double(9.91)*(double(4095)-double(temp))/double(temp);
    Serial.print("resist = ");
    Serial.println(resist);
    Serial.print("temp = ");
    Serial.println(temp);
    //char a[10];
    //itoa(resist, a, 10);
    // //Serial.print("+++++++++ ");
    //Serial.println(a);
    //client.publish("test1", a, 2);
    temp=perevod(resist);*/
    
    sensors.requestTemperatures();
    delay(750); 
    timeClient.update();

    // Получаем время
    Serial.print("Current time: ");
    Serial.println(timeClient.getFormattedTime());
    float tempC = sensors.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
    delay(1000);
    String payload = "{\"sensor_id\":\"" + device_id + "\", \"temperature\":" + String(tempC) + "\", \"tickstamp\":"+  previousMillis   +"}";
    // Публикация сообщения
    client.publish("v1/devices/me/telemetry", payload.c_str());
    Serial.println("Data sent: " + payload);
    delay(2000);
  }

  
  // char a[10];
  // itoa(resist, a, 10);
  // //Serial.print("+++++++++ ");
  // //Serial.println(a);
  // client.publish("test1", a, 2);
  // itoa(temp, a, 10);
  // client.publish("test1", a, 2);
 //delay(1000);

  
}
