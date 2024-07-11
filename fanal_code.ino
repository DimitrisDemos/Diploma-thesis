#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include<string.h>
#include <MFRC522.h>

#define SS_PIN    5  // ESP32 pin GPIO5 
#define RST_PIN   27 // ESP32 pin GPIO27 
#define PIN_RELAY_1  32 // The ESP32 pin GPIO33 connected to the IN1 pin of relay module
#define PIN_RELAY_2  33 // The ESP32 pin GPIO32 connected to the IN2 pin of relay module
 
MFRC522 rfid(SS_PIN, RST_PIN);

// WiFi
const char *ssid = "VODAFONE_1278"; // Enter your Wi-Fi name
const char *password = "gk6gbfm97538e2t4";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "Mqtt.motesense.com";
const char *topic = "emqx/esp32";
const char *mqtt_username = "demos";
const char *mqtt_password = "demos1#$";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

byte nuidPICC[4];
//void callback(char *topic, byte *payload, unsigned int length);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // initialize digital pin as an output.
    pinMode(PIN_RELAY_1, OUTPUT);
    pinMode(PIN_RELAY_2, OUTPUT);

    SPI.begin(); // init SPI bus
    rfid.PCD_Init(); // init MFRC522


    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
   
    
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    String s = String();
    Serial.println();
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        s += (char) payload[i];
    }
    Serial.println();
    Serial.println("-----------------------");
    Serial.println(s);
    if (s == "true"){
      Serial.println("Access Granted");
      OpenLock();
    }
    payload[length]='\0';
    s = "";
}

void array_to_string(byte a[],unsigned int len,char buffer[])
{
  for(unsigned int i=0;i<len;i++)
  {
    byte nib1=(a[i]>>4)&0x0F;
    byte nib2=(a[i]>>0)&0x0F;
    buffer[i*2+0]=nib1 < 0x0A ? '0' + nib1 : 'A'+ nib1 - 0x0A;
    buffer[i*2+1]=nib2 < 0x0A ? '0' + nib2 : 'A'+ nib2 - 0x0A;
  }
  buffer[len*2]='\0';
}


void OpenLock(){
  Serial.println("Turn on PIN_RELAY_1");
  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_RELAY_2, LOW);
  delay(1000);
  Serial.println("Turn on PIN_RELAY_2");
  digitalWrite(PIN_RELAY_1, LOW);
  digitalWrite(PIN_RELAY_2, HIGH);
  delay(1000);
  digitalWrite(PIN_RELAY_2, LOW);
}

void loop() {
  client.loop();
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      if (rfid.uid.uidByte[0] != nuidPICC[0] || 
        rfid.uid.uidByte[1] != nuidPICC[1] || 
        rfid.uid.uidByte[2] != nuidPICC[2] || 
        rfid.uid.uidByte[3] != nuidPICC[3] ) {
        Serial.println(F("A new card has been detected."));
        
        for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
        }

        char msg[50];
        msg[0]='\0';
        array_to_string(nuidPICC,4,msg);

        // Publish and subscribe
        client.publish(topic, msg);
        client.subscribe(topic);

        client.setCallback(callback);

      }
      else
      {
        Serial.print("Access denied, UID:");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD

      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = '\0';
        }
    }
  }
}
