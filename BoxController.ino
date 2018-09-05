#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN D3 // D3 is a pin for LED Strip

//FILL REAL VALUES
const char* ssid = "Muspelheim";
const char* password = "******";

const char* mqtt_server = "192.168.0.170";
const char* mqtt_user = "******";
const char* mqtt_password = "******";

int servoPosition = 0;
int servoPin = 2; // Arduino GIO2 equal D4 ESP Pin
int isAlreadyInited = 0;
int numberOfLedPixels = 24;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberOfLedPixels, PIN, NEO_GRB + NEO_KHZ800);
//Create new servo object to control box open/close
Servo servo;
//Create new web server instance
ESP8266WebServer server(80);
//Pub-Sub CLient
WiFiClient espClient;
PubSubClient pubSubClient(espClient);

//Setup runned once
void setup() 
{ 
  strip.setBrightness(100);
  strip.begin();
  strip.show();

  //Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pubSubClient.setServer(mqtt_server, 1883);
  pubSubClient.setCallback(pubSubCallback);

  server.onNotFound(handleRoot);
  server.on("/", handleRoot);
  server.on("/open", [](){
    openBox();
    server.send(200, "text/plain", "box open");
  });

  server.on("/close", [](){
    closeBox();
    server.send(200, "text/plain", "box open");
  });

  server.begin();

  rainbowCycle(5); //first cycle before servo (change color every 5ms)
  servo.attach(servoPin);  // attaches the servo on GIO2 to the servo object 
} 

//Loop runned without stop
void loop() 
{ 
  if(isAlreadyInited < 1) {
    connectPubSubAndProducePayload();
    
    for(int i=0;i<numberOfLedPixels;i++)
    {
      // 220-20-60 is a red color
      strip.setPixelColor(i, strip.Color(220, 20, 60));
    } 
    strip.show();

    //moved to custom function
    openBox();

    isAlreadyInited = 1;

    // 65% strip brightness after opening
    strip.setBrightness(65);
  }

  server.handleClient();
  
  rainbowCycle(20);
} 

//------------------------------ PUB-SUB Logic -----------------------------
long lastReconnectAttempt = 0;

boolean pubSubReconnect() {
  if (pubSubClient.connect("WeddingBox", mqtt_user, mqtt_password)) {
    // Once connected, publish an announcement...
    pubSubClient.publish("wedding/proposal-box/start","on");
  }
}

void connectPubSubAndProducePayload() {
  if (!pubSubClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (pubSubReconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    pubSubClient.loop();
  }
}

void pubSubCallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

//------------------------------ SERVO OPEN/CLOSE -----------------------------
int boxOpened = 0;

void openBox() {
  if(boxOpened == 0)
  {
    servo.attach(servoPin);  // attaches the servo on GIO2 to the servo object 
    
    for(servoPosition = 90; servoPosition <= 180; servoPosition += 1)
    {
      servo.write(102); //servo speed (slow)
      delay(35); // send SIGNAL 35 ms
    } 
  
    servo.detach();

    boxOpened = 1;
  }
}

void closeBox() {
  if(boxOpened == 1)
  {
    servo.attach(servoPin);  // attaches the servo on GIO2 to the servo object 
    
    for(servoPosition = 90; servoPosition <= 180; servoPosition += 1)
    {
      servo.write(78); //servo speed (slow)
      delay(35); // send SIGNAL 35 ms
    } 
  
    servo.detach();

    boxOpened = 0;
  }
}

//------------------------------ WEB SERVER -----------------------------------
void handleRoot() {
  server.send(200, "text/html", "<p><a href='/open'>Open Box</a></p><p><a href='/close'>Close Box</a></p>");
}


//--------------------------------- LED ---------------------------------------
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);

    server.handleClient();
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//--------------------------------- END LED -----------------------------------
