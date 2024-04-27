#include <Wire.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 displays[7] = {
  {128, 64, &Wire, -1}, {128, 64, &Wire, -1}, {128, 64, &Wire, -1}, {128, 64, &Wire, -1},
  {128, 64, &Wire, -1}, {128, 64, &Wire, -1}, {128, 64, &Wire, -1}
};

const int BUTTON_PINS[7] = {0, 1, 2, 3, 4, 5, 6};  
int buttonStates[7] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastButtonStates[7] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
bool displayEnabled[7] = {false, false, false, false, false, false, false};

unsigned long lastPressTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

String metricData[7] = {"0", "0", "0", "0", "0", "0", "0"};
String metricNames[7] = {
  "Current World Population", "Deaths Today", "People who died of hunger today", "Undernourished people in the world",
  "People with no access to a safe drinking water source", "Deaths caused by water related diseases today", "Suicides today"
};

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  for (int i = 0; i < 7; i++) {
    pinMode(BUTTON_PINS[i], INPUT);
    TCA9548A(i);
    displays[i].begin(SSD1306_SWITCHCAPVCC, 0x3C);
    displays[i].clearDisplay();
    displays[i].display();
  }
  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < 7; i++) {
    int reading = digitalRead(BUTTON_PINS[i]);
    if (reading != lastButtonStates[i]) {
      lastPressTime = millis();
      delay(DEBOUNCE_DELAY);  
    }

    if ((millis() - lastPressTime) > DEBOUNCE_DELAY) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        if (buttonStates[i] == HIGH && lastButtonStates[i] == HIGH) {
          displayEnabled[i] = !displayEnabled[i];
          Serial.print(displayEnabled[i] ? "ON " : "OFF ");
          Serial.println(i + 1); 
        }
      }
    }
    lastButtonStates[i] = reading;
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    int index = 0;
    for (int i = 0; i < 7; i++) {
      metricData[i] = getValue(data, ',', index++);
    }
  }

  for (int i = 0; i < 7; i++) {
    TCA9548A(i);
    displays[i].clearDisplay();
    displays[i].setTextColor(WHITE);
    displays[i].setTextSize(2);
    displays[i].setCursor(0, 0);
    if (displayEnabled[i]) {
      displays[i].print(metricData[i]);
    } else {
      displays[i].clearDisplay();
    }
    displays[i].display();
  }
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}