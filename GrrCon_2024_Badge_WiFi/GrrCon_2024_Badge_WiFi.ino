#include <LedControl.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// LED Matrix Pins
#define DATA_PIN 13
#define CLK_PIN 14
#define CS_PIN 15

// WiFi settings
const char* ssid = "BadgeBuddy";
const char* password = "#W*Lx#LEoJC3BCVz1u42"; // Complex 12 character string

// LED Matrix setup
LedControl lc = LedControl(DATA_PIN, CLK_PIN, CS_PIN, 1);

struct Pixel {
  int x, y;
  int dx, dy;
  unsigned long lastMove;
};

const int maxPixels = 10; // Max number of animated pixels
Pixel pixels[maxPixels];
int pixelCount = 0;
unsigned long lastScanTime = 0;
const unsigned long scanInterval = 30000; // 30 seconds

void setup() {
  Serial.begin(115200);

  // Initialize LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8); // Adjust brightness here
  lc.clearDisplay(0);

  // Illuminate LEDs during startup
  illuminateLoadingLEDs();

  // Initialize WiFi
  WiFi.mode(WIFI_AP_STA);
  illuminateLoadingLEDs();
  WiFi.softAP(ssid, password);
  illuminateLoadingLEDs();
  // Clear the display before starting the animation
  lc.clearDisplay(0);
}

void loop() {
  if (millis() - lastScanTime > scanInterval) {
    illuminateLoadingLEDs();
    scanNetworks();
    lc.clearDisplay(0);
    lastScanTime = millis();
  }

  updatePixels();
  displayPixels();
  delay(50); // Adjust the delay for smoother animation
}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  int uniqueCount = 0;

  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == "BadgeBuddy") {
      bool found = false;
      for (int j = 0; j < uniqueCount; ++j) {
        if (WiFi.BSSIDstr(i) == WiFi.BSSIDstr(j)) {
          found = true;
          break;
        }
      }
      if (!found && uniqueCount < maxPixels - 1) {
        uniqueCount++;
      }
    }
  }

  // Ensure at least one pixel is animated
  pixelCount = uniqueCount + 1;
  if (pixelCount > maxPixels) {
    pixelCount = maxPixels;
  }

  // Initialize or reset pixels
  for (int i = 0; i < pixelCount; ++i) {
    pixels[i] = { random(8), random(8), random(2) * 2 - 1, random(2) * 2 - 1, millis() };
  }
}

void updatePixels() {
  unsigned long currentMillis = millis();
  for (int i = 0; i < pixelCount; ++i) {
    if (currentMillis - pixels[i].lastMove > 100) {
      pixels[i].x += pixels[i].dx;
      pixels[i].y += pixels[i].dy;

      // Bounce on walls
      if (pixels[i].x < 0 || pixels[i].x > 7) {
        pixels[i].dx = -pixels[i].dx;
        pixels[i].x += pixels[i].dx;
      }
      if (pixels[i].y < 0 || pixels[i].y > 7) {
        pixels[i].dy = -pixels[i].dy;
        pixels[i].y += pixels[i].dy;
      }

      pixels[i].lastMove = currentMillis;
    }
  }
}

void displayPixels() {
  lc.clearDisplay(0);
  for (int i = 0; i < pixelCount; ++i) {
    lc.setLed(0, pixels[i].y, pixels[i].x, true);
  }
}

void illuminateLoadingLEDs() {
  //for (int i = 0; i < 8; i++) {
    lc.setRow(0, 0, B01010101);
    lc.setRow(0, 1, B10101010);
    lc.setRow(0, 2, B01010101);
    lc.setRow(0, 3, B10101010);
    lc.setRow(0, 4, B01010101);
    lc.setRow(0, 5, B10101010);
    lc.setRow(0, 6, B01010101);
    lc.setRow(0, 7, B10101010);
  //}
}