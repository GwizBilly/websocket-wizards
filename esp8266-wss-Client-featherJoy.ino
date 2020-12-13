#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "Adafruit_seesaw.h"

Adafruit_seesaw ss;
#define BUTTON_RIGHT 6
#define BUTTON_DOWN  7
#define BUTTON_LEFT  9
#define BUTTON_UP    10
#define BUTTON_START 14
uint32_t button_mask = (1 << BUTTON_RIGHT)| 
                       (1 << BUTTON_DOWN) | 
                       (1 << BUTTON_LEFT) |
                       (1 << BUTTON_UP)   |
                       (1 << BUTTON_START);
#if defined(ESP8266)
  #define IRQ_PIN 2
  #define TFT_CS  3
  #define TFT_RST 12                                       
  #define TFT_DC  1
#endif

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
const char* ssid = "";
const char* password = "";
const char* websockets_connection_string = "wss://swimmable-swallow-5092.dataplicity.io/"; 
// This fingerprint(thumbfrint) is wizards at https://swimmable-swallow-5092.dataplicity.io/
//"58 3b ef be 28 e8 47 b1 02 3f 89 50 3b 67 70 7c b8 be 53 60"
const char wizards_ssl_thumbprint[] PROGMEM = "58 3b ef be 28 e8 47 b1 02 3f 89 50 3b 67 70 7c b8 be 53 60";
using namespace websockets;
void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;

void setup() {
    WiFi.begin(ssid, password);
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        delay(1000);
    }
    client.onMessage(onMessageCallback);    
    client.onEvent(onEventsCallback);
    client.setFingerprint(wizards_ssl_thumbprint);
    client.connect(websockets_connection_string);
    // Say the secret password
    client.send("Wizards rule");
    //client.ping();
  // setup seesaw
  if (!ss.begin(0x49)) {
    // "error"
    while(1);
  }
  ss.pinModeBulk(button_mask, INPUT_PULLUP);
  ss.setGPIOInterrupts(button_mask, 1);  
  pinMode(IRQ_PIN, INPUT);
  // setup tft display
  tft.init(240, 240);  
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);  
  tft.setCursor(0, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(true);
  tft.setTextSize(5);
  //tft.print("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ");
}
bool moveMade = false;

int last_x = 0, last_y = 0;
void loop() { 
  client.poll();
  if (moveMade) {
    client.send("stayPut");
    delay(3000);
    moveMade = false;
  } else {
    delay(100); 
  }  
  tft.setCursor(0, 90);
  int y = ss.analogRead(2);
  int x = ss.analogRead(3);
  if (x > 600 && last_x < 600) {
    tft.fillTriangle(120, 30, 120, 50, 110, 40, ST77XX_WHITE);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Right");
  } else if (last_x > 600 && x < 600) {
    tft.fillTriangle(120, 30, 120, 50, 110, 40, ST77XX_BLACK);
    tft.setCursor(0, 90);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Right");
  }
  if (x < 400 && last_x > 400) {
    tft.fillTriangle(150, 30, 150, 50, 160, 40, ST77XX_WHITE);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Left");
  } else if (last_x < 400 && x > 400) {
    tft.fillTriangle(150, 30, 150, 50, 160, 40, ST77XX_BLACK);
    tft.setCursor(0, 90);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Left");
  }
  if (y > 600 && last_y < 600) {
    tft.fillTriangle(125, 26, 145, 26, 135, 16, ST77XX_WHITE);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0, 125);
    tft.print("Down");
  } else if (last_y > 600 && y < 600) {
    tft.fillTriangle(125, 26, 145, 26, 135, 16, ST77XX_BLACK);
    tft.setCursor(0, 125);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Down");
  }  
  if (y < 400 && last_y > 400) {
    tft.fillTriangle(125, 53, 145, 53, 135, 63, ST77XX_WHITE);
    tft.setCursor(0, 125);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Up");
  } else if (last_y < 400 && y > 400) {
    tft.fillTriangle(125, 53, 145, 53, 135, 63, ST77XX_BLACK);
    tft.setCursor(0, 125);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Up");
  }
  if ((abs(x - last_x) > 3) || (abs(y - last_y) > 3)) {
    //tft.print(x);
    //tft.setCursor(0, 0);
    //tft.print(y);
    last_x = x;
    last_y = y;
  } 
  if (!digitalRead(IRQ_PIN)) {
    moveMade = true;
    uint32_t buttons = ss.digitalReadBulk(button_mask);
    //Serial.println(buttons, BIN);
    if (!(buttons & (1 << BUTTON_DOWN))) {
      tft.fillCircle(30, 18, 10, ST77XX_GREEN);
    } else {
      tft.fillCircle(30, 18, 10, ST77XX_BLACK);
    }
    if (!(buttons & (1 << BUTTON_RIGHT))) {
      tft.fillScreen(ST77XX_BLACK);  
      tft.fillCircle(10, 40, 10, ST77XX_RED);
    } else {
      tft.fillCircle(10, 40, 10, ST77XX_BLACK);
    }    
    if (!(buttons & (1 << BUTTON_LEFT))) {
      tft.fillCircle(50, 40, 10, ST77XX_BLUE);
    } else {
      tft.fillCircle(50, 40, 10, ST77XX_BLACK);
    }
    if (!(buttons & (1 << BUTTON_UP))) {
      tft.fillCircle(30, 57, 10, ST77XX_YELLOW);
    } else {
      tft.fillCircle(30, 57, 10, ST77XX_BLACK);
    }
    if (!(buttons & (1 << BUTTON_START))) {
      tft.fillCircle(80, 40, 7, ST77XX_WHITE);
    } else {
      tft.fillCircle(80, 40, 7, ST77XX_BLACK);
    }
  }
  delay(10);
}
