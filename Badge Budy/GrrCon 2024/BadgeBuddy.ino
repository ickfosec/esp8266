// I relied heavily on ChatGPT to cleanup, improve and comment this code. This is the code that was on the badges during GrrCon 2024, however your mileage may vary should you choose to use this yourself. 
#include <LedControl.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Configure LED matrix pins
#define DATA_PIN 13
#define CLK_PIN 14
#define CS_PIN 15

// WiFi settings
const char* ssid = "BadgeBuddy";
const char* password = "#W*Lx#LEoJC3BCVz1u42";

// Establish matrix
LedControl lc = LedControl(DATA_PIN, CLK_PIN, CS_PIN, 1);

struct Pixel {
  int x, y;
  int dx, dy;
  unsigned long lastMove;
};

const int maxPixels = 10; // Max number of animated pixels for normal animation
Pixel pixels[maxPixels];
int pixelCount = 0;
int uniqueBadgeCount = 0; // Count of unique BadgeBuddy BSSIDs
bool usePinwheelPattern = false; // Flag to switch between animations

unsigned long lastScanTime = 0;
const unsigned long scanInterval = 30000; // 30 seconds

// Pinwheel pattern frame index
int pinwheelFrame = 0;
unsigned long lastPinwheelUpdate = 0;
const unsigned long pinwheelSpeed = 100; // Speed of pinwheel rotation

void setup() {
  Serial.begin(115200);

  // startup LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8); // Adjust brightness here
  lc.clearDisplay(0);

  // Illuminate LEDs during startup
  illuminateLoadingLEDs();

  // Startup WiFi
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

  if (usePinwheelPattern) {
    updatePinwheel();
  } else {
    updatePixels();
  }

  displayPixels();
  delay(50); // Adjust the delay for smoother animation
}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  uniqueBadgeCount = 0; // Reset unique badge count

  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == "BadgeBuddy") {
      bool found = false;
      for (int j = 0; j < uniqueBadgeCount; ++j) {
        if (WiFi.BSSIDstr(i) == WiFi.BSSIDstr(j)) {
          found = true;
          break;
        }
      }
      // Count unique BadgeBuddy networks without limiting to maxPixels
      if (!found) {
        uniqueBadgeCount++;
      }
    }
  }

  // If 10 or more other badges are detected, switch to pinwheel pattern
  usePinwheelPattern = (uniqueBadgeCount > 9);

  // For normal animation, calculate pixel count based on unique badges + 1, but limit to maxPixels
  if (!usePinwheelPattern) {
    pixelCount = min(uniqueBadgeCount + 1, maxPixels);

    // Initialize or reset pixels for normal animation
    for (int i = 0; i < pixelCount; ++i) {
      pixels[i] = { random(8), random(8), random(2) * 2 - 1, random(2) * 2 - 1, millis() };
    }
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

// Pinwheel animation frames
const byte pinwheelFrames[4][8] = {
  {B00011000, B00100100, B01000010, B10000001, B10000001, B01000010, B00100100, B00011000}, // Frame 1
  {B10000001, B11000011, B01100110, B00111100, B00111100, B01100110, B11000011, B10000001}, // Frame 2
  {B00011000, B00100100, B01000010, B10000001, B10000001, B01000010, B00100100, B00011000}, // Frame 3
  {B10000001, B11000011, B01100110, B00111100, B00111100, B01100110, B11000011, B10000001}  // Frame 4
};

void updatePinwheel() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPinwheelUpdate > pinwheelSpeed) {
    pinwheelFrame = (pinwheelFrame + 1) % 4;
    lastPinwheelUpdate = currentMillis;
  }
}

void displayPixels() {
  lc.clearDisplay(0);
  if (usePinwheelPattern) {
    for (int i = 0; i < 8; ++i) {
      lc.setRow(0, i, pinwheelFrames[pinwheelFrame][i]);
    }
  } else {
    for (int i = 0; i < pixelCount; ++i) {
      lc.setLed(0, pixels[i].y, pixels[i].x, true);
    }
  }
}

void illuminateLoadingLEDs() {
  lc.setRow(0, 0, B01010101);
  lc.setRow(0, 1, B10101010);
  lc.setRow(0, 2, B01010101);
  lc.setRow(0, 3, B10101010);
  lc.setRow(0, 4, B01010101);
  lc.setRow(0, 5, B10101010);
  lc.setRow(0, 6, B01010101);
  lc.setRow(0, 7, B10101010);
}
