#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int flowSensorPin = PB3; // Flow sensor pin
unsigned long duration; // Variable to store the duration of a pulse
float pulseFrequency; // Variable to store the pulse frequency
float flowRate; // Variable to store the flow rate

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display
  display.clearDisplay();
}

void loop() {
  // Measure the duration of a pulse
  duration = pulseIn(flowSensorPin, HIGH);

  // Calculate the pulse frequency
  pulseFrequency = 1000000.0 / duration;

  // Calculate the flow rate
  flowRate = pulseFrequency * 60.0 / 4.8;

  // Display the flow rate on the OLED screen
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Flow Rate: ");
  display.print(flowRate);
  display.println(" mL/min");
  display.display();

  // Print the flow rate to the serial monitor
  Serial.print("Flow Rate: ");
  Serial.print(flowRate);
  Serial.println(" mL/min");

  // Wait for a second
  delay(1000);
}