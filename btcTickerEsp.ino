//Project Created by Michael Kern AKA powerpoint45 AKA immawake
//Outer Neopixel ring main IN is connected to IO16 on ESP32
//Display SDA connected to IO21 (which is SDA on the esp32)
//Output of outer LED ring connects to input of inner LED Ring
//Display SCL connected to IO22 (which is SCL on the esp32)
//Make sure you enter your Wifi username and password below
//You must press reset after uploading your program in order for the Display to work. Seems to be an issue with the ESP32


#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>

#include <aJSON.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const String GDAX_URL = "https://api.gdax.com/products/BTC-USD/ticker";
const char* ssid = "wifiESSID";
const char* password = "wifiPassword";
HTTPClient http;

const int ledPin = 16;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(24, ledPin);

Adafruit_SSD1306 display(4);

float currentPrice = 0;

int ledBrightness = 5;
const int priceInterval = 99;

RgbColor red(ledBrightness, 0, 0);
RgbColor green(0, ledBrightness, 0);
RgbColor blue(0, 0, ledBrightness);
RgbColor orange(ledBrightness, ledBrightness/2, 0 );


void setup() {
  // put your setup code here, to run once:
  strip.Begin();
  strip.Show();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  
  
  Serial.begin(115200);
  delay(1000);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void loop() {
  // put your main code here, to run repeatedly:
  http.begin("https://api.gdax.com/products/BTC-USD/ticker");
  int httpCode = http.GET(); 

  delay(50);
  //move all pixels up one
  if (strip.GetPixelColor(0)!=blue){
      for (int i = 11; i>0; i--){
         strip.SetPixelColor(i, strip.GetPixelColor(i-1));
      }
      strip.SetPixelColor(1, strip.GetPixelColor(0));
  }

  delay(50);
  strip.SetPixelColor(0, orange);
  strip.Show();
  delay(50);
  
  strip.SetPixelColor(0, blue);
  strip.Show();
  delay(50);
  
  if (httpCode > 0) { //Check for the returning code

        String payload = http.getString();
        //Serial.println(httpCode);

        char jsonChars[payload.length()];
        payload.toCharArray(jsonChars, payload.length()+1);
         
        aJsonObject* jsonObject = aJson.parse(jsonChars);
        aJsonObject* price = aJson.getObjectItem(jsonObject, "price");
        String priceString = price->valuestring;
        float priceFloat = priceString.toFloat();

        delay(50);
        display.clearDisplay();
        display.setTextSize(4);
        display.setTextColor(WHITE);
        display.setCursor(0,5);
        display.println((int)priceFloat);
        display.setTextColor(BLACK, WHITE);
        delay(50);
        display.display();
        delay(50);

        int intensity = abs(currentPrice-priceFloat);
        if (intensity>priceInterval){
            intensity = intensity-priceInterval;
            
            if (intensity >255)
                intensity = 255;
                
            if (currentPrice == 0)
              strip.SetPixelColor(0, blue);
            else if (priceFloat>currentPrice)
              strip.SetPixelColor(0, RgbColor(0, intensity, 0));
            else if (priceFloat<currentPrice)
              strip.SetPixelColor(0, RgbColor(intensity, 0, 0));
            else
              strip.SetPixelColor(0, orange);
              
            currentPrice = priceFloat;
        }

        //clear all inner ring LED's
        for (int i = 0; i<12; i++){
          strip.SetPixelColor(i+12, 0);
        }

        if (intensity>0 && currentPrice!=0 && priceFloat!=currentPrice){
            if (intensity>100)
                intensity = 100;
            int numLedsLit = (12 * intensity) / priceInterval;
            if (numLedsLit>0){
                if (priceFloat>currentPrice){
                  for (int i = 0; i<numLedsLit; i++){
                      strip.SetPixelColor(i+12, RgbColor(0, ledBrightness, 0));
                  }
                }else if (priceFloat<currentPrice){
                  strip.SetPixelColor(12, RgbColor(ledBrightness, 0, 0));
                  numLedsLit --;
                  for (int i = 0; i<numLedsLit; i++){
                      strip.SetPixelColor(23-i, RgbColor(ledBrightness, 0, 0));
                  }
                }
            }
        }

        delay(50);
        strip.Show();
        delay(50);
        
        
        Serial.print(priceFloat);
        Serial.print(":");
        Serial.print(intensity);
        Serial.println("");
        aJson.deleteItem(jsonObject);

        //change this delay to set how often price is updated
        delay(50);
 
  }else {
        Serial.println("Error on HTTP request");
  }

 // delay(30000);
}


