// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

/* -------------------- Config -------------------- */

#define PEDAL_PIN 2
#define PEDAL_SECONDS 30 // How much time (in s) to add for each pedal push

#define USE_FLOAT true // Use float pin in logic to turn SHOWER_PUMP_PIN HIGH
#define FLOAT_PIN 3

#define SHOWER_PUMP_PIN 4

#define EVAP_PUMP_PIN 6
#define EVAP_SECONDS 1 // How long to run the evap pump every TEMP_INTERVAL

#define USE_TEMP_SENSOR true
#if (USE_TEMP_SENSOR)
  #define TEMP_INTERVAL 2000 // Time between polling temp sensor (minimum of 2s)
  #define DHT_PIN 5
  #define TEMP_MIN 80 // Minimum temperature to turn pump on
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
int floatState; // HIGH == Low Water
int previousFloatState = LOW;
unsigned long lastFloatDebounceTime = 0;     

// Shower Pump Output
unsigned long previousShowerMillis = 0;      
int showerOnTime = 0;  // How long (in s) to turn the output io HIGH for

// Evap Float Switch


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
  Serial.print("USE_FLOAT: "); if (USE_FLOAT) Serial.println("TRUE"); else  Serial.println("FALSE");
  Serial.print("PEDAL_SECONDS: "); Serial.println(PEDAL_SECONDS);
  Serial.print("PEDAL_PIN: "); Serial.println(PEDAL_PIN);
  Serial.print("FLOAT_PIN: "); Serial.println(FLOAT_PIN);
  Serial.print("SHOWER_PUMP_PIN: "); Serial.println(SHOWER_PUMP_PIN);
  Serial.print("DEBOUNCE_DELAY(ms): "); Serial.println(DEBOUNCE_DELAY);
  #if (USE_TEMP_SENSOR)
    Serial.print("TEMP_MIN: "); Serial.println(TEMP_MIN);
    Serial.print("TEMP_INTERVAL: "); Serial.println(TEMP_INTERVAL);
    Serial.print("DHT_PIN: "); Serial.println(DHT_PIN);
  #endif
  Serial.println("------------------------------------------------\n");

  pinMode(PEDAL_PIN, INPUT);
  pinMode(FLOAT_PIN, INPUT);
  pinMode(SHOWER_PUMP_PIN, OUTPUT);
  pinMode(EVAP_PUMP_PIN, OUTPUT);
  #if (USE_TEMP_SENSOR)
    dht.begin();
  #endif
 
}

void loop() {
  currentMillis = millis();
  pollPedal();
  pollFloat();
  pollTemp();
  showerPump();
  evapPump();
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
         showerOnTime += PEDAL_SECONDS;
      }
      pedalState = currentPedalState;
    }
  }
  previousPedalState = currentPedalState;
  
}

void pollFloat(){
  #if (USE_FLOAT)
    int currentFloatState = digitalRead(FLOAT_PIN);
    if (currentFloatState != previousFloatState) {
      lastFloatDebounceTime = currentMillis;
    }
  
    if ((currentMillis - lastFloatDebounceTime) > DEBOUNCE_DELAY) {
      if (currentFloatState != floatState) {
        if (floatState == HIGH) {
           Serial.println("floatState: HIGH - Low Water");
           if (USE_FLOAT) showerOnTime = 0;
        } else {
          Serial.println("floatState: LOW - Adequate Water");
        }
        floatState = currentFloatState;
      }
    }
    previousFloatState = currentFloatState;
  #endif
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

void showerPump(){
  if(currentMillis - previousShowerMillis > 1000 || (USE_FLOAT && !floatState)) {
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

void evapPump() {
  if(currentMillis - previousEvapMillis > 1000) {
    if(evapOnTime > 0) {
      Serial.print("evapOnTime: ");Serial.println(evapOnTime);
      digitalWrite(EVAP_PUMP_PIN, HIGH);
      evapOnTime = evapOnTime - 1;
      previousEvapMillis = currentMillis;
    } else {
      digitalWrite(EVAP_PUMP_PIN, LOW);
    }
  } 
}
