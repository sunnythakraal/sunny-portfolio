#define BLYNK_TEMPLATE_ID "TMPL61iE5uxMM"
#define BLYNK_TEMPLATE_NAME "RC Cart"
#define BLYNK_AUTH_TOKEN "fVqRoKNf7zGrvmPFasBgei2MEXIuOgd5"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <vector>

// ------------------ Pin Definitions --------------------
#define RST_PIN 27
#define SS_PIN 5
#define BUTTON_PIN 4
#define TRIGGER_PIN 13
#define ECHO_PIN 15
#define LEFT_IR 17
#define RIGHT_IR 16
#define MOTOR_IN1 33
#define MOTOR_IN2 32
#define MOTOR_IN3 26
#define MOTOR_IN4 25

// ------------------ WiFi Credentials --------------------
char ssid[] = "Bhavuk";
char pass[] = "Bhavuk2005";

// ------------------ Module Setup --------------------
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ------------------ Product Details --------------------
const int numItems = 2;
const String productIDs[numItems] = {"03:E6:01:04", "8A:FF:38:02"};
const String productNames[numItems] = {"Chocolate", "Cold Drink"};
const int itemPrices[numItems] = {100, 200};

// ------------------ System Variables --------------------
int totalBill = 0;
bool wifiConnected = false;
std::vector<int> cartPrices;
std::vector<String> cartNames;

// ------------------ Motor Control Functions --------------------
void moveForward() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  Serial.println("Moving Forward");
}

void turnLeft() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  Serial.println("Turning Left");
}

void turnRight() {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, HIGH);
  Serial.println("Turning Right");
}

void stopMotors() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  Serial.println("Motors Stopped");
}

// ------------------ Ultrasonic Functions --------------------
long getDistance() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// ------------------ WiFi Functions --------------------
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected) {
    Serial.println("Connected to WiFi");
  } else {
    Serial.println("Failed to connect to WiFi");
  }
}

void checkWiFiStatus() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi: OFF");
    Serial.println("WiFi: OFF");
    delay(1000);
    updateTotalDisplay();
  }
}

// ------------------ Button Functions --------------------
bool debounceButton(int pin) {
  static unsigned long lastDebounce = 0;
  if (digitalRead(pin) == LOW && (millis() - lastDebounce > 300)) {
    lastDebounce = millis();
    return true;
  }
  return false;
}

// ------------------ Cart Functions --------------------
void addItemToCart(String cardID) {
  for (int i = 0; i < numItems; i++) {
    if (cardID.equalsIgnoreCase(productIDs[i])) {
      totalBill += itemPrices[i];
      cartPrices.push_back(itemPrices[i]);
      cartNames.push_back(productNames[i]);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(productNames[i] + " Added");
      lcd.setCursor(0, 1);
      lcd.print("Rs." + String(itemPrices[i]));
      Serial.println(productNames[i] + " added to cart. Price: Rs." + String(itemPrices[i]));

      if (wifiConnected) {
        Blynk.virtualWrite(V0, productNames[i] + " Added - Rs." + String(itemPrices[i]) + "\n");
        Blynk.virtualWrite(V1, totalBill);
      }

      delay(1500);
      updateTotalDisplay();
      return;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Invalid Product");
  Serial.println("Invalid Product ID: " + cardID);
  if (wifiConnected) Blynk.virtualWrite(V0, "Invalid Product\n");
  delay(1000);
  updateTotalDisplay();
}

void removeLastItem() {
  if (!cartPrices.empty()) {
    totalBill -= cartPrices.back();
    String name = cartNames.back();
    cartPrices.pop_back();
    cartNames.pop_back();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(name + " Removed");
    lcd.setCursor(0, 1);
    lcd.print("Total: Rs." + String(totalBill));
    Serial.println(name + " removed from cart. New Total: Rs." + String(totalBill));

    if (wifiConnected) {
      Blynk.virtualWrite(V0, name + " Removed\n");
      Blynk.virtualWrite(V1, totalBill);
    }

    delay(1500);
    updateTotalDisplay();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cart Empty");
    Serial.println("Attempted to remove item from empty cart.");
    if (wifiConnected) Blynk.virtualWrite(V0, "Cart Empty\n");
    delay(1000);
    updateTotalDisplay();
  }
}

// ------------------ Display Functions --------------------
void updateTotalDisplay() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Total Bill:");
  lcd.setCursor(4, 1);
  lcd.print("Rs." + String(totalBill));
  Serial.println("Total Bill Updated: Rs." + String(totalBill));
  if (wifiConnected) Blynk.virtualWrite(V1, totalBill);
}

// ------------------ Main Functions --------------------
void setup() {
  Serial.begin(115200);
  SPI.begin();
  Wire.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("System Ready");
  Serial.println("System Ready");

  connectWiFi();

  if (wifiConnected) {
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    lcd.setCursor(0, 1);
    lcd.print("WiFi: Connected");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("WiFi: Failed");
  }

  delay(2000);
  updateTotalDisplay();
}

void loop() {
  if (wifiConnected) Blynk.run();

  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 5000) {
    checkWiFiStatus();  // Corrected function call
    lastWiFiCheck = millis();
  }

  if (debounceButton(BUTTON_PIN)) removeLastItem();

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) cardID += "0";
      cardID += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) cardID += ":";
    }
    cardID.toUpperCase();
    Serial.println("Card ID Read: " + cardID);
    addItemToCart(cardID);
    rfid.PICC_HaltA();
  }

  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);
  
  int leftIR = digitalRead(LEFT_IR);
  int rightIR = digitalRead(RIGHT_IR);
  Serial.print("Left IR: ");
  Serial.print(leftIR);
  Serial.print(" Right IR: ");
  Serial.println(rightIR);

  if (distance >= 25 && distance <= 100) {
    if (leftIR == LOW && rightIR == HIGH) {
      turnLeft();
    } else if (rightIR == LOW && leftIR == HIGH) {
      turnRight();
    } else if (leftIR == HIGH && rightIR == HIGH) {
      moveForward();
    } else {
      stopMotors();
    }
  } else {
    stopMotors();
  }

  delay(2000);
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) removeLastItem();
}