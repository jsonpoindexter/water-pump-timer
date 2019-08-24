// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

/* -------------------- Config -------------------- */
// https://escapequotes.net/esp8266-wemos-d1-mini-pins-and-diagram/
#define PEDAL_PIN 16 // Wemos: D0
#define PEDAL_SECONDS 10 // How much time (in s) to add for each pedal push

#define USE_SHOWER_FLOAT true // Use float pin in logic to turn SHOWER_PUMP_PIN HIGH
#define SHOWER_FLOAT_PIN 14 // Wemos: D5

#define SHOWER_PUMP_PIN 12 // Wemos: D6

#define EVAP_FLOAT_PIN 13 // Wemos:  D7 
#define EVAP_PUMP_PIN 15 // Wemos: D8  
#define EVAP_SECONDS 60 * 1 // How long to run the evap pump every TEMP_INTERVAL

#define USE_TEMP_SENSOR true
#if (USE_TEMP_SENSOR)
  #define TEMP_INTERVAL 1000 * 60 * 10 // (Ms) Time between polling temp sensor (minimum of 2s)
  #define DHT_PIN 5  // Wemos: D1
  #define TEMP_MIN 70 // Minimum temperature to turn pump on
  #include <Adafruit_Sensor.h>
  #include <DHT.h>
  // REQUIRES the following Arduino libraries:
  // - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
  // - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
  //#define DHTTYPE DHT11   // DHT 11
  #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  DHT dht(DHT_PIN, DHTTYPE);
#endif
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers

/* -------------------- Init Variables -------------------- */
// Pedal Switch
int pedalState;
int previousPedalState = LOW;
unsigned long lastPedalDebounceTime = 0;

// Shower Float Switch
int showerFloatState; // HIGH == Low Water
int previousShowerFloatState = LOW;
unsigned long lastShowerFloatDebounceTime = 0;     

// Shower Pump Output
unsigned long previousShowerMillis = 0;      
int showerOnTime = 0;  // How long (in s) to turn the output io HIGH for

// Evap Float Switch
int evapFloatState; // HIGH == Low Water
int previousEvapFloatState = LOW;
unsigned long lastEvapFloatDebounceTime = 0;   

// Evap Pump Output
unsigned long previousEvapMillis = 0;     
int evapOnTime = 0;

// Temperature
int temperature;
unsigned long previousTempMillis = 0;

unsigned long currentMillis;

void setup() {
  Serial.begin(9600);
  delay(1);
  Serial.println("-------------------- Config --------------------");
  Serial.print("USE_SHOWER_FLOAT: "); if (USE_SHOWER_FLOAT) Serial.println("TRUE"); else  Serial.println("FALSE");
  Serial.print("PEDAL_SECONDS: "); Serial.println(PEDAL_SECONDS);
  Serial.print("PEDAL_PIN: "); Serial.println(PEDAL_PIN);
  Serial.print("SHOWER_FLOAT_PIN: "); Serial.println(SHOWER_FLOAT_PIN);
  Serial.print("SHOWER_PUMP_PIN: "); Serial.println(SHOWER_PUMP_PIN);
  Serial.print("EVAP_PUMP_PIN: "); Serial.println(EVAP_PUMP_PIN);
  Serial.print("EVAP_FLOAT_PIN: "); Serial.println(EVAP_FLOAT_PIN);
  Serial.print("DEBOUNCE_DELAY(ms): "); Serial.println(DEBOUNCE_DELAY);
  #if (USE_TEMP_SENSOR)
    Serial.print("TEMP_MIN: "); Serial.println(TEMP_MIN);
    Serial.print("TEMP_INTERVAL: "); Serial.println(TEMP_INTERVAL);
    Serial.print("DHT_PIN: "); Serial.println(DHT_PIN);
  #endif
  Serial.println("------------------------------------------------\n");

  pinMode(PEDAL_PIN, INPUT);
  pinMode(SHOWER_FLOAT_PIN, INPUT);
  pinMode(SHOWER_PUMP_PIN, OUTPUT);
  pinMode(EVAP_FLOAT_PIN, INPUT);
  pinMode(EVAP_PUMP_PIN, OUTPUT);
  #if (USE_TEMP_SENSOR)
    dht.begin();
    temperature = dht.readTemperature(true);
    Serial.print("temperature: ");Serial.print(temperature);Serial.println("°F");
    if (temperature >= TEMP_MIN) evapOnTime = EVAP_SECONDS;
  #endif
 
}

void loop() {
  currentMillis = millis();
  pollPedal();
  pollShowerFloat();
  showerPump();
  pollEvapFloat();
  pollTemp();
  evapPump();
}

void pollPedal(){
  if ( USE_SHOWER_FLOAT && showerFloatState == LOW) return;
  int currentPedalState = digitalRead(PEDAL_PIN);
  if (currentPedalState != previousPedalState) {
    lastPedalDebounceTime = currentMillis;
  }

  if ((currentMillis - lastPedalDebounceTime) > DEBOUNCE_DELAY) {
    if (currentPedalState != pedalState) {
      if (pedalState == HIGH) {
         Serial.println("pedalState: HIGH");
         showerOnTime += PEDAL_SECONDS;
      }
      pedalState = currentPedalState;
    }
  }
  previousPedalState = currentPedalState;
  
}

void pollShowerFloat(){
  #if (USE_SHOWER_FLOAT)
    int currentFloatState = digitalRead(SHOWER_FLOAT_PIN);
    if (currentFloatState != previousShowerFloatState) {
      lastShowerFloatDebounceTime = currentMillis;
    }
  
    if ((currentMillis - lastShowerFloatDebounceTime) > DEBOUNCE_DELAY) {
      if (currentFloatState != showerFloatState) {
        if (showerFloatState == HIGH) {
           Serial.println("showerFloatState: HIGH - Low Water");
           if (USE_SHOWER_FLOAT) showerOnTime = 0;
        } else {
          Serial.println("showerFloatState: LOW - Adequate Water");
        }
        showerFloatState = currentFloatState;
      }
    }
    previousShowerFloatState = currentFloatState;
  #endif
}

void showerPump(){
  // TODO: use prevShowerFloatState and currentshowerFloatState
  if(currentMillis - previousShowerMillis > 1000 || (USE_SHOWER_FLOAT && !showerFloatState)) {
    if(showerOnTime > 0) {
      Serial.print("showerOnTime: ");Serial.println(showerOnTime);
      digitalWrite(SHOWER_PUMP_PIN, HIGH);
      showerOnTime = showerOnTime - 1;
      previousShowerMillis = currentMillis;
    } else {
      digitalWrite(SHOWER_PUMP_PIN, LOW);
    }
  } 
}

void pollEvapFloat(){
  int currentFloatState = digitalRead(EVAP_FLOAT_PIN);
  if (currentFloatState != previousEvapFloatState) {
    lastEvapFloatDebounceTime = currentMillis;
  }

  if ((currentMillis - lastEvapFloatDebounceTime) > DEBOUNCE_DELAY) {
    if (currentFloatState != evapFloatState) {
      if (evapFloatState == HIGH) {
          Serial.println("evapFloatState: HIGH - Low Water");
          evapOnTime = 0;
      } else {
        Serial.println("evapFloatState: LOW - Adequate Water");
      }
      evapFloatState = currentFloatState;
    }
  }
  previousEvapFloatState = currentFloatState;
}

void evapPump() {
  // TODO: use prevShowerFloatState and currentshowerFloatState
  if(currentMillis - previousEvapMillis > 1000 || !evapFloatState) {
    if(evapOnTime > 0 && evapFloatState) {
      Serial.print("evapOnTime: ");Serial.println(evapOnTime);
      digitalWrite(EVAP_PUMP_PIN, HIGH);
      evapOnTime = evapOnTime - 1;
      previousEvapMillis = currentMillis;
    } else {
      digitalWrite(EVAP_PUMP_PIN, LOW);
    }
  } 
}

void pollTemp() {
  #if (USE_TEMP_SENSOR)
    if ( currentMillis - previousTempMillis > TEMP_INTERVAL) {
      previousTempMillis = currentMillis;
      // float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      // float temp = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      temperature = dht.readTemperature(true);
      Serial.print("temperature: ");Serial.print(temperature);Serial.println("°F");
      if (temperature >= TEMP_MIN) evapOnTime = EVAP_SECONDS;
    }
  #endif
}
