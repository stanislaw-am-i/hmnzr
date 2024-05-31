#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <set>
LiquidCrystal_I2C displays[7] = {
  {0x27, 16, 2}, {0x26, 16, 2}, {0x25, 16, 2}, {0x24, 16, 2},
  {0x23, 16, 2}, {0x22, 16, 2}, {0x21, 16, 2}
};

const int BUTTON_PINS[7] = {2, 3, 4, 5, 6, 7, 8};  
int buttonStates[7] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastButtonStates[7] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
bool displayEnabled[7] = {false, false, false, false, false, false, false};

unsigned long lastPressTime = 0;
const unsigned long DEBOUNCE_DELAY = 20;

String metricData[7] = {"0", "0", "0", "0", "0", "0", "0"};
String metricNames[7] = {
  "World Population", "Deaths Today", "People died of hunger today", "Undernourished people",
  "People with no access to water", "Deaths by water diseases today", "Suicides today"
};
long metricDataState[7] = {0, 0, 0, 0, 0, 0, 0};
std::set<int> metricsWitnNoConstAudio;

void setup() {
  // Add a metric by index below to set it plays audio only if increase 
  metricsWitnNoConstAudio.insert(1);
  metricsWitnNoConstAudio.insert(4);
  metricsWitnNoConstAudio.insert(6);

  Wire.begin();
  for (int i = 0; i < 7; i++) {
    displays[i].init();  
    displays[i].backlight();  
    displays[i].clear();  
  }
  Serial.begin(9600);
  for (int pin : BUTTON_PINS) {
    pinMode(pin, INPUT);
  }
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
          if (metricsWitnNoConstAudio.find(i) == metricsWitnNoConstAudio.end()) {
            Serial.print(displayEnabled[i] ? "ON " : "OFF ");
            Serial.println(i + 1);
          } else if (displayEnabled[i] == false) {
            Serial.print("OFF ");
            Serial.println(i + 1); 
          }
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
    if (displayEnabled[i]) {
      // Плавная прокрутка текста для первой строки
      static unsigned long lastScrollTime = millis();
      static int scrollOffset = 0;
      unsigned long now = millis();

      if (now - lastScrollTime >= 50) { // Измените скорость прокрутки по желанию
        lastScrollTime = now;
        scrollOffset++;
        if (scrollOffset >= metricNames[i].length() * 6) { // Увеличиваем скорость прокрутки за счет умножения на коэффициент
          scrollOffset = 0;
        }
      }

      displays[i].setCursor(0, 0);
      String scrollingText = metricNames[i] + "    "; // Добавляем пробелы в конец, чтобы текст начинался с начала экрана
      scrollingText = scrollingText.substring(scrollOffset / 6, (scrollOffset / 6) + 16); // Извлекаем подстроку для плавной прокрутки
      displays[i].print(scrollingText);
      displays[i].setCursor(0, 1);
      displays[i].print(metricData[i]);
      
      if (metricsWitnNoConstAudio.find(i) != metricsWitnNoConstAudio.end()) {
        long currentMetricValue = (long) metricData[i].toInt();
        if (currentMetricValue > metricDataState[i]) {
          Serial.print("ON ");
          Serial.println(i + 1); 
        } 
        metricDataState[i] = currentMetricValue;
      }
    } else {
      displays[i].clear();
    }
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