// ###FEHLER:
// Alias und Server werden vermischt

// In Arbeit
// empfangen für LED

// ###ToDo
// Daten nur senden, wenn sich was geändert hart
// empfangen einer reset Aufforderung
// Update over Air
// Bei der Initialisierung der Sensoren abfragen, ob erfolgreich und nur dann weiter mit dem Sensor
// VBatt Sensor und die anderen Systemwerte
// MQTT Client ID -> DeviceID

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>              // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>         // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#include <Ticker.h>
#include <EEPROM.h>               // Funktionsweise übernommen von https://github.com/tzapu/SonoffBoilerplate/blob/master/SonoffBoilerplate.ino

// Sendeinterval
#define sendSystemValueInterval  3600000 //3600000 = 1x pro Stunde
#define sendSensorValueInterval  60000 //60000 = 1x pro Minute
bool firstSendSystemValue = 0;
bool firstSendSensorValue = 0;

// Ticker
Ticker ticker;
int tickerCount = 0;

//Alias
String alias_str;
//char alias_char[5];
char alias_Topic[30] = "";

//Touch
const int button1Pin = D5;
bool button1State = 0;
bool button1State_last = 0;
String Touch1_state_str;
char Touch1_state_char[5];
char Touch1_state_Topic[30] = "";

//DS10B20 Temp OneWire
OneWire  myOneWire(D7);  // on pin 10 (a 4.7K resistor is necessary)
DallasTemperature myDS10B20(&myOneWire);
float DS10B20_temperatur;
String DS10B20_temperatur_str = "";
char DS10B20_temperatur_char[10] = "";
char DS10B20_temperatur_Topic[30] = "";

//TSL2561 Light I2C
Adafruit_TSL2561_Unified myTSL2561 = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
float TSL2561_brightness;
String TSL2561_brightness_str = "";
char TSL2561_brightness_char[10] = "";
char TSL2561_brightness_Topic[30] = "";

//SHT20 Humidity, temperatur I2C
HTU21D myHTU21D;
float HTU21D_humidity;
String HTU21D_humidity_str = "";
char HTU21D_humidity_char[10] = "";
char HTU21D_humidity_Topic[30] = "";

float HTU21D_temperatur;
String HTU21D_temperatur_str = "";
char HTU21D_temperatur_char[10] = "";
char HTU21D_temperatur_Topic[30] = "";

//ESP
int configmode = 0;
long  chipid;
String  chipidString;
char chipidChar[8] = "";
char hostName[30] = "SensorNode-";

char ESPWiFiMAC_char[10] = "";
String ESPWiFiMAC_str;
char ESPFlashChipId_char[10] = "";
char ESPFlashChipSize_char[10] = "";
char ESPFlashChipSpeed_char[10] = "";
char ESPCycleCount_char[10] = "";
char ESPFreeHeap_char[10] = "";
char ESPmillis_char[10] = "";
char ESPVBatt_char[10] = "";

char ESPWiFiMAC_Topic[30] = "";
char ESPFlashChipId_Topic[30] = "";
char ESPFlashChipSize_Topic[30] = "";
char ESPFlashChipSpeed_Topic[30] = "";
char ESPCycleCount_Topic[30] = "";
char ESPFreeHeap_Topic[30] = "";
char ESPmillis_Topic[30] = "";
char ESPVBatt_Topic[30] = "";

// subscribe
char subscribe_Topic[30] = "";

// MQTT
#define EEPROM_SALT 12662
typedef struct {
  char mqtt_server[40] = "192.168.1.15";
  char mqtt_port[6]    = "1883";
  char mqtt_user[20]   = "*****";
  char mqtt_pass[20]   = "*****";
  char mqtt_alias[20]   = "Alias";
  int  salt            = EEPROM_SALT;
} WMSettings;

WMSettings settings;

WiFiManager wifiManager;  // wifimanager Instanz
WiFiClient wifiClient;

//NEOPIXEL
const int NEOPIXEL_PIN = D6; //Neopixel Pin
const int NEOPIXEL_COUNT = 2; //Neopixel Anzahl
Adafruit_NeoPixel myNEOPIXEL = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

int NEO_R = 0;
int NEO_G = 0;
int NEO_B = 0;
int NEO_00R = 0;
int NEO_00G = 0;
int NEO_00B = 0;
int NEO_01R = 0;
int NEO_01G = 0;
int NEO_01B = 0;
int NEO_BRIGHT = 255;

void callback(char* topic, byte* payload, unsigned int length) { //nicht verschieben!
  // handle message arrived
  Serial.println("### Message arrived");
  Serial.print("Topic: ");
  Serial.println(String(topic));

  String value = String((char*)payload);
  Serial.println("---------");
  Serial.println(value);
  Serial.println(value.substring(1, 3));
  Serial.println(value.substring(3, 5));
  Serial.println(value.substring(5, 7));
  Serial.println("---------");

  NEO_R = value.substring(1, 3).toInt();
  NEO_G = value.substring(3, 5).toInt();
  NEO_B = value.substring(5, 7).toInt();

  // Aktoren setzen
  if (String(topic) == "192a66/IN/LED") {
    Serial.println("Daten fuer alle LED");
    NEO_00R = NEO_R;
    NEO_00G = NEO_G;
    NEO_00B = NEO_B;
    NEO_01R = NEO_R;
    NEO_01G = NEO_G;
    NEO_01B = NEO_B;
  }
  if (String(topic) == "192a66/IN/LED01") {
    Serial.println("Daten fuer LED 01");
    NEO_00R = NEO_R;
    NEO_00G = NEO_G;
    NEO_00B = NEO_B;

  }
  if (String(topic) == "192a66/IN/LED02") {
    Serial.println("Daten fuer LED 02");

    NEO_01R = NEO_R;
    NEO_01G = NEO_G;
    NEO_01B = NEO_B;
  }
}

bool shouldSaveConfig = false;
void saveConfigCallback () {
  Serial.println("### Daten müssen gespeichert werden");
  shouldSaveConfig = true;
}

//PubSubClient mqttClient(settings.mqtt_server, atoi(settings.mqtt_port), callback, wifiClient);
PubSubClient mqttClient(wifiClient); //##Test ohne PW

//### SETUP ################################################################################
void setup() {
  mqttClient.setServer(settings.mqtt_server, atoi(settings.mqtt_port)); //##Test ohne PW
  mqttClient.setCallback(callback);

  Serial.begin(115200);
  Serial.println("\n Starte ESP NODE");

  // Heartbeat
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  //Touch
  pinMode(button1Pin, INPUT);

  //EEPROM
  EEPROM.begin(512);
  EEPROM.get(0, settings);
  EEPROM.end();

  if (settings.salt != EEPROM_SALT) {
    Serial.println("### Fehlerhafte Speicherdaten im EEPROM, verwende default Werte");
    WMSettings defaults;
    settings = defaults;
  }

  // Hostname
  chipid = ESP.getChipId();
  chipidString = String(ESP.getChipId(), HEX);
  ultoa(chipid, chipidChar, HEX);  // befüllt chipid_Topic char mit chipid long
  strcat(hostName, chipidChar);  // hängt chipid_Topic an hostName an

  Serial.println("### ChipID");
  Serial.print("chipidChar: ");
  Serial.println(chipidChar);
  Serial.print("hostName: ");
  Serial.println(hostName);
  Serial.println("");

  // MQTT
  Serial.println("### MQTT Verbindungsaufbau");
  Serial.print("mqtt_user: ");
  Serial.println(settings.mqtt_user);
  Serial.print("mqtt_pass: ");
  Serial.println(settings.mqtt_pass);

  //mqttClient.connect ("arduinoClient", settings.mqtt_user, settings.mqtt_pass);
  mqttClient.connect ("arduinoClient");

  WiFiManagerParameter custom_mqtt_text1("<br/><b>MQTT config</b><br/>");
  wifiManager.addParameter(&custom_mqtt_text1);

  WiFiManagerParameter custom_mqtt_text2("Server");
  wifiManager.addParameter(&custom_mqtt_text2);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", settings.mqtt_server, 40);
  wifiManager.addParameter(&custom_mqtt_server);

  WiFiManagerParameter custom_mqtt_text3("Port");
  wifiManager.addParameter(&custom_mqtt_text3);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", settings.mqtt_port, 6);
  wifiManager.addParameter(&custom_mqtt_port);

  WiFiManagerParameter custom_mqtt_text4("User");
  wifiManager.addParameter(&custom_mqtt_text4);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", settings.mqtt_user, 20);
  wifiManager.addParameter(&custom_mqtt_user);

  WiFiManagerParameter custom_mqtt_text5("Pass");
  wifiManager.addParameter(&custom_mqtt_text5);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", settings.mqtt_pass, 20);
  wifiManager.addParameter(&custom_mqtt_pass);

  WiFiManagerParameter custom_mqtt_text6("Alias");
  wifiManager.addParameter(&custom_mqtt_text6);
  WiFiManagerParameter custom_mqtt_alias("server", "mqtt alias", settings.mqtt_alias, 20);
  wifiManager.addParameter(&custom_mqtt_alias);

  wifiManager.setAPCallback (configModeCallback); // wird Aufgerufen, wenn wifimanager in den Config Mode wechselt
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setCustomHeadElement("<style>button{border:1;border-radius:0;background-color:#cccccc;color:#000000;}</style>");
  // wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0)); // IP in Config
  // wifiManager.setMinimumSignalQuality(10); Mindestqualität des WiFi Netzes
  wifiManager.setTimeout(300); // Timeout 300 Sekunden, falls beim Neustart nur die Verbindung niht hergestellt werden konnte

  button1State = !digitalRead(button1Pin);
  if (digitalRead(button1State) == 1) {  // manueller Start des WiFi Managers
    Serial.println ("### Konfiguration durch Tastendruck beim Startprozess");
    if (!wifiManager.startConfigPortal(hostName)) {
      delay(3000);
      ESP.reset();
      delay(5000);
    }
  }

  //Heartbeat
  ticker.attach(0.01, setupHeartbeatTick); // langsames Blinken

  wifiManager.autoConnect (hostName); //Startet AccesPoint, wenn keine Konfiguration im System oder keine Verbindung möglich

  if (shouldSaveConfig) {
    Serial.println("### Konfiguration speichern");

    Serial.print("mqtt_server: ");
    Serial.println(custom_mqtt_server.getValue());
    Serial.print("mqtt_port: ");
    Serial.println(custom_mqtt_port.getValue());
    Serial.print("mqtt_user: ");
    Serial.println(custom_mqtt_user.getValue());
    Serial.print("mqtt_pass: ");
    Serial.println(custom_mqtt_pass.getValue());
    Serial.print("mqtt_alias: ");
    Serial.println(custom_mqtt_alias.getValue());

    strcpy(settings.mqtt_server, custom_mqtt_server.getValue());
    strcpy(settings.mqtt_port, custom_mqtt_port.getValue());
    strcpy(settings.mqtt_user, custom_mqtt_user.getValue());
    strcpy(settings.mqtt_pass, custom_mqtt_pass.getValue());
    strcpy(settings.mqtt_alias, custom_mqtt_alias.getValue());

    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  }

  //NEOPIXEL
  myNEOPIXEL.setBrightness(NEO_BRIGHT);
  myNEOPIXEL.begin();
  myNEOPIXEL.show();

  //Alias
  ultoa(chipid, alias_Topic, HEX);
  strcat(alias_Topic, "/alias");  // Type anhängen

  //Touch
  ultoa(chipid, Touch1_state_Topic, HEX);
  strcat(Touch1_state_Topic, "/Touch1_state");  // Type anhängen

  //DS10B20 temperatur OneWire
  myDS10B20.begin();
  ultoa(chipid, DS10B20_temperatur_Topic, HEX); //Topic mit ChipID in HEX füllen
  strcat(DS10B20_temperatur_Topic, "/DS10B20_temperatur");  // Type anhängen

  //TSL2561 brightness I2C
  myTSL2561.begin();
  myTSL2561.enableAutoRange(true);
  myTSL2561.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  ultoa(chipid, TSL2561_brightness_Topic, HEX);
  strcat(TSL2561_brightness_Topic, "/TSL2561_brightness");

  //SHT20 Humidity, Temp I2C
  myHTU21D.begin();
  ultoa(chipid, HTU21D_humidity_Topic, HEX);
  strcat(HTU21D_humidity_Topic, "/HTU21D_humidity");
  ultoa(chipid, HTU21D_temperatur_Topic, HEX);
  strcat(HTU21D_temperatur_Topic, "/HTU21D_temperatur");

  //ESP
  ESPWiFiMAC_str = WiFi.macAddress();
  ultoa(chipid, ESPWiFiMAC_Topic, HEX);
  strcat(ESPWiFiMAC_Topic, "/ESPWiFiMAC");
  ultoa(chipid, ESPFlashChipId_Topic, HEX);
  strcat(ESPFlashChipId_Topic, "/FlashChipId");
  ultoa(chipid, ESPFlashChipSize_Topic, HEX);
  strcat(ESPFlashChipSize_Topic, "/FlashChipSize");
  ultoa(chipid, ESPFlashChipSpeed_Topic, HEX);
  strcat(ESPFlashChipSpeed_Topic, "/FlashChipSpeed");
  ultoa(chipid, ESPCycleCount_Topic, HEX);
  strcat(ESPCycleCount_Topic, "/CycleCount");
  ultoa(chipid, ESPFreeHeap_Topic, HEX);
  strcat(ESPFreeHeap_Topic, "/FreeHeap");
  ultoa(chipid, ESPmillis_Topic, HEX);
  strcat(ESPmillis_Topic, "/millis");
  ultoa(chipid, ESPVBatt_Topic, HEX);
  strcat(ESPVBatt_Topic, "/VBatt");

  //subscribe
  ultoa(chipid, subscribe_Topic, HEX);
  strcat(subscribe_Topic, "/IN/#");
}

//### LOOP ################################################################################
void loop() {
  if (configmode == 1) {
    ESP.reset();
  }

  mqttClient.loop();

  //Daten empfangen
  if (mqttClient.connect("arduinoClient")) {
    mqttClient.subscribe(subscribe_Topic);
  }

  //sofort senden, wenn sich was ändert
  //Touch
  button1State = digitalRead(button1Pin);
  if (button1State != button1State_last) {
    Touch1_state_str = String(button1State);
    Touch1_state_str.toCharArray(Touch1_state_char, Touch1_state_str.length() + 1);
    //if (mqttClient.connect("arduinoClient", settings.mqtt_user, settings.mqtt_pass)) {
    if (mqttClient.connect("arduinoClient")) {
      mqttClient.publish(Touch1_state_Topic, Touch1_state_char);
    }
    button1State_last = button1State;
  }

  // Senden im sendSystemValueInterval
  if ((millis() % sendSystemValueInterval  == 0) || (firstSendSystemValue == 0)) {  //System Values senden
    firstSendSystemValue = 1;
    ESPWiFiMAC_str.toCharArray(ESPWiFiMAC_char, ESPWiFiMAC_str.length() + 1);
    ultoa(ESP.getFlashChipId(), ESPFlashChipId_char, HEX);
    ultoa(ESP.getFlashChipSize(), ESPFlashChipSize_char, DEC); ;
    ultoa(ESP.getFlashChipSpeed(), ESPFlashChipSpeed_char, DEC);
    ultoa(ESP.getCycleCount(), ESPCycleCount_char, DEC);
    ultoa(ESP.getFreeHeap(), ESPFreeHeap_char, DEC);
    ultoa(millis(), ESPmillis_char, DEC); //millis();
    strcat(ESPVBatt_char, "");  //Analogeingang auslesen

    //if (mqttClient.connect("arduinoClient", settings.mqtt_user, settings.mqtt_pass)) {
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("### mqttClient.connect - Systemwerte senden erfolgreich");
      mqttClient.publish(alias_Topic, settings.mqtt_alias);
      mqttClient.publish(ESPWiFiMAC_Topic, ESPWiFiMAC_char);
      mqttClient.publish(ESPFlashChipId_Topic, ESPFlashChipId_char);
      mqttClient.publish(ESPFlashChipSize_Topic, ESPFlashChipSize_char);
      mqttClient.publish(ESPFlashChipSpeed_Topic, ESPFlashChipSpeed_char);
      mqttClient.publish(ESPCycleCount_Topic, ESPCycleCount_char);
      mqttClient.publish(ESPFreeHeap_Topic, ESPFreeHeap_char);
      mqttClient.publish(ESPmillis_Topic, ESPmillis_char);
      mqttClient.publish(ESPVBatt_Topic, ESPVBatt_char);

    } else {
      Serial.println("### mqttClient.connect - Systemwerte senden mit Fehler");
    }
  }

  // Senden im sendSensorValueInterval
  if ((millis() % sendSensorValueInterval  == 0) || (firstSendSensorValue == 0) ) { //Sensorabfrage und senden
    firstSendSensorValue = 1;
    Serial.println("### Sensordaten senden");

    //Touch
    Touch1_state_str = String(button1State);
    Touch1_state_str.toCharArray(Touch1_state_char, Touch1_state_str.length() + 1);

    //DS10B20 Temperatur OneWire
    myDS10B20.requestTemperatures();
    DS10B20_temperatur = myDS10B20.getTempCByIndex(0);
    DS10B20_temperatur_str = String(DS10B20_temperatur);
    DS10B20_temperatur_str.toCharArray(DS10B20_temperatur_char, DS10B20_temperatur_str.length() + 1);

    //TSL2561 Light I2C
    digitalWrite(LED_BUILTIN, HIGH); // LED Aus vor Lichtmessung
    sensors_event_t TSL2561_event;
    myTSL2561.getEvent(&TSL2561_event);
    TSL2561_brightness = TSL2561_event.light;
    TSL2561_brightness_str = String(TSL2561_brightness);
    TSL2561_brightness_str.toCharArray(TSL2561_brightness_char, TSL2561_brightness_str.length() + 1);

    //SHT20 Humidity, Temp I2C
    HTU21D_humidity = myHTU21D.readHumidity();
    HTU21D_humidity_str = String(HTU21D_humidity);
    HTU21D_humidity_str.toCharArray(HTU21D_humidity_char, HTU21D_humidity_str.length() + 1);

    HTU21D_temperatur = myHTU21D.readTemperature();
    HTU21D_temperatur_str = String(HTU21D_temperatur);
    HTU21D_temperatur_str.toCharArray(HTU21D_temperatur_char, HTU21D_temperatur_str.length() + 1);

    // MQTT senden
    Serial.println("");
    Serial.println("### MQTT Daten senden");

    Serial.print("Server: ");
    Serial.println(settings.mqtt_server);

    Serial.print("Port: ");
    Serial.println(settings.mqtt_port);

    Serial.print("User: ");
    Serial.println(settings.mqtt_user);

    Serial.print("Pass: ");
    Serial.println(settings.mqtt_pass);

    Serial.println("");

    Serial.print("DS10B20_temperatur: ");
    Serial.print(DS10B20_temperatur_Topic);
    Serial.print(" | ");
    Serial.println(DS10B20_temperatur_char);

    Serial.print("TSL2561_brightness: ");
    Serial.print(TSL2561_brightness_Topic);
    Serial.print(" | ");
    Serial.println(TSL2561_brightness_char);

    Serial.print("HTU21D_temperatur: ");
    Serial.print(HTU21D_temperatur_Topic);
    Serial.print(" | ");
    Serial.println(HTU21D_temperatur_char);

    Serial.print("HTU21D_humidity: ");
    Serial.print(HTU21D_humidity_Topic);
    Serial.print(" | ");
    Serial.println(HTU21D_humidity_char);

    //if (mqttClient.connect("arduinoClient", settings.mqtt_user, settings.mqtt_pass)) {
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("### mqttClient.connect - Messdaten senden erfolgreich");
      mqttClient.publish(Touch1_state_Topic, Touch1_state_char);
      mqttClient.publish(DS10B20_temperatur_Topic, DS10B20_temperatur_char);
      mqttClient.publish(TSL2561_brightness_Topic, TSL2561_brightness_char);
      mqttClient.publish(HTU21D_temperatur_Topic, HTU21D_temperatur_char);
      mqttClient.publish(HTU21D_humidity_Topic, HTU21D_humidity_char);
    } else {
      Serial.println("### mqttClient.connect - Messdaten senden mit Fehler");
    }
  }

  // Aktoren setzen
  myNEOPIXEL.setPixelColor(0, myNEOPIXEL.Color(NEO_00R, NEO_00G, NEO_00B));
  myNEOPIXEL.setPixelColor(1, myNEOPIXEL.Color(NEO_01R, NEO_01G, NEO_01B));
  myNEOPIXEL.setBrightness(NEO_BRIGHT);
  myNEOPIXEL.show();
}

//### FUNCTIONEN ################################################################################
void configModeCallback (WiFiManager * myWiFiManager) {
  //wifimanager Config Mode gestartet
  Serial.println("### Funktion configModeCallback aufgerufen");
  ticker.attach(0.1, setupWiFiTick); // schnelles Blinken, wenn im WiFi Setup
  configmode = 1;
}

void setupWiFiTick() { // schnelles Blinken, wenn im WiFi Setup
  if (digitalRead(LED_BUILTIN) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setupHeartbeatTick() { // langsames Blinken, Heartbeat
  if (tickerCount == 100) {
    digitalWrite(LED_BUILTIN, LOW);
    tickerCount = 0;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    tickerCount = tickerCount + 1;
  }
}



