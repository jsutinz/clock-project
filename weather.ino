#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

char* ssid = "Fergus WiFi-504";
char* password = "RogersFergus";
int status = 0;
int contentLength = 0;

char* hostname = "api.open-meteo.com";
char* getReq = "/v1/forecast?latitude=43.482586&longitude=-80.559364&current=temperature_2m,weather_code&hourly=weather_code&timezone=America%2FNew_York&forecast_days=1";

String jsonBuffer;

void setup() {
  Serial.begin(9600);
  delay(5000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != 3) {
    // unsuccessful, retry in 4 seconds
    Serial.println("failed ... ");
    delay(1000);
    Serial.println("retrying ... ");
    WiFi.begin(ssid, password);
    delay(4000);
  }

  delay(500);

  if(WiFi.status() == 3) {
    Serial.println("Connected");
  }
  else {
    Serial.println("error wifi not connected");
  }
  
}

void loop() {
  for(int i = 0;i<2;i++) {
    // Check WiFi connection status
    if(WiFi.status() == 3){
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
      temp = obj["current"]["temperature_2m"];
      int currWeatherCode = obj["current"]["weather_code"];
      int futureWeatherCode = obj["hourly"]["weather_code"][12];

      if(temp == -1000) {
        break;
      }
      else {
        Serial.print("The current temp is: ");
        Serial.println(temp);
        Serial.println(" ");
        delay(10000);
      }
    }
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HttpClient http = HttpClient(client, serverName, 80);
    
  int httpResponseCode = http.get(getReq);
  
  String payload = "{}"; 
  
  if (httpResponseCode == 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.readString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  return payload;
}
