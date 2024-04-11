#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_http_client.h>

  #define ss 10
  #define rst 17
  #define dio0 2

  const char* ssid = "ieee student branch";
  const char* password = "ieeeUniw@";
  const char* mqttServer = "mqtt-dashboard.com";
  const int mqttPort = 1883; // Default MQTT port
  const char* mqttUser = "mqtt";
  const char* mqttPassword = "mqtt";

  WiFiClient espClient;
  PubSubClient client(espClient);

  void callback(char* topic, byte* payload, unsigned int length);

  void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }

  void reconnect() {
    while (!client.connected()) {
      if (client.connect("NodeMCU_Client", mqttUser, mqttPassword)) {
        Serial.println("mcu-con: ok ");
      } else {
        Serial.println("mcu-con: " + String(client.state()));
        delay(5000);
      }
    }
  }

  void extractTopic(String inputString, String &topic) 
  {
    int firstParenthesisIndex = inputString.indexOf('('); // Find the index of the first parenthesis
    if (firstParenthesisIndex != -1) { // If a parenthesis is found
      int secondParenthesisIndex = inputString.indexOf(')', firstParenthesisIndex); // Find the index of the matching parenthesis
      if (secondParenthesisIndex != -1) { // If a matching parenthesis is found
        topic = inputString.substring(firstParenthesisIndex + 1, secondParenthesisIndex); // Extract the characters between the parentheses
      }
    }
  }

  void extractCleanMessage(String inputString, String &cleanMessage) 
  {
    int firstParenthesisIndex = inputString.indexOf('('); // Find the index of the first parenthesis
    if (firstParenthesisIndex != -1) { // If a parenthesis is found
      int secondParenthesisIndex = inputString.indexOf(')', firstParenthesisIndex); // Find the index of the matching parenthesis
      if (secondParenthesisIndex != -1) { // If a matching parenthesis is found
        cleanMessage = inputString.substring(0, firstParenthesisIndex); // Extract characters before the first parenthesis
        cleanMessage += inputString.substring(secondParenthesisIndex + 1); // Append characters after the second parenthesis
      }
    }
  }

  void setup() {

   

    Serial.begin(115200);
    while (!Serial);
    Serial.println("LoRa Receiver");

    LoRa.setPins(ss, rst, dio0);

    while (!LoRa.begin(433E6)) {
      Serial.println(".");
      delay(500);
    }

    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
    
     WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

   client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
    while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
      if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
        Serial.println("Connected to MQTT broker");
      } else {
        Serial.print("Failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
    
    client.subscribe("testtopic/1");
    
  }



  void loop() {
   
    String LoRaData = "";
    String topic;
    String cleanMessage;

    int packetSize = LoRa.parsePacket();

    if (packetSize) {

      Serial.print("Received packet '");
      
      while (LoRa.available()) {
        LoRaData = LoRa.readString();
        Serial.print(LoRaData);
        
        extractTopic(LoRaData, topic);
        Serial.println(LoRaData);
        extractCleanMessage(LoRaData, cleanMessage);
        Serial.println("topic:");
        Serial.println(topic);
        Serial.println("clean message:");
        Serial.println(cleanMessage); 
        
      }
      String testtopic ="testtopic/"+topic;
      Serial.println(testtopic);
      client.publish(testtopic.c_str(),cleanMessage.c_str());
      client.publish("testtopic/1", LoRaData.c_str());
      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
    }

    if (!client.connected()) {
      reconnect();
    }
   
    client.loop();
    delay(1000);
  }
