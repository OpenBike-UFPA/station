/*
 *  Created by TheCircuit
*/

#define SS_PIN 4  //D2
#define RST_PIN 5 //D1
#define rele 2 //D4

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

char data2[20];
//Topicos de entrada
#define mqtt_saida_slot01 "/openbike/ufpa/5a5fd0312fcc3e0010adbcba/devolution2/1"
#define mqtt_entrada_slot01 "/openbike/ufpa/station/loan/"


#define ssid "mqttlasse" //Nome da rede que queresmo conectar
#define password "l@ss3m0tt8266" //Senha da rede

#define MQTT_SERVER "200.239.93.85" //Host do servidor MQTT (Broker)
#define MQTT_USER " "
#define MQTT_PASSWORD " "

//Parametros usados no cliente MQTT
long lastMsg = 0;
int cont = 0;
char msg[50];

boolean flag = true;

WiFiClient CLIENT;
PubSubClient MQTT(CLIENT);


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int statuss = 0;
int out = 0;

void setup()
{
  pinMode(rele, OUTPUT);
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  setupwifi(); //Configura o wifi
  setupOTA(); // Configura o sistema de OTA
  setMQTT(); //Configura o MQTT
}
void loop()
{
  ArduinoOTA.handle(); //Função em loop do sistema de OTA

  //Verifica a coneção com o Broker MQTT
  if (!MQTT.connected()) {
    reconectar();
  }
  MQTT.loop(); //Função em loop do cliente MQTT
  if(!MQTT.loop()) MQTT.connect("Estation01", MQTT_USER, MQTT_PASSWORD);


  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  content.substring(1).toCharArray(data2, 20); //59
  if(flag==true)
  {
      MQTT.publish(mqtt_saida_slot01, data2, false);
      flag = false;
  }
  if(content.substring(1) != "85 62 A4 75")
  {
    flag = true;
  }

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived [");
  Serial1.print(topic);
  Serial1.print("] ");
  for (int i = 0; i < length; i++) {
    Serial1.print(char(payload[i]));
  }
  Serial1.println();

  // Switch on the LED if an 1 was received as first character
  //if (topic == mqtt_command) {
    if ((char)payload[0] == '1') {
      digitalWrite(rele, HIGH);
      delay(500);
      digitalWrite(rele, LOW);
      flag = true;

    } else {

      Serial1.println("Comando nao aceito");

    }
  //}

}


void reconectar() {
  while (!MQTT.connected()) {
    Serial1.println("Conectando ao Broker MQTT.");
    if (MQTT.connect("Estacao01", MQTT_USER, MQTT_PASSWORD))
    {
      Serial1.println("Conectado com Sucesso ao Broker");
      MQTT.subscribe(mqtt_entrada_slot01);
    } else {
      Serial1.print("Falha ao Conectador, rc=");
      Serial1.print(MQTT.state());
      Serial1.println(" tentando se reconectar...");
      delay(3000);
    }
  }
}

void setupOTA() {
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("estacao_01");

    // No authentication by default
    ArduinoOTA.setPassword((const char *)"cop@");

    ArduinoOTA.onStart([]()
    {
      Serial1.println("Start");
    });
    ArduinoOTA.onEnd([]()
    {
      Serial1.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error)
    {
      Serial1.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial1.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial1.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial1.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial1.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial1.println("End Failed");
    });
    ArduinoOTA.begin();
}

void setupwifi ()
{
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("Booting---");
  WiFi.mode(WIFI_STA);
  //WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  int teltje = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial1.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial1.println("Ready");
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());
 }

void setMQTT()
{
  MQTT.setServer(MQTT_SERVER, 1883);
  MQTT.setCallback(callback);
}
