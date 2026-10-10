#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID reader
SoftwareSerial RFID(2, 11); // RX = 2, (TX unused)

// RFID TAGS
const char RFID1[] = "4600AE4AD476";  // Tea
const char RFID2[] = "4600AE515AE3";  // Chocolate
const char RFID3[] = "4600AE4ADE7C";  // Rice
const char RFID4[] = "4600AE50AB13";  // Battery

// Item names & prices
String items[4] = {"Tea", "Chocolate", "Rice", "Battery"};
int price[4] = {10, 20, 30, 40};
int qty[4] = {0, 0, 0, 0};
int total = 0;

// Buttons
#define ADD_BUTTON 3      // Pay Button
#define REMOVE_BUTTON 4   // Delete Button

// Buzzer & LEDs
#define BUZZER 8
#define RED_LED 9
#define GREEN_LED 10

bool locked = false;
String rawBuffer = "";

// CUSTOM HEART ICON
byte heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

void beepAdd() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(120);
  digitalWrite(BUZZER, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void beepRemove() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(160);
  digitalWrite(BUZZER, LOW);
  digitalWrite(RED_LED, LOW);
}

int matchRFIDString(const String &tag) {
  if (tag == RFID1) return 0;
  if (tag == RFID2) return 1;
  if (tag == RFID3) return 2;
  if (tag == RFID4) return 3;
  return -1;
}

void addItem(int i) {
  if (locked) return;
  qty[i]++;
  total += price[i];

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(items[i] + " Added");
  lcd.setCursor(0, 1);
  lcd.print("Price: ₹");
  lcd.print(price[i]);

  beepAdd();
  delay(800);

  lcd.clear();
  lcd.print("Scan Item...");
}

void removeItem(int i) {
  if (locked) return;

  if (qty[i] == 0) {
    lcd.clear();
    lcd.print("Not in cart!");
    beepRemove();
    delay(800);
    lcd.clear();
    lcd.print("Scan Item...");
    return;
  }

  qty[i]--;
  total -= price[i];

  lcd.clear();
  lcd.print(items[i] + " Removed");
  lcd.setCursor(0, 1);
  lcd.print("Total: ₹");
  lcd.print(total);

  beepRemove();
  delay(800);
  lcd.clear();
  lcd.print("Scan Item...");
}

void payNow() {
  locked = true;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Final Bill: ₹");
  lcd.print(total);

  lcd.setCursor(0,1);
  lcd.print("Items: ");
  lcd.print(qty[0] + qty[1] + qty[2] + qty[3]);
  delay(2500);

  lcd.clear();
  lcd.print("Payment Done!");
  lcd.setCursor(0,1);
  lcd.print("Thank You ");
  lcd.write(byte(0)); // heart symbol

  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  digitalWrite(GREEN_LED, LOW);

  // RESET CART
  qty[0] = qty[1] = qty[2] = qty[3] = 0;
  total = 0;
  locked = false;

  lcd.clear();
  lcd.print("Scan Item...");
}

String extractLast12(const String &s) {
  String d = "";
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isDigit(c) || isxdigit(c)) d += c;
  }
  if (d.length() >= 12) return d.substring(d.length() - 12);
  return "";
}

void processRawBuffer() {
  String tag = extractLast12(rawBuffer);
  rawBuffer = "";

  if (tag.length() != 12) return;

  Serial.print("Tag: ");
  Serial.println(tag);

  lcd.clear();
  lcd.print("Scanned:");
  lcd.setCursor(0,1);
  lcd.print(tag);

  delay(250);

  int idx = matchRFIDString(tag);
  if (idx != -1) {
    addItem(idx);
  } else {
    lcd.clear();
    lcd.print("Unknown Tag!");
    beepRemove();
    delay(800);
    lcd.clear();
    lcd.print("Scan Item...");
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, heart);

  pinMode(ADD_BUTTON, INPUT_PULLUP);
  pinMode(REMOVE_BUTTON, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(9600);
  RFID.begin(9600);

  lcd.clear();
  lcd.print("Smart Cart Ready");
  delay(1000);
  lcd.clear();
  lcd.print("Scan Item...");
}

void loop() {

  // Read RFID continuously
  while (RFID.available()) {
    char c = RFID.read();
    rawBuffer += c;

    if (rawBuffer.length() >= 12) {
      processRawBuffer();
    }
  }

  // PAY NOW (long press)
  if (digitalRead(ADD_BUTTON) == LOW) {
    unsigned long t = millis();
    while (digitalRead(ADD_BUTTON) == LOW) {
      if (millis() - t > 1200) {
        payNow();
        while (digitalRead(ADD_BUTTON) == LOW);
        break;
      }
    }
  }

  // DELETE LAST ITEM (short press)
  if (digitalRead(REMOVE_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(REMOVE_BUTTON) == LOW) {

      int last = -1;
      for (int i = 3; i >= 0; i--) {
        if (qty[i] > 0) { last = i; break; }
      }

      if (last != -1) removeItem(last);
      else {
        lcd.clear();
        lcd.print("Cart empty");
        beepRemove();
        delay(800);
        lcd.clear();
        lcd.print("Scan Item...");
      }

      while (digitalRead(REMOVE_BUTTON) == LOW);
    }
  }
}#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID reader
SoftwareSerial RFID(2, 11); // RX = 2, (TX unused)

// RFID TAGS
const char RFID1[] = "4600AE4AD476";  // Tea
const char RFID2[] = "4600AE515AE3";  // Chocolate
const char RFID3[] = "4600AE4ADE7C";  // Rice
const char RFID4[] = "4600AE50AB13";  // Battery

// Item names & prices
String items[4] = {"Tea", "Chocolate", "Rice", "Battery"};
int price[4] = {10, 20, 30, 40};
int qty[4] = {0, 0, 0, 0};
int total = 0;

// Buttons
#define ADD_BUTTON 3      // Pay Button
#define REMOVE_BUTTON 4   // Delete Button

// Buzzer & LEDs
#define BUZZER 8
#define RED_LED 9
#define GREEN_LED 10

bool locked = false;
String rawBuffer = "";

// CUSTOM HEART ICON
byte heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

void beepAdd() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(120);
  digitalWrite(BUZZER, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void beepRemove() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(160);
  digitalWrite(BUZZER, LOW);
  digitalWrite(RED_LED, LOW);
}

int matchRFIDString(const String &tag) {
  if (tag == RFID1) return 0;
  if (tag == RFID2) return 1;
  if (tag == RFID3) return 2;
  if (tag == RFID4) return 3;
  return -1;
}

void addItem(int i) {
  if (locked) return;
  qty[i]++;
  total += price[i];

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(items[i] + " Added");
  lcd.setCursor(0, 1);
  lcd.print("Price: ₹");
  lcd.print(price[i]);

  beepAdd();
  delay(800);

  lcd.clear();
  lcd.print("Scan Item...");
}

void removeItem(int i) {
  if (locked) return;

  if (qty[i] == 0) {
    lcd.clear();
    lcd.print("Not in cart!");
    beepRemove();
    delay(800);
    lcd.clear();
    lcd.print("Scan Item...");
    return;
  }

  qty[i]--;
  total -= price[i];

  lcd.clear();
  lcd.print(items[i] + " Removed");
  lcd.setCursor(0, 1);
  lcd.print("Total: ₹");
  lcd.print(total);

  beepRemove();
  delay(800);
  lcd.clear();
  lcd.print("Scan Item...");
}

void payNow() {
  locked = true;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Final Bill: ₹");
  lcd.print(total);

  lcd.setCursor(0,1);
  lcd.print("Items: ");
  lcd.print(qty[0] + qty[1] + qty[2] + qty[3]);
  delay(2500);

  lcd.clear();
  lcd.print("Payment Done!");
  lcd.setCursor(0,1);
  lcd.print("Thank You ");
  lcd.write(byte(0)); // heart symbol

  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  digitalWrite(GREEN_LED, LOW);

  // RESET CART
  qty[0] = qty[1] = qty[2] = qty[3] = 0;
  total = 0;
  locked = false;

  lcd.clear();
  lcd.print("Scan Item...");
}

String extractLast12(const String &s) {
  String d = "";
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    if (isDigit(c) || isxdigit(c)) d += c;
  }
  if (d.length() >= 12) return d.substring(d.length() - 12);
  return "";
}

void processRawBuffer() {
  String tag = extractLast12(rawBuffer);
  rawBuffer = "";

  if (tag.length() != 12) return;

  Serial.print("Tag: ");
  Serial.println(tag);

  lcd.clear();
  lcd.print("Scanned:");
  lcd.setCursor(0,1);
  lcd.print(tag);

  delay(250);

  int idx = matchRFIDString(tag);
  if (idx != -1) {
    addItem(idx);
  } else {
    lcd.clear();
    lcd.print("Unknown Tag!");
    beepRemove();
    delay(800);
    lcd.clear();
    lcd.print("Scan Item...");
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, heart);

  pinMode(ADD_BUTTON, INPUT_PULLUP);
  pinMode(REMOVE_BUTTON, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(9600);
  RFID.begin(9600);

  lcd.clear();
  lcd.print("Smart Cart Ready");
  delay(1000);
  lcd.clear();
  lcd.print("Scan Item...");
}

void loop() {

  // Read RFID continuously
  while (RFID.available()) {
    char c = RFID.read();
    rawBuffer += c;

    if (rawBuffer.length() >= 12) {
      processRawBuffer();
    }
  }

  // PAY NOW (long press)
  if (digitalRead(ADD_BUTTON) == LOW) {
    unsigned long t = millis();
    while (digitalRead(ADD_BUTTON) == LOW) {
      if (millis() - t > 1200) {
        payNow();
        while (digitalRead(ADD_BUTTON) == LOW);
        break;
      }
    }
  }

  // DELETE LAST ITEM (short press)
  if (digitalRead(REMOVE_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(REMOVE_BUTTON) == LOW) {

      int last = -1;
      for (int i = 3; i >= 0; i--) {
        if (qty[i] > 0) { last = i; break; }
      }

      if (last != -1) removeItem(last);
      else {
        lcd.clear();
        lcd.print("Cart empty");
        beepRemove();
        delay(800);
        lcd.clear();
        lcd.print("Scan Item...");
      }

      while (digitalRead(REMOVE_BUTTON) == LOW);
    }
  }
}