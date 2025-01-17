#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS   15
#define TFT_RST  18
#define TFT_DC    2
#define TFT_MOSI 19
#define TFT_SCLK 12
#define UP_BUTTON_PIN 4
#define DOWN_BUTTON_PIN 5
#define SELECT_BUTTON_PIN 23
#define HOME_BUTTON_PIN 22

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Graph constant
#define WIDTH 320
#define HEIGHT 240
#define BANNER_HEIGHT 16
#define GRAPH_BASELINE (HEIGHT - 18)
#define GRAPH_HEIGHT (HEIGHT - 52)
#define CHANNEL_WIDTH (WIDTH / 16)

// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100
#define NEAR_CHANNEL_RSSI_ALLOW -70

// define color
#define TFT_WHITE   ST77XX_WHITE    /* 255, 255, 255 */
#define TFT_BLACK   ST77XX_BLACK    /*   0,   0,   0 */
#define TFT_RED     ST77XX_RED      /* 255,   0,   0 */
#define TFT_ORANGE  0xFD20          /* 255, 165,   0 */
#define TFT_YELLOW  ST77XX_YELLOW   /* 255, 255,   0 */
#define TFT_GREEN   ST77XX_GREEN    /*   0, 255,   0 */
#define TFT_CYAN    ST77XX_CYAN     /*   0, 255, 255 */
#define TFT_BLUE    ST77XX_BLUE     /*   0,   0, 255 */
#define TFT_MAGENTA ST77XX_MAGENTA  /* 255,   0, 255 */

// Channel color mapping from channel 1 to 14
uint16_t channel_color[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE
};

uint8_t scan_count = 0;

int selected_network = 0;
bool function1_active = false;
bool draw_menu_active = false;

void draw_menu() {
  draw_menu_active = true;
  // code to draw on the display here
  uint8_t ap_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int32_t max_rssi[] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  // clear old graph
  tft.fillRect(0, BANNER_HEIGHT, 320, 224, TFT_BLACK);
  tft.setTextSize(1);
  if (n == 0) {
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, BANNER_HEIGHT);
    tft.println("no networks found");
  } else {
    // plot found WiFi info
    for (int i = 0; i < n; i++) {
      int32_t channel = WiFi.channel(i);
      int32_t rssi = WiFi.RSSI(i);
      uint16_t color = channel_color[channel - 1];
      int height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, GRAPH_HEIGHT), 1, GRAPH_HEIGHT);

      // channel stat
      ap_count[channel - 1]++;
      if (rssi > max_rssi[channel - 1]) {
        max_rssi[channel - 1] = rssi;
      }

      tft.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);
      tft.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel + 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);

      // Print SSID, signal strengh and if not encrypted
      tft.setTextColor(color);
      tft.setCursor((channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE - 10 - height);
      tft.print(WiFi.SSID(i));
      tft.print('(');
      tft.print(rssi);
      tft.print(')');

      // rest for WiFi routine?
      delay(10);
    }
  }
  while(draw_menu_active) {
  // print WiFi stat
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, BANNER_HEIGHT);
  tft.print(n);
  tft.print(" networks channels: ");
  bool listed_first_channel = false;
  for (int i = 1; i <= 11; i++) { // channels 12-14 may not available
    if ((i == 1) || (max_rssi[i - 2] < NEAR_CHANNEL_RSSI_ALLOW)) { // check previous channel signal strengh
      if ((i == sizeof(channel_color)) || (max_rssi[i] < NEAR_CHANNEL_RSSI_ALLOW)) { // check next channel signal strengh
        if (ap_count[i - 1] == 0) { // check no AP exists in same channel
          if (!listed_first_channel) {
            listed_first_channel = true;
          } else {
            tft.print(", ");
          }
          tft.print(i);
        }
      }
    }
  }
  // draw graph base axle
  tft.drawFastHLine(0, GRAPH_BASELINE, 320, TFT_WHITE);
  for (int i = 1; i <= 14; i++) {
    tft.setTextColor(channel_color[i - 1]);
    tft.setCursor((i * CHANNEL_WIDTH) - ((i < 10)?3:6), GRAPH_BASELINE + 2);
    tft.print(i);
    if (ap_count[i - 1] > 0) {
      tft.setCursor((i * CHANNEL_WIDTH) - ((ap_count[i - 1] < 10)?9:12), GRAPH_BASELINE + 11);
      tft.print('(');
      tft.print(ap_count[i - 1]);
      tft.print(')');
    }
  }
    if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
      delay(200);
      draw_menu_active = false;
      function1();
    }
    if (digitalRead(HOME_BUTTON_PIN) == LOW) {
      delay(200);
      draw_menu();
    }
  }
}

void function1_details() {
    if (selected_network >= 0) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.println("Network:");
        tft.println(WiFi.SSID(selected_network));
        tft.println("BSSID: " + WiFi.BSSIDstr(selected_network));
        tft.println("Signal: " + String(WiFi.RSSI(selected_network)) + " dBm");
        tft.println("Channel: " + String(WiFi.channel(selected_network)));
    } else {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.println("No network selected");
        delay(1000);
    }
    while(function1_active) {
        if (digitalRead(HOME_BUTTON_PIN) == LOW) {
            delay(200);
            function1();
        }
    }
}

void function1() {
    function1_active = true;
    int n = WiFi.scanNetworks();
    delay(100);
    tft.fillScreen(TFT_BLACK);
    if (n == 0) {
    } else {
        tft.setTextSize(2);
        tft.setCursor(0, 0);
        for (int i = 0; i < n; i++) {
            if (i == selected_network) {
                tft.setTextColor(TFT_BLUE);
            } else {
                tft.setTextColor(TFT_WHITE);
            }
            tft.print(i + 1);
            tft.print(": ");
            tft.println(WiFi.SSID(i));
        }
    }
    while(function1_active) {
        if (digitalRead(UP_BUTTON_PIN) == LOW) {
            if (selected_network > 0) {
                selected_network--;
            }
            if (n == 0) {
            } else {
            tft.setTextSize(2);
            tft.setCursor(0, 0);
            for (int i = 0; i < n; i++) {
            if (i == selected_network) {
                tft.setTextColor(TFT_BLUE);
            } else {
                tft.setTextColor(TFT_WHITE);
            }
            tft.print(i + 1);
            tft.print(": ");
            tft.println(WiFi.SSID(i));
            }
          }
        }
        if (digitalRead(DOWN_BUTTON_PIN) == LOW) {
            if (selected_network < n - 1) {
                selected_network++;
            }
            if (n == 0) {
            } else {
            tft.setTextSize(2);
            tft.setCursor(0, 0);
            for (int i = 0; i < n; i++) {
            if (i == selected_network) {
                tft.setTextColor(TFT_BLUE);
            } else {
                tft.setTextColor(TFT_WHITE);
            }
            tft.print(i + 1);
            tft.print(": ");
            tft.println(WiFi.SSID(i));
            }
          }
        }
        if (digitalRead(HOME_BUTTON_PIN) == LOW) {
            delay(200);
            function1_active = false;
            tft.fillScreen(TFT_BLACK);
            draw_menu();
        }
        if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
            delay(200);
            function1_details();
    }
  }
}

void setup() {
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HOME_BUTTON_PIN, INPUT_PULLUP);
  tft.init(240, 320);
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(90, 90);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println("phittinan@");
  tft.setCursor(40, 60);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.println("WiFi-Analyzer");
  tft.setCursor(135, 110);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("v0.7.eq");
  delay(1000);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // rest for WiFi routine?
  delay(100);
  draw_menu();
}

void loop() {
  
}
