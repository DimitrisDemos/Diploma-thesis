#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>


 

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

byte nuidPICC[4] = {'77', '3d', '61', '9b'};
void callback(char *topic, byte *payload, unsigned int length);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
   

    client.setCallback(callback);
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

    char msg[50];
    msg[0]='\0';
    array_to_string(nuidPICC,4,msg);


    // Publish and subscribe
    client.publish(topic, msg);
    client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
    payload[length]='\0';
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


void OpenLOck(){
  

}

void loop() {
    client.loop();
}
