#Hardware
##Anschluss Sensoren und Aktoren:

- D0  [Wake nicht belegt]
- D1  SCL
- D2  SDA
- D3  [Flash nicht belegt]
- D4  
- D5  Touch
- D6  NeoPixel
- D7
- D8  
- A0  [VBatt nicht belegt]

#Firmware
##Unterstützte Sensoren
- Touch
- DS10B20 OneWire Temperatur
- TSL2561 I2C Helligkeit
- SHT20 Luftfeuchte und Temperatur

##Unterstützte Aktoren
- 2 NEOPIXEL

#Funktionen
- WifiManager zur Einstellung IP und MQTT Server über WEB Frontend. Das Web Frontend wird gestartet, wenn keine Parameterdaten im EEPROPM gefunden werden oder beim Start wird der Touch gedrückt. Der Node medldet sich als AP mit der SSID "Sensornode-[NodeID]. 
- Senden Alias an MQTT zur Zuordnung NodeID zu Realen Device
- Senden Messdaten mindestens im Abstand "sendSensorValueInterval"
- Senden Systemwerte mindestens im Abstand "sendSystemValueInterval"
- Watchdog Ticker über "LED_BUILTIN" (Blinken 1/10 = normaler Betrieb, Blinken 1/1 = Wifi Setup Mode)
- Senden der Daten an MQTT Server mit Prefix "NodeID"

#Entwicklung
###Bekannte Fehler
- Alias und Server werden vermischt (Speicherung in EEPROM nicht richtig?!)

###In Arbeit
- empfangen für LED

###offene ToDo's
- Daten nur senden, wenn sich was geändert hart
- empfangen einer Reset Aufforderung
- Updatemöglichkeit (Over Air oder Bereitstellung Flash File Link over MQTT)
- Bei der Initialisierung der Sensoren abfragen, ob erfolgreich und nur dann weiter mit dem Sensor
- VBatt Sensor und die anderen Systemwerte
- MQTT Client ID -> DeviceID
- Senden Watchdog 
- Batteriebetrieb mit Schlafmodus
- Weitere Aktoren (Pumpe, Magnetventil, Fenstersteller)
- Weitere Sensoren (Bodenfeuchte, Füllstand, Wind, Regen)
- Sicherheitsfunktionen (SSL?)