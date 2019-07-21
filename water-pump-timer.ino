// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

/* -------------------- Config -------------------- */
#define PEDAL_PIN 2
#define PEDAL_SECONDS 30 // How much time (in s) to add for each pedal push
#define USE_FLOAT true // Use float pin in logic to turn output HIGH
#define FLOAT_PIN 3
#define OUTPUT_PIN 13
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers
#define USE_TEMP_SENSOR true
#define TEMP_MIN 70 // Minimum temperature to turn pump on
#define DHT22_PIN 7
#define TEMP_INTERVAL 2000 // Time between polling temp sensor (minimum of 2s
#if (USE_TEMP_SENSOR)
#include <DHT22.h>
DHT22 myDHT22(DHT22_PIN);
#endif


/* -------------------- Init Variables -------------------- */
// Pedal Switch
int pedalState;
int previousPedalState = LOW;
unsigned long lastPedalDebounceTime = 0;

// Float Switch
int floatState; // HIGH == Low Water
int previousFloatState = LOW;
unsigned long lastFloatDebounceTime = 0;     

// Motor Output
unsigned long previousOutputMillis = 0;      
int outputOnTime = 0;  // How long (in s) to turn the output io HIGH for
unsigned long currentMillis;

// Temp Sensor
unsigned long previousTempMillis = 0;
DHT22_ERROR_t errorCode;

void setup() {
  Serial.begin(9600);
  delay(1);
  Serial.println("-------------------- Config --------------------");
  Serial.print("USE_FLOAT: "); if (USE_FLOAT) Serial.println("TRUE"); else  Serial.println("FALSE");
  Serial.print("PEDAL_SECONDS: "); Serial.println(PEDAL_SECONDS);
  Serial.print("PEDAL_PIN: "); Serial.println(PEDAL_PIN);
  Serial.print("FLOAT_PIN: "); Serial.println(FLOAT_PIN);
  Serial.print("OUTPUT_PIN: "); Serial.println(OUTPUT_PIN);
  Serial.print("DEBOUNCE_DELAY: "); Serial.println(DEBOUNCE_DELAY);
  Serial.println("------------------------------------------------\n");
  pinMode(PEDAL_PIN, INPUT);
  pinMode(FLOAT_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
}

void loop() {
  currentMillis = millis();
  pollPedal();
  pollFloat();
  pollTemp();
  output();
}

void pollPedal(){
  if ( USE_FLOAT && floatState == LOW) return;
  int currentPedalState = digitalRead(PEDAL_PIN);
  if (currentPedalState != previousPedalState) {
    lastPedalDebounceTime = currentMillis;
  }

  if ((currentMillis - lastPedalDebounceTime) > DEBOUNCE_DELAY) {
    if (currentPedalState != pedalState) {
      if (pedalState == HIGH) {
         Serial.println("pedalState: HIGH");
         outputOnTime += PEDAL_SECONDS;
      }
      pedalState = currentPedalState;
    }
  }
  previousPedalState = currentPedalState;
  
}

void pollFloat(){
  int currentFloatState = digitalRead(FLOAT_PIN);
  if (currentFloatState != previousFloatState) {
    lastFloatDebounceTime = currentMillis;
  }

  if (currentMillis - lastFloatDebounceTime) > DEBOUNCE_DELAY) {
    if (currentFloatState != floatState) {
      if (floatState == HIGH) {
         Serial.println("floatState: HIGH - Low Water");
         if (USE_FLOAT) outputOnTime = 0;
      } else {
        Serial.println("floatState: LOW - Adequate Water");
      }
      floatState = currentFloatState;
    }
  }
  previousFloatState = currentFloatState;
}

void pollTemp() {
  #if (USE_TEMP_SENSOR)
    if ( currentMillis - previouseTempMillis > TEMP_INTERVAL) {
      previousTempMillis = currentMillis;
      Serial.println("Requesting data...");
      errorCode = myDHT22.readData();
      switch(errorCode) {
        case DHT_ERROR_NONE:
          Serial.print("Got Data ");
          Serial.print(myDHT22.getTemperatureC());
          Serial.print("C ");
          Serial.print(myDHT22.getHumidity());
          Serial.println("%");
          break;
        case DHT_ERROR_CHECKSUM:
          Serial.print("check sum error ");
          Serial.print(myDHT22.getTemperatureC());
          Serial.print("C ");
          Serial.print(myDHT22.getHumidity());
          Serial.println("%");
          break;
        case DHT_BUS_HUNG:
          Serial.println("BUS Hung ");
          break;
        case DHT_ERROR_NOT_PRESENT:
          Serial.println("Not Present ");
          break;
        case DHT_ERROR_ACK_TOO_LONG:
          Serial.println("ACK time out ");
          break;
        case DHT_ERROR_SYNC_TIMEOUT:
          Serial.println("Sync Timeout ");
          break;
        case DHT_ERROR_DATA_TIMEOUT:
          Serial.println("Data Timeout ");
          break;
        case DHT_ERROR_TOOQUICK:
          Serial.println("Polled to quick ");
          break;
      }
    }
  #endif
}

void output(){
  if(currentMillis - previousOutputMillis > 1000) {
    if(outputOnTime > 0) {
      Serial.print("outputOnTime: ");Serial.println(outputOnTime);
      digitalWrite(OUTPUT_PIN, HIGH);
      outputOnTime = outputOnTime - 1;
      previousOutputMillis = currentMillis;
    } else {
      digitalWrite(OUTPUT_PIN, LOW);
    }
  } 
}
