// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.  NO FUNCIONA BIEN???
// ---------------------------------------------------------------------------
int level = 0;
long ancho = 200; //ancho en cm
long largo = 200;  //largo en cm
long litros = 0;  //volumen en litros
long AreaBase =100;
int level_Perc=100;
// include the library code
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <NewPing.h>

#define TRIGGER_PIN  7  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     6  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight 
  pinMode(7,OUTPUT);
   pinMode(6,INPUT);
}

void loop() {
  level =  MAX_DISTANCE - sonar.ping_cm();
  level_Perc=map(level,0,MAX_DISTANCE,0,100);
  //volumen calculation
  //largo=200;
 // ancho=200;
  AreaBase=ancho*largo;
  litros=level*AreaBase/1000;
  delay(3000);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.print("cm  ");
  Serial.print(" AreaBase  ");
  Serial.print(AreaBase);
  Serial.print(" Largo  ");
  Serial.print(largo);
  Serial.print(" Litros  ");
  Serial.println(litros);
  

  lcd.setCursor(0,0);
  lcd.print("Litros=");
  lcd.setCursor(8,0);
  lcd.print("       ");
  lcd.setCursor(8,0);
  lcd.print(litros);
  lcd.setCursor(14,0);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("Nivel=");
 
  lcd.setCursor(6,1);
  lcd.print("    "); // clears 8, 9 & 10 position
  lcd.setCursor(6,1);
  lcd.print(level);
  lcd.setCursor(10,1);
  lcd.print("cm");
  lcd.setCursor(13,1);
  lcd.print("   ");
  lcd.setCursor(13,1);
  lcd.print(level_Perc);
  
delay(1000);

}
