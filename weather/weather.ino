#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char* ssid = "Fergus WiFi-504";
char* password = "RogersFergus";
// char* ssid = "I cry when IP";
// char* password = "kvln5318";
int contentLength = 0;

char* hostname = "api.open-meteo.com";
char* getReq = "/v1/forecast?latitude=43.482586&longitude=-80.559364&current=temperature_2m,apparent_temperature,weather_code&hourly=weather_code&timezone=America%2FNew_York&forecast_days=1";

String jsonBuffer;

#define BMP_WIDTH 32
#define BMP_HEIGHT 32
#include <logo.h>


void setup() {
  Serial.begin(9600);
  delay(5000);

  wifiSetup();

  delay(500);

  if(WiFi.status() == 3) {
    Serial.println("Connected");
  }
  else {
    Serial.println("error wifi not connected");
  }
  
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  for(int i = 0;i<2;i++) {
    // Check WiFi connection status
    if(WiFi.status() == 3) {

      delay(1000);
      String buffer = httpGETRequest(hostname);
      jsonBuffer = buffer.substring(buffer.indexOf("{"), buffer.lastIndexOf("}")+1);
      //Serial.println(jsonBuffer);
      
      JsonDocument obj;
      DeserializationError error = deserializeJson(obj, jsonBuffer);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        delay(500);
        return;
      }
    
      double temp = -1000;
      double feelsLike = -1000;
      temp = obj["current"]["temperature_2m"];
      feelsLike = obj["current"]["apparent_temperature"];
      int currWeatherCode = obj["current"]["weather_code"];
      int futureWeatherCode = obj["hourly"]["weather_code"][12];


      if(temp == -1000) {
        Serial.println("error in reading temp.");
        delay(1000);
        break;
      }
      else {
        Serial.print("The current temp is: ");
        Serial.println(temp);
        Serial.println(" ");

        display.setTextSize(3);           
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(65,10);             // Start to the right of screen center
        display.cp437(true);         // Use full 256 char 'Code Page 437' font

        display.print((int)round(temp));
        display.setTextSize(1);
        display.print("o");
        display.setTextSize(3);
        display.println("C");
        
        display.setCursor(20, 50);
        display.setTextSize(1);
        display.print("Feels like: ");
        display.print((int)round(feelsLike)); //
        if((int)round(feelsLike) > 9 || (int)round(feelsLike) < 0) {
          display.drawCircle(107, 50, 1, 1);
        }
        else {
          display.drawCircle(101, 50, 1, 1);
        }
        
        display.println(" C");
        

        if(currWeatherCode != futureWeatherCode) {
          displayWeatherBmp(currWeatherCode);
        }
        else {
          displayWeatherBmp(futureWeatherCode);
        }

        display.display();
        delay(10000);
        display.clearDisplay();
      }
    }
  }
}

void displayWeatherBmp(int weatherCode) {
  Serial.print("weather code: ");
  Serial.println(weatherCode);
  if(weatherCode == 0 || weatherCode == 1) { // Sunny
    display.drawBitmap(0, 0, sun_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 2) { // Partly Cloudy
    display.drawBitmap(0, 0, partlyCloudy_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 3) { // Cloudy
    display.drawBitmap(0, 0, cloudy_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 45 || weatherCode == 48) { // Fog
    display.drawBitmap(0, 0, fog_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 51 || weatherCode == 53 || weatherCode == 55 || weatherCode == 56 || weatherCode == 61 || weatherCode == 63 || weatherCode == 65 || weatherCode == 66 || 
  weatherCode == 67 || weatherCode == 80 || weatherCode == 81 || weatherCode == 82) { // Rain
    display.drawBitmap(0, 0, rain_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 71 || weatherCode == 73 || weatherCode == 75 || weatherCode == 77 || weatherCode == 85 || weatherCode == 86) { // Snow
    display.drawBitmap(0, 0, snow_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else if(weatherCode == 95) { // Thunderstorms
    display.drawBitmap(0, 0, thunderstorm_bmp, BMP_WIDTH, BMP_HEIGHT, 1);
  }
  else {
    Serial.println("error: no valid weather code."); // error condition
  }
  return;
}

void wifiSetup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != 3) {
    // unsuccessful, retry in 4 seconds
    Serial.println("failed ... ");
    delay(1000);
    Serial.println("retrying ... ");
    WiFi.begin(ssid, password);
    delay(4000);
  }
  return;
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HttpClient http = HttpClient(client, serverName, 80);

  int httpResponseCode = http.get(getReq);
  delay(1000);
  
  String payload = "{}"; 
  
  if (httpResponseCode == 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.readString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);

    int numRestarts = 0;
    while(httpResponseCode != 0 & numRestarts < 20) {
      WiFiClient client;
      HttpClient http = HttpClient(client, serverName, 80);
      httpResponseCode = http.get(getReq);
      Serial.println("retrying HTTP connection...");
      delay(1000);
      numRestarts++;
    }
    if(numRestarts == 20) {
      Serial.println("could not connect after 20 retrys.");
      wifiSetup();
      Serial.println(WiFi.status());
    }
    else {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.readString();
      Serial.println(payload);
    }

  }
  http.stop();
  return payload;
}
