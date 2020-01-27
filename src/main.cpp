#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <NewPing.h>
#include <Wire.h>
#include <Smoothed.h>

#define TRIGGER_PIN 14 // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 12    // Arduino pin tied to echo pin on the ultrasonic sensor.

#define NUM_ROWS 2
#define NUM_COLUMNS 16

#define SECONDS 1000

#define MQTT_TOPIC_LINK         "/level/monitor/link"
#define MQTT_TOPIC_STATUS       "/level/monitor/status"

#define MQTT_TOPIC_SUBSCRIBE      "/level/control/#"
#define MQTT_SUB_TOPIC_VOLUME     "/level/control/volume"
#define MQTT_SUB_TOPIC_RESTART    "/level/control/restart"
#define MQTT_SUB_TOPIC_ENABLEMQTT "/level/control/enablemqtt"
#define MQTT_SUB_TOPIC_SENSORAVG  "/level/control/sensoravg"
#define MQTT_SUB_TOPIC_TIMERMQTT  "/level/control/timermqtt"
#define MQTT_SUB_TOPIC_MINDISTANCE  "/level/control/mindistance"
#define MQTT_SUB_TOPIC_MAXDISTANCE  "/level/control/maxdistance"

#define MQTT_MAX_ATTEMPS 10

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27, NUM_ROWS, NUM_COLUMNS);

Smoothed<float> sensorAvg;
Smoothed<float> sensorExp;
float currentSensorValue;
bool sensorAverage = true;

float max_volume = 7000;
float max_distance = 1700;
float min_distance = 250;
float bottom = 0;

float level = 0;
int volume = 0; //volumen en litros
int level_percent = 0;
float level_factor = 0;

unsigned long statusTimer;
unsigned int timerMqtt = 5;

bool mqttConnected = false;
bool enableMqtt = true;

String DeviceID = "";

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

void wifi_connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("MONTFLOR", "2129612842");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("AUTOM");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_publish(const char *topic, const char *payload)
{
  String topic_buf = DeviceID + topic;
  byte counter = 0;
  while (!mqtt_client.publish(topic_buf.c_str(), payload) && counter <= 10)
  {
    counter++;
  }
  if (counter > MQTT_MAX_ATTEMPS)
  {
    Serial.println("MQTT publish: FALLO");
    Serial.println("RC = " + mqtt_client.state());
    mqtt_client.disconnect();
  }
  else
  {
    Serial.println("MQTT publish: TOPIC: " + topic_buf);
  }
}

bool mqtt_subscribe(const char *topic)
{
  String topic_buf = DeviceID + topic;
  if (mqtt_client.subscribe(topic_buf.c_str()))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void mqtt_callback(char *_topic, byte *payload, unsigned int length)
{
  /* TOPIC */
  String topic = String(_topic);
  Serial.println("Mensaje en Topic: " + topic);

  /* PAYLOAD */
  String messageTemp;
  for (uint8_t i = 0; i < length; i++)
  {
    messageTemp += (char)payload[i];
  }

  /* VERIFICACION DE TOPIC */
  //RESTART
  if (topic == DeviceID + MQTT_SUB_TOPIC_RESTART)
  {
    if (messageTemp.toInt())
    {
      ESP.restart();
    }
  }
  //VOLUME
  if (topic == DeviceID + MQTT_SUB_TOPIC_VOLUME)
  {
    if (messageTemp.toInt())
    {
      max_volume = messageTemp.toInt();
    }
  }
  //MIN_DISTANCE
  if (topic == DeviceID + MQTT_SUB_TOPIC_MINDISTANCE)
  {
    if (messageTemp.toInt())
    {
      min_distance = messageTemp.toInt(); // in mm
    }
  }
  //MAX_DISTANCE
  if (topic == DeviceID + MQTT_SUB_TOPIC_MAXDISTANCE)
  {
    if (messageTemp.toInt())
    {
      max_distance = messageTemp.toInt(); // in mm
    }
  }
  //TIMER MQTT
  if (topic == DeviceID + MQTT_SUB_TOPIC_TIMERMQTT)
  {
    if (messageTemp.toInt())
    {
      timerMqtt = messageTemp.toInt(); // in seconds
    }
  }

  //ENABLEMQTT
  if (topic == DeviceID + MQTT_SUB_TOPIC_ENABLEMQTT)
  {
    if (messageTemp.toInt())
    {
      enableMqtt = false;
    }
    else{
      enableMqtt = false;
    }
  }
  //Select smoother
  if (topic == DeviceID + MQTT_SUB_TOPIC_SENSORAVG)
  {
    if (messageTemp.toInt())
    {
      sensorAverage = true;
    }
    else{
      sensorAverage = false;
    }
  }
}

void mqtt_connect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Inicio conexion MQTT");

    while (!mqtt_client.connected())
    {
      Serial.println("MQTT no esta conectado, iniciando conexion");

      String lwt_str_topic = DeviceID + MQTT_TOPIC_LINK;
      char lwt_char_topic[60] = "";
      lwt_str_topic.toCharArray(lwt_char_topic, sizeof(lwt_char_topic));

      if (mqtt_client.connect(DeviceID.c_str(), "", "", lwt_char_topic, 0, false, "Disconnected", false))
      {
        Serial.println("conexion a Broker MQTT exitosa");
        mqtt_publish(MQTT_TOPIC_LINK, "Connected");

        mqtt_subscribe(MQTT_TOPIC_SUBSCRIBE);
      }
      else
      {
        //esperar 5 segundos antes de reintentar
        Serial.println("conexion a mqtt fallida");
        Serial.println("RC = " + String(mqtt_client.state()));
        Serial.println("Intentando de nuevo en 5 seg");
        delay(5000);
      }
    }
  }
}

// Conectar MQTT
void mqtt_start()
{
  DeviceID = WiFi.macAddress();
  DeviceID.replace(":", "");
  // Configurar conexion al broker
  mqtt_client.setServer("controlador-bombas.eastus.cloudapp.azure.com", 1883);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_connect();
}

void mqtt_status()
{
  String payload = "{\"level\":" + String(level) + ", \"level_percent\":" + String(level_percent) + ", \"liters\":" + String(volume) + "}";
  mqtt_publish(MQTT_TOPIC_STATUS, payload.c_str());
}

unsigned long readDistance()
{
  unsigned long duration;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(100);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(150);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  return duration / 5.82; //This is in mm
}

void setup()
{
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  lcd.init();         //initialize the lcd
  lcd.backlight();    //open the backlight
  wifi_connect();
  mqtt_start();
  statusTimer = millis();
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  sensorAvg.begin(SMOOTHED_AVERAGE, 12);
  sensorExp.begin(SMOOTHED_EXPONENTIAL, 12);
}

void loop()
{
  //Conectar wifi si se desconecta
  if (WiFi.status() != WL_CONNECTED)
  {
    wifi_connect();
  }

  // Verifica que mqtt esta conectado
  mqttConnected = mqtt_client.connected();
  if (!mqttConnected)
  {
    mqtt_connect();
  }
  mqtt_client.loop();

  bottom = min_distance + max_distance;

  currentSensorValue = readDistance();
  if(sensorAverage){
    sensorAvg.add(currentSensorValue);
    level = bottom - sensorAvg.get();
  }
  else{
    sensorExp.add(currentSensorValue);
    level = bottom - sensorExp.get();
  }
  
  level_factor = level / max_distance;
  level_percent = int(100 * level_factor);
  volume = level_factor * max_volume;

  Serial.println("Level: " + String(currentSensorValue) + "mm, " + String(level_percent) + "%, volume: " + String(volume));

  lcd.setCursor(0, 0);
  lcd.print("Lvl:");
  lcd.setCursor(4, 0);
  lcd.print("    ");
  lcd.setCursor(4, 0);
  lcd.print(int(level));
  lcd.setCursor(8, 0);
  lcd.print("mm");
  lcd.setCursor(10, 0);
  lcd.print("      ");
  lcd.setCursor(12, 0);
  lcd.print(level_percent);
  lcd.setCursor(15, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Vol:");
  lcd.setCursor(4, 1);
  lcd.print("    ");
  lcd.setCursor(4, 1);
  lcd.print(int(volume));
  lcd.setCursor(8, 1);
  lcd.print("L       ");

  if (enableMqtt && (millis() - statusTimer > (timerMqtt * SECONDS)))
  {
    mqtt_status();
    statusTimer = millis();
  }

  delay(2000);
}