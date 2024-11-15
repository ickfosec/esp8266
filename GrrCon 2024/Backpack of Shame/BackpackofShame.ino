// I relied heavily on ChatGPT to cleanup, improve and comment this code. This is the code that was on the badges during GrrCon 2024, however your mileage may vary should you choose to use this yourself. 
#include <ESP8266WiFi.h>
#include <LedControl.h>

// Define the number of devices (LED matrices)
#define MAX_DEVICES 4

// Pins for the LED matrix
#define CLK_PIN   14  // D5
#define DATA_PIN  13  // D7
#define CS_PIN    15  // D8

// Create a LedControl object
LedControl lc = LedControl(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// WiFi network details
const char* ssid = "GrrCon WiFi";
const char* password = "";  // Leave blank for an open network

// Store connected device count
int deviceCount = 0;

// Track unique device MAC addresses
std::vector<String> connectedDevices;

// WiFi Event Handler
WiFiEventHandler stationConnectedHandler;

// Character bitmaps for '#' and digits '0' to '9'
uint8_t hashChar[8] = {
  0x00,  // Empty row (top)
  0x24,  //  0 0 1 0 0 1 0 0
  0x7E,  //  0 1 1 1 1 1 1 0  (horizontal bar in middle)
  0x24,  //  0 0 1 0 0 1 0 0
  0x24,  //  0 0 1 0 0 1 0 0
  0x7E,  //  0 1 1 1 1 1 1 0  (another horizontal bar in middle)
  0x24,  //  0 0 1 0 0 1 0 0
  0x00   // Empty row (bottom)
};

uint8_t digits[10][8] = {
  {0x3E, 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x3E},  // 0
  {0x0C, 0x1C, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F},  // 1
  {0x3E, 0x63, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x7F},  // 2
  {0x3E, 0x63, 0x03, 0x1E, 0x03, 0x03, 0x63, 0x3E},  // 3
  {0x06, 0x0E, 0x1E, 0x36, 0x66, 0x7F, 0x06, 0x06},  // 4
  {0x7F, 0x60, 0x7E, 0x03, 0x03, 0x03, 0x63, 0x3E},  // 5
  {0x3E, 0x63, 0x60, 0x7E, 0x63, 0x63, 0x63, 0x3E},  // 6
  {0x7F, 0x63, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x30},  // 7
  {0x3E, 0x63, 0x63, 0x3E, 0x63, 0x63, 0x63, 0x3E},  // 8
  {0x3E, 0x63, 0x63, 0x3F, 0x03, 0x03, 0x63, 0x3E}   // 9
};

// Function to initialize the LED matrices
void initLED() {
  for (int i = 0; i < MAX_DEVICES; i++) {
    lc.shutdown(i, false);  // Wake up the display
    lc.setIntensity(i, 5);  // Set brightness level (0-15)
    lc.clearDisplay(i);     // Clear the display
  }
}

// Function to rotate an 8x8 bitmap by 90 degrees counter-clockwise
void rotate90CounterClockwise(uint8_t input[8], uint8_t output[8]) {
  for (int x = 0; x < 8; x++) {
    output[x] = 0;
    for (int y = 0; y < 8; y++) {
      if (input[y] & (1 << x)) {
        output[x] |= (1 << (7 - y));
      }
    }
  }
}

// Function to display a character manually on a given device with rotation
void displayCharacter(int device, uint8_t bitmap[8]) {
  uint8_t rotatedBitmap[8];
  rotate90CounterClockwise(bitmap, rotatedBitmap);  // Rotate before displaying

  for (int row = 0; row < 8; row++) {
    lc.setRow(device, row, rotatedBitmap[row]);
  }
}

// Function to display the count as "#XXX" (e.g., #003 for 3 connected devices)
void displayCount() {
  char countStr[4];  // 3 digits plus null terminator
  snprintf(countStr, sizeof(countStr), "%03d", deviceCount);  // Format count with leading zeros

  // Display '#' on the first matrix
  displayCharacter(0, hashChar);

  // Display the digits in the next three matrices
  for (int i = 0; i < 3; i++) {
    int digit = countStr[i] - '0';  // Convert char '0'-'9' to the corresponding integer
    displayCharacter(i + 1, digits[digit]);
  }
}

// Function to handle when a station connects
void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", 
           evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5]);

  Serial.print("Device connected: ");
  Serial.println(macStr);

  String deviceMAC = String(macStr);

  // If the device hasn't connected before, count it as new
  if (std::find(connectedDevices.begin(), connectedDevices.end(), deviceMAC) == connectedDevices.end()) {
    Serial.println("New device detected, adding to the list.");
    connectedDevices.push_back(deviceMAC);
    deviceCount++;
    displayCount();  // Update the display whenever a new device connects
  } else {
    Serial.println("Device already connected, ignoring.");
  }
}

// Setup WiFi and LED matrix
void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Initialize the LED matrices
  initLED();

  // Set up WiFi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");

  // Set up the event handler for new station connections
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(onStationConnected);

  // Display initial count
  displayCount();  // Show the count (initially 0) when the program starts
}

void loop() {
  // The display will update when a device connects, nothing needed here.
}
