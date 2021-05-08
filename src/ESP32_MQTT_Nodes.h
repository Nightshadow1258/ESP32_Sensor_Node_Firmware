// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

#define SLEEP 1
#define BME_connected 1
#define Doorsensor_connected 1
#define Battery_powered 1

// -----------------------------------------------------------------------------
// Libraries
// -----------------------------------------------------------------------------

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

// -----------------------------------------------------------------------------
// Variables
// -----------------------------------------------------------------------------
float temperature, humidity, pressure, altitude;
int light;
#if BME_connected
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME280_ADDRESS  (0x76) //0x76 for violett PCB | 0x77 for blue PCB!!
Adafruit_BME280 bme;
#define BME_Power_Pin 32
#endif

/*Put your SSID & Password*/
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASS "WIFI_PASSWORD"

#define mqtt_server "MQTT_SERVER_IP"
#define mqtt_port 1883
#define mqtt_user "MQTT_USERNAME"
#define mqtt_password "MQTT_PASSWORD"

WiFiClient espClient;
PubSubClient client(espClient);

//MQTT_Topics
/*
Node 1 → Wohnzimmer
Node 2 → Arbeitszimmer
Node 3 → Schlafzimmer
Node 4 → Badezimmer
Node 6 → Terrasse
*/

#if BME_connected
#define humidity_topic "/home/sensors/hum/node1"
#define temperature_topic "/home/sensors/temp/node1"
#define pressure_topic "/home/sensors/pres/node1"
#define battery_topic "/home/sensors/bat/node1"
#define altitude_topic "/home/sensors/alt/node1"
#endif


//Sleep constants
#if SLEEP
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define S_TO_M_FACTOR 60
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (Minutes) */
RTC_DATA_ATTR int bootCount = 0;
#endif


#if Doorsensor_connected
const int Doorsensorpin = 4;
int Doorstate; // 0 close - 1 open wwitch
#define doorsensor_topic "/home/sensors/door/node1"
#endif
  
//light sensor
#define lightpin 34
//#define lightpindigital 25

//motion sensor
#define motionpin 23              // the pin that the sensor is atteched to

#if Battery_powered
//Battery pin
#define batpin 2
float batvolt=0;
#endif
// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void readsensordata(){

#if BME_connected
  bme.begin(0x76);   //initalize Sensor communication
  pressure = bme.readPressure() / 100.0F;
  temperature = bme.readTemperature();
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  humidity = bme.readHumidity();
#endif
  light = map(analogRead(lightpin),0,4095,100,0);

#if Battery_powered
  batvolt = analogRead(batpin);
  Serial.println(batvolt);
  batvolt = mapf(batvolt, 0.0, 4095, 0.0, 4.2); //Prozent angabe zwischen 3V und 4.2V
#endif

}


void printsensordata(){
#if BME_connected
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(altitude);
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");
#endif


#if Battery_powered
    Serial.print("Batteryvoltage = ");
    Serial.print(batvolt);
    Serial.println(" V");
#endif

    Serial.println();
}

void pushtopics(){
  
  client.connect(mqtt_server, mqtt_user, mqtt_password);
  #if BME_connected
  client.publish(temperature_topic, String(temperature).c_str(), true);
  client.publish(humidity_topic, String(humidity).c_str(), true);
  client.publish(altitude_topic, String(altitude).c_str(), true);
  client.publish(pressure_topic, String(pressure).c_str(), true);
  #endif

  #if Doorsensor_connected
  client.publish(doorsensor_topic, String(Doorstate).c_str(), true);
  #endif
  #if Battery_powered
  client.publish(battery_topic, String(batvolt).c_str(), true);
  #endif
  delay(50);
  }


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  // Feel free to add more if statements to control more GPIOs with MQTT

  Serial.print(String(topic));
  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message

}


#if Doorsensor_connected

void DoorSensor(){
Doorstate = digitalRead(Doorsensorpin);
  if (Doorstate == HIGH){
      Serial.println("State HIGH");
    }
    else{
      Serial.println("State LOW");
    }
}
#endif
