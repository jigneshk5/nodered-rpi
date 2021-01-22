#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <Servo.h>
#include "DHT.h"

WiFiClient net;
MQTTClient client;
Servo myservo;  // create servo object to control a servo
#define DHTTYPE DHT11   // DHT 11

const int DHTPin = 5;   //D1

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
String ssid="your-wifi-name";
String pass="your-wifi-password";
unsigned long lastMillis = 0;

const int led= D0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("nodemcu_client")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");

  client.subscribe("nodemcu/led");
  client.subscribe("nodemcu/servo");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if(topic=="nodemcu/led"){
    if(payload=="ON")
      digitalWrite(led,LOW);
    else
      digitalWrite(led,HIGH);
  }
  if(topic=="nodemcu/servo"){
   myservo.write(payload.toInt());   
   }
  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  pinMode(led, OUTPUT);
  digitalWrite(led,HIGH);
  dht.begin();
  myservo.attach(D8);  // attaches the servo on D8 to the servo object
  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("192.168.0.196", net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(1000);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every 5 second.
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    int ldr = analogRead(A0);   // read the ldr input on analog pin 0
    client.publish("nodemcu/ldr",String(ldr));
    
    float h = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(h) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    client.publish("nodemcu/dht","{\"temp\":"+String(temp)+", \"humidity\":"+String(h)+"}");
  }
}
