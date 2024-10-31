// #include <Adafruit_GFX.h>
// #include <Adafruit_GrayOLED.h>
// #include <Adafruit_SPITFT.h>
// #include <Adafruit_SPITFT_Macros.h>
// #include <gfxfont.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <TimeLib.h>
#include <WiFiS3.h>
#include <Servo.h>

#define minuteDigit1MotorPin 8
#define minuteDigit2MotorPin 9
#define hourDigit1MotorPin 12
#define hourDigit2MotorPin 13
#define angleIncrement 36

int status = WL_IDLE_STATUS;

int rotDirection = 0;
int pressed = false;
char ssid[10] = "VIRGIN783";
char pass[13] = "9F34A456FAA1";

unsigned int localPort = 2390;
const int NTP_PACKET_SIZE = 48;

IPAddress timeServer(162, 159, 200, 123);
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

//Digit1 Digit2: Digit1 Digit2
Servo hourDigit1Servo; 
Servo hourDigit2Servo; 
Servo minuteDigit1Servo; 
Servo minuteDigit2Servo;

const int TIME_ZONE = -4;

void setup() {
  minuteDigit1Servo.attach(minuteDigit1MotorPin);
  minuteDigit2Servo.attach(minuteDigit2MotorPin);
  hourDigit1Servo.attach(hourDigit1MotorPin);
  hourDigit2Servo.attach(hourDigit2MotorPin);
  // pinMode(in1, OUTPUT);
  // pinMode(in2, OUTPUT);
  // pinMode(button, INPUT);
  // // Set initial rotation direction
  // digitalWrite(in1, LOW);
  // digitalWrite(in2, HIGH);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  // printCurrentNet();
  // printWifiData();

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);

}

void displayTime(int hours, int minutes) {
  int minDigit1 = minutes / 10;
  int minDigit2 = minutes % (minDigit1*10);
  int hrDigit1 = hours / 10;
  int hrDigit2 = hours % (hrDigit1*10);
  
  Serial.print("Hour Digit 1: ");
  Serial.println(hrDigit1);
   Serial.print("Hour Digit 2: ");
  Serial.println(hrDigit2);
  Serial.print("Minute Digit 1: ");
  Serial.println(minDigit1);
  Serial.print("Minute Digit 2: ");
  Serial.println(minDigit2);
  hourDigit1Servo.write(hrDigit1*angleIncrement);
  hourDigit2Servo.write(hrDigit2*angleIncrement);
  minuteDigit1Servo.write(minDigit1*angleIncrement);
  minuteDigit2Servo.write(minDigit2*angleIncrement);
}

void loop() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  int hour = 0;
  int minutes = 0;
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Seconds since Jan 1, 1970 = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long timeInSec = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(timeInSec);

    // print the hour, minute and second:
    Serial.print("The time is ");
  
    hour = (((timeInSec + TIME_ZONE*3600) % 86400L) / 3600);
    // print the hour (86400 equals secs per day)
    // if (hour < 10) 
    //   // In the first 10 minutes of each hour, we'll want a leading '0'
    //   Serial.print('0');
    // }
    Serial.print(hour); 
    Serial.print(':');

    minutes = (timeInSec  % 3600) / 60;
    Serial.print(minutes); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((timeInSec % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(timeInSec % 60); // print the second
  }
  // loop until 12 hrs is reached, then compare to the std time again.
  for(int i = 0;i < 12; i++) {
    for(int j = 0;j < 60;j++) {
      displayTime(hour,minutes);
      delay(6000); // wait one minute
      if(minutes < 60) {
        minutes++;
      }
      else if(minutes == 59) {
        minutes = 0;
        hour++;
        break;
      }
    }
  }
}


// Servo hourDigit1Servo; 
// Servo hourDigit2Servo; 
// Servo minuteDigit1Servo; 
// Servo minuteDigit2Servo;



// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}