/*
  SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
  This example code is released into the public domain

  (New in SmartMatrix Library 4.0) To update a SmartMatrix Library sketch to use Adafruit_GFX compatible layers:

  - Make sure you have the Adafruit_GFX Library installed in Arduino (you can use Arduino Library Manager)
  - add `#define USE_ADAFRUIT_GFX_LAYERS` at top of sketch (this is needed for any sketch to tell SmartMatrix Library that Adafruit_GFX is present, not just this sketch)
    - Add this *before* #include <SmartMatrix.h>
    - Check the documentation Wiki for more details on why you may or may not want to use these layers
*/

//#define USE_ADAFRUIT_GFX_LAYERS

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#include <WiFi.h>
#include <esp_now.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 64;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_64ROW_MOD32SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#ifdef USE_ADAFRUIT_GFX_LAYERS
  // there's not enough allocated memory to hold the long strings used by this sketch by default, this increases the memory, but it may not be large enough
  SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, 6*1024, 1, COLOR_DEPTH, kScrollingLayerOptions);
#else
  SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
#endif

SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

#include "colorwheel.c"
#include "gimpbitmap.h"

unsigned long lastRefreshTime = 0; // Variable pour stocker le temps du dernier rafraîchissement
const unsigned long refreshInterval = 5000; // Intervalle de rafraîchissement en millisecondes (5 secondes)

const int defaultBrightness = (100*255)/100;        // full (100%) brightness
//const int defaultBrightness = (15*255)/100;       // dim: 15% brightness
const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = {0, 0, 0};

// Define LED and pushbutton state booleans
bool buttonDown = false;
bool ledOn = false;
int buttonState = 0;

// Define LED Active pin
const int ledActive1 = 19;
const int ledActive2= 18;

unsigned long lastReceive1;
unsigned long lastReceive2;
unsigned long lastReceive3;
unsigned long lastReceive4;
unsigned long lastReceive5;


// Define data structure
typedef struct struct_message {
    int a; //Button State
    int b;  // State 'dispo' led
    int c;  // ESP32 id
    int d;  // State 'indispo' led

} struct_message;

// Create structured data object
struct_message myData;

// Callback function
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{

  if (myData.c ==1 ){
    lastReceive1 = millis();
  }
  if (myData.c ==2 ){
    lastReceive2 = millis();
  }
  if (myData.c ==3 ){
    lastReceive3 = millis();
  }
  if (myData.c ==4 ){
    lastReceive4 = millis();
  }
  if (myData.c ==5 ){
    lastReceive5 = millis();
  }  

  // Get incoming data
  memcpy(&myData, incomingData, sizeof(myData));
  
  // Print to Serial Monitor
  Serial.print("Button");
  Serial.print(myData.c);
  Serial.print(": ");
  Serial.print(myData.a);
    
  Serial.print(" - led_dispo");
  Serial.print(myData.c);
  Serial.print(": ");
  Serial.print(myData.b); 

  Serial.print(" - led_indispo");
  Serial.print(myData.c);
  Serial.print(": ");
  Serial.println(myData.d); 

 
}




// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      rgb24 pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };

      backgroundLayer.drawPixel(x + j, y + i, pixel);
    }
  }
}

// the setup() method runs once, when the sketch starts
void setup() {

  WiFi.mode(WIFI_STA);
  

  // Initalize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.begin(115200);

    pinMode(ledActive1, OUTPUT);
  digitalWrite(ledActive1, LOW);
  pinMode(ledActive2, OUTPUT);
  digitalWrite(ledActive2, LOW);

  lastReceive1 = millis();
  lastReceive2 = millis();
  lastReceive3 = millis();
  lastReceive4 = millis();
  lastReceive5 = millis();
  

  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.addLayer(&indexedLayer); 
  matrix.begin();

  

  

  scrollingLayer.setOffsetFromTop(defaultScrollOffset);

  backgroundLayer.enableColorCorrection(true);
  esp_now_register_recv_cb(OnDataRecv);

    if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  

}

#define DEMO_INTRO              1
#define DEMO_DRAWING_INTRO      1
#define DEMO_DRAWING_PIXELS     1
#define DEMO_DRAWING_LINES      1
#define DEMO_DRAWING_TRIANGLES  1
#define DEMO_DRAWING_CIRCLES    1
#define DEMO_DRAWING_RECTANGLES 1
#define DEMO_DRAWING_ROUNDRECT  1
#define DEMO_DRAWING_FILLED     1
#define DEMO_FILL_SCREEN        1
#define DEMO_DRAW_CHARACTERS    1
#define DEMO_FONT_OPTIONS       1
#define DEMO_MONO_BITMAP        1
#define DEMO_SCROLL_COLOR       1
#define DEMO_SCROLL_MODES       1
#define DEMO_SCROLL_SPEED       1
#define DEMO_SCROLL_FONTS       1
#define DEMO_SCROLL_POSITION    1
#define DEMO_SCROLL_ROTATION    1
#define DEMO_BRIGHTNESS         1
#define DEMO_RAW_BITMAP         1
#define DEMO_COLOR_CORRECTION   1
#define DEMO_BACKGND_BRIGHTNESS 1
#define DEMO_INDEXED_LAYER      1
#define DEMO_REFRESH_RATE       1
#define DEMO_READ_PIXEL         1
#define DEMO_COLOR              1


unsigned long previousMillis = 0;
unsigned long interval = 100; // Temps d'attente en millisecondes


// the loop() method runs over and over again,
// as long as the board has power
void loop() {
  int i, j;
  unsigned long currentMillis;

  

  // clear screen
  backgroundLayer.fillScreen(defaultBackgroundColor);
  //backgroundLayer.swapBuffers();



unsigned long currentTime = millis(); // Obtenir le temps actuel

 indexedLayer.setFont(font5x7);
  indexedLayer.drawString(5, 5, 1, "STEPHANE");
  indexedLayer.drawString(5, 13, 1, "VANESSA");
  indexedLayer.drawString(5, 21, 1, "AXEL");
  indexedLayer.drawString(5, 29, 1, "RASHA");
  indexedLayer.drawString(5, 37, 1, "COACH");
indexedLayer.setFont(font3x5);
  

   // pour dessiner un carré 

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            // draw for 100ms, then update frame, repeat
    
       
            int x0, y0, x1, y1;
            // x0,y0 pair is always on the screen
 x0 = 5;
 y0 = 44;
 x1 = x0 + 4;
 y1 = y0 + 4;

            // radius is positive, up to screen width size


             //orange
            rgb24 Color = {0xFF, 0x00, 0x00};
            

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
            //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);



indexedLayer.drawString(11, 44, 1, "absent");

// pour dessiner un carré 

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            // draw for 100ms, then update frame, repeat
    
       
            int x2, y2, x3, y3;
            // x0,y0 pair is always on the screen
 x2 = 5;
 y2 = 51;
 x3 = x2 + 4;
 y3 = y2 + 4;

            // radius is positive, up to screen width size


             //orange
            rgb24 Color1 = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x2, y2, x3, y3, Color1);
              

            
            
            //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);

indexedLayer.drawString(11, 51, 1, "indisponible");

// pour dessiner un carré 

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            // draw for 100ms, then update frame, repeat
    
       
            int x5, y5, x6, y6;
            // x0,y0 pair is always on the screen
 x5 = 5;
 y5 = 58;
 x6 = x5 + 4;
 y6 = y5 + 4;

            // radius is positive, up to screen width size


             //orange
            rgb24 Color2 = {0x00, 0xFF, 0x00};
            

           

          
             backgroundLayer.fillRectangle(x5, y5, x6, y6, Color2);
              

            
            
            //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);

indexedLayer.drawString(11, 58, 1, "disponible");













// Check if the signal is still active

      if(millis() - lastReceive1 > 900) {
        // j'ai rien reçu depuis 3 secondes donc pas stéphane
        //indexedLayer.swapBuffers();

        // pour dessiner un carré 

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            // draw for 100ms, then update frame, repeat
    
       
            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 5;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


             //orange
            rgb24 Color = {0xFF, 0x00, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
            //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        

       
      }

      else {
         matrix.setBrightness(100); // 10% brightness, for a dark room
     
 


if(myData.c == 1){

if(myData.d == 1 ){
  // pour dessiner un carré 

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            // draw for 100ms, then update frame, repeat
    
       
            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 5;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


             //orange
            rgb24 Color = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
            //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        

  
  }

  if(myData.b == 1){
  // pour dessiner un carré

        
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);


            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 5;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


            //vert
       
            rgb24 Color = {0x00, 0xFF, 0x00};

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
             //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  }
}






      if(millis() - lastReceive2 > 900) {

  

      
      }

      else {
        digitalWrite(ledActive2, LOW);
 



if(myData.c == 2){

if(myData.d == 1 ){
  // pour dessiner un carré
 
       
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

 
            int x6, y6, x7, y7;
            // x0,y0 pair is always on the screen
 x6 = 45;
 y6 = 0;
 x7 = x6 + 5;
 y7 = y6 + 5;

            // radius is positive, up to screen width size
      

            //orange
            rgb24 Color4 = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x6, y6, x7, y7, Color4);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        

  
  }

  if(myData.b == 1){
  // pour dessiner un carré
  
    
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);


            int x6, y6, x7, y7;
            // x0,y0 pair is always on the screen
 x6 = 45;
 y6 = 0;
 x7 = x6 + 5;
 y7 = y6 + 5;

            // radius is positive, up to screen width size


            //vert
            rgb24 Color4 = {0x00, 0xFF, 0x00};

           

          
             backgroundLayer.fillRectangle(x6, y6, x7, y7, Color4);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  }




 
 

  
          
      }
      



      if(millis() - lastReceive3 > 900) {
         // j'ai rien reçu depuis 3 secondes donc pas stéphane

        //indexedLayer.swapBuffers();




      }

      else {
   
         matrix.setBrightness(100); // 10% brightness, for a dark room
        
        
   




if(myData.c == 3){

if(myData.d == 1 ){
  // pour dessiner un carré
 
       
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 17;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


            //orange
            rgb24 Color = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        

  
  }

  if(myData.b == 1){
  // pour dessiner un carré
  
      
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);


            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 17;
 x1 = x0 + 5;
 y1 = y0 + 5;



            //vert
            rgb24 Color = {0x00, 0xFF, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  
}



 
 

  
          
      }
      



      if(millis() - lastReceive4 > 900) {
      // j'ai rien reçu depuis 3 secondes donc pas stéphane

        //indexedLayer.swapBuffers();



      }

      else {
   
         matrix.setBrightness(100); // 10% brightness, for a dark room
        
        
        




if(myData.c == 4){

if(myData.d == 1 ){
  // pour dessiner un carré
 
   
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 25;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


            //orange
            rgb24 Color = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           // backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  

  if(myData.b == 1){
  // pour dessiner un carré
  

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);

            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 25;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size


            //vert
            rgb24 Color = {0x00, 0xFF, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  
}



 
 

  
          
      }

  




      if(millis() - lastReceive5 > 900) {
      // j'ai rien reçu depuis 3 secondes donc pas stéphane

        //indexedLayer.swapBuffers();




      }

      else {
   
         matrix.setBrightness(100); // 10% brightness, for a dark room
        
        





if(myData.c == 5){

if(myData.d == 1 ){
  // pour dessiner un carré
 
  
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);


            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 33;
 x1 = x0 + 5;
 y1 = y0 + 5;

            // radius is positive, up to screen width size
            radius = random(matrix.getScreenWidth());
            radius2 = random(matrix.getScreenWidth());

            //orange
            rgb24 Color = {0xFF, 0x80, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           // backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  

  if(myData.b == 1){
  // pour dessiner un carré
  
       
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);


            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
 x0 = 45;
 y0 = 33;
 x1 = x0 + 5;
 y1 = y0 + 5;



            //vert
            rgb24 Color = {0x00, 0xFF, 0x00};

           

          
             backgroundLayer.fillRectangle(x0, y0, x1, y1, Color);
              

            
            
           //backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
         //   while (millis() < currentMillis + delayBetweenShapes);
        }

  
  
}



 
 

  
          
      }


indexedLayer.swapBuffers(); 
backgroundLayer.swapBuffers();


}
  



  
