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
bool useFallingCodePattern = false; // Flag to switch between animations

unsigned long lastScanTime = 0;
const unsigned long scanInterval = 30000; // 30 seconds

// Falling code variables
const int columns = 8; // 8 columns for the LED matrix
int fallPos[columns]; // Tracks the y-position of the falling pixel in each column
bool columnActive[columns]; // Whether a column has an active falling pixel

unsigned long lastFallingUpdate = 0;
const unsigned long fallingSpeed = 100; // Speed of falling pixels

void setup() {
  Serial.begin(115200);

  // startup LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 8); // Adjust brightness here
  lc.clearDisplay(0);

  // Initialize falling pixel positions
  for (int i = 0; i < columns; ++i) {
    fallPos[i] = random(8); // Random initial position
    columnActive[i] = false; // No active falling pixels initially
  }

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

  if (useFallingCodePattern) {
    updateFallingCode();
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

  // If more than 9 other badges are detected, switch to falling code pattern
  useFallingCodePattern = (uniqueBadgeCount > 8);

  // For normal animation, calculate pixel count based on unique badges + 1, but limit to maxPixels
  if (!useFallingCodePattern) {
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

void updateFallingCode() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastFallingUpdate > fallingSpeed) {
    // Update each column's falling position
    for (int col = 0; col < columns; ++col) {
      if (columnActive[col]) {
        fallPos[col]++;

        // Reset if it falls beyond the bottom
        if (fallPos[col] > 7) {
          fallPos[col] = 0;
          columnActive[col] = false; // Deactivate this column
        }
      } else {
        // Randomly activate a new falling pixel
        if (random(10) < 2) { // 20% chance to activate
          fallPos[col] = 0;
          columnActive[col] = true;
        }
      }
    }

    lastFallingUpdate = currentMillis;
  }
}

void displayPixels() {
  lc.clearDisplay(0);
  if (useFallingCodePattern) {
    // Display the falling code animation
    for (int col = 0; col < columns; ++col) {
      if (columnActive[col]) {
        lc.setLed(0, fallPos[col], col, true);
      }
    }
  } else {
    // Display the normal animation with bouncing pixels
    for (int i = 0; i < pixelCount; ++i) {
      lc.setLed(0, pixels[i].y, pixels[i].x, true);
    }
  }
}

void illuminateLoadingLEDs() {
 // Display a question mark symbol
  byte questionMark[8] = {
    B00111100,
    B01000010,
    B00000010,
    B00000100,
    B00001000,
    B00000000,
    B00001000,
    B00000000
  };

  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, questionMark[i]);
  }
}
