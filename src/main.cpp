#include <Arduino.h>
#include "ESP32_MQTT_Nodes.h"

// -----------------------------------------------------------------------------
// setup
// -----------------------------------------------------------------------------

void setup() {  client.disconnect();
  Serial.begin(9600);
  delay(100);
  
  pinMode(BME_Power_Pin,OUTPUT);
  
  pinMode(lightpin, INPUT);
  pinMode(motionpin, INPUT);    // initialize sensor as an input
  #if LED_connected
  pinMode(LED_PIN, OUTPUT);    
  #endif
#if BME_connected
  bme.begin(0x76);   
#endif

#if battery_powered
  pinMode(battpin, INPUT);
#endif

  readsensordata();
  printsensordata();

  setup_wifi();

  Serial.println("HTTP server started");

  Serial.println("\nThis is the Node 1!\n");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  #if LED_connected
  // this resets all the neopixels to an off state
  strip.begin();
  strip.clear();
  strip.show();
  #endif
  #if Doorsensor_connected
  pinMode(Doorsensorpin, INPUT_PULLUP);
  #endif
}

// -----------------------------------------------------------------------------
// loop 
// -----------------------------------------------------------------------------

void loop() {

#if LED_connected
  if (!client.connected()) {
    reconnect();
  }
#endif
  client.loop();
  digitalWrite(BME_Power_Pin, HIGH);
  delay(50);
  readsensordata();
  printsensordata();
  digitalWrite(BME_Power_Pin, LOW);
  DoorSensor();
  pushtopics();
  //automaticlighton();
#if SLEEP
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * S_TO_M_FACTOR * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
#endif
}
