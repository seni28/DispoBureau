/*
  ESP-NOW Remote Sensor - Transmitter (Multiple Version)
  esp-now-xmit-multiple.ino
  Sends Temperature & Humidity data to other ESP32 via ESP-NOW
  Uses DHT22
  Multiple Transmitter modification
  
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/

// Include required libraries
#include <WiFi.h>
#include <esp_now.h>


// Define LED and pushbutton state booleans
bool buttonDown = false;
bool ledOn = false;

//Debounce
unsigned long lastActionTime = 0; 
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 200;    // the debounce time; increase if the output flickers
 
// Define LED and pushbutton pins
// Define LED and pushbutton pins

#define LED_ID1_DISPO 17
#define LED_ID1_INDISPO 18
#define LED_DEBUG 16
#define LED_BUTTON 34


#define STATUS_BUTTON 5
 

// Variables for temperature and humidity
int buttonState = 0 ;
int led_dispo = LOW;
int led_indispo = LOW;

// Integer for identification (make unique for each transmitter)
int ident = 5;

// Responder MAC Address (Replace with your responders MAC Address)
uint8_t broadcastAddress[] = {0x8, 0xB6, 0x1F, 0x3D, 0x1C, 0x8C};

// Define data structure
typedef struct struct_message {
  int a;
  int b;
  int c;
  int d;

} struct_message;

// Create structured data object
struct_message myData;

// Register peer
esp_now_peer_info_t peerInfo;

// Sent data callback function
void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status)
{
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status != ESP_NOW_SEND_SUCCESS){
    digitalWrite(LED_DEBUG, true);

  }

  else{
    digitalWrite(LED_DEBUG, false);
  }
}

void setup() {

  // Setup Serial monitor
  Serial.begin(115200);
  delay(100);



  // Set ESP32 WiFi mode to Station temporarly
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Define callback
  esp_now_register_send_cb(OnDataSent);


  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

    // Pushbutton uses built-in pullup resistor
  pinMode(STATUS_BUTTON, INPUT_PULLUP);
 
  // LED Output
  pinMode(LED_ID1_DISPO, OUTPUT);
  pinMode(LED_DEBUG, OUTPUT);
  pinMode(LED_BUTTON, OUTPUT);
  pinMode(LED_ID1_INDISPO, OUTPUT);
  
  digitalWrite(LED_ID1_DISPO, true);
  digitalWrite(LED_ID1_INDISPO, false);
  digitalWrite(LED_DEBUG, false);

}

void loop() {

      digitalWrite(LED_BUTTON, true);

  if ( (millis() - lastDebounceTime) > debounceDelay) {

    buttonState = digitalRead(STATUS_BUTTON);
    led_dispo = digitalRead(LED_ID1_DISPO);
    led_indispo = digitalRead(LED_ID1_INDISPO);

    Serial.print("button: ");
    Serial.println(buttonState);
    Serial.print("led_dispo: ");
    Serial.println(led_dispo);
    Serial.print("led_indispo: ");
    Serial.println(led_indispo);

    // Add to structured data object
    myData.a = buttonState;
    myData.b = led_dispo;
    myData.c = ident;
    myData.d = led_indispo;

   

    if (buttonState== 0 && !buttonDown){

      Serial.print("Data a =0");
      //Now we set that the button has been pressed
      buttonDown=true;
      buttonState=1;


      if (ledOn == false && led_dispo == 0) {
        ledOn = true;
        digitalWrite(LED_ID1_DISPO, ledOn);
        Serial.println("LED Dispo turned ON");

        if (led_indispo == 1) {
          digitalWrite(LED_ID1_INDISPO, false);
          Serial.println("LED Indispo turned OFF");
        }
      }


         
    // Check if the 'Dispo' Led is ON AND the button was pressed in ESP id = 1
    if (led_dispo == 1){

      Serial.println("Le bouton a été appuyé et la LED dispo est déjà allumée");
      ledOn=true;
      digitalWrite(LED_ID1_INDISPO,ledOn);
      digitalWrite(LED_ID1_DISPO,false);
      Serial.println("LED Dispo turned OFF & Indispo");

    }
      
  

    }
    else
    {
      // Reset the button state
      buttonDown = false;
      ledOn = false;
    
      //digitalWrite(LED_ID2_DISPO, ledOn);
      //digitalWrite(LED_ID2_INDISPO, ledOn);

    }


    // Send data
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    lastDebounceTime = millis();
  }
}
