#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
//#include <NewPing.h>

#define TRIGGER_PIN 7 // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 6    // Arduino pin tied to echo pin on the ultrasonic sensor.

#define NUM_ROWS 2
#define NUM_COLUMNS 16

#define WIDTH 200  //ancho en cm
#define LENGTH 200 //ancho en cm
#define BASE_AREA WIDTH *LENGTH
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define MQTT_TOPIC_LINK "/level/monitor/link"
#define MQTT_TOPIC_STATUS "/level/monitor/status"
#define MQTT_MAX_ATTEMPS 10

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27, NUM_ROWS, NUM_COLUMNS); // set the LCD address to 0x27 for a 16 chars and 2 line display
//NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

int level = 0;
int liters = 0; //volumen en litros
int level_percent = 100;

bool mqttConnected = false;
String DeviceID = "";

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

//DUMMY LEVEL DATA
int dummy_level = 0;
bool up = true;

int simularNivel()
{
  if (up)
  {
    dummy_level++;
    if (dummy_level > 199)
    {
      up = false;
    }
  }
  else
  {
    dummy_level--;
    if (dummy_level < 1)
    {
      up = true;
    }
  }

  return dummy_level;
}

void wifi_connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("AUTOM", "since1968");
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




void mqtt_status(){
  String payload = "{\"level\":" + String(level_percent) + ", \"liters\":" + String(liters) +"}";
  mqtt_publish(MQTT_TOPIC_STATUS, payload.c_str());
}

void setup()
{
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  lcd.init();         //initialize the lcd
  lcd.backlight();    //open the backlight
  wifi_connect();
  mqtt_start();
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

  //level =  MAX_DISTANCE - sonar.ping_cm();
  level = MAX_DISTANCE - simularNivel();
  level_percent = map(level, 0, MAX_DISTANCE, 0, 100);
  liters = level * BASE_AREA / 1000;

  Serial.println("Level: " + String(level) + "cm, " + String(level_percent) + "%, Liters: " + String(liters));

  lcd.setCursor(0, 0);
  lcd.print("Liters=");
  lcd.setCursor(8, 0);
  lcd.print("       ");
  lcd.setCursor(8, 0);
  lcd.print(liters);
  lcd.setCursor(14, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Level=");

  lcd.setCursor(6, 1);
  lcd.print("    "); // clears 8, 9 & 10 position
  lcd.setCursor(6, 1);
  lcd.print(level);
  lcd.setCursor(10, 1);
  lcd.print("cm");
  lcd.setCursor(13, 1);
  lcd.print("   ");
  lcd.setCursor(13, 1);
  lcd.print(level_percent);

  mqtt_status();

  delay(5000);
}