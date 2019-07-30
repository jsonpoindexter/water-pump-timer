// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

/* -------------------- Config -------------------- */
#define PEDAL_PIN 2
#define PEDAL_SECONDS 30 // How much time (in s) to add for each pedal push
#define USE_FLOAT true // Use float pin in logic to turn output HIGH
#define FLOAT_PIN 3
#define SHOWER_PUMP_PIN 13
#define EVAP_PUMP_PIN 14
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers
#define USE_TEMP_SENSOR true
#define TEMP_MIN 70 // Minimum temperature to turn pump on
#define DHT_PIN 4
#define TEMP_INTERVAL 2000 // Time between polling temp sensor (minimum of 2s
#if (USE_TEMP_SENSOR)
  #include <Adafruit_Sensor.h>
  #include <DHT.h>
  // REQUIRES the following Arduino libraries:
  // - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
  // - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
  // Uncomment whatever type you're using!
  //#define DHTTYPE DHT11   // DHT 11
  #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  DHT dht(DHT_PIN, DHTTYPE);
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

// Shower Pump Output
unsigned long previousShowerMillis = 0;      
int showerOnTime = 0;  // How long (in s) to turn the output io HIGH for

// Evap Pump
unsigned long previousEvapMillis = 0;     

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
      
    }
  #endif
}

void showerPump(){
  if(currentMillis - previousShowerMillis > 1000) {
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
  if(currentMillis - previousEvapMillis) {
    if( USE_TEMP_SENSOR && (temperature > TEMP_MIN)) {
      Serial.println("evapPump: ON");
      digitalWrite(EVAP_PUMP_PIN, HIGH);
      previousEvapMillis = currentMillis;
    } else {
      Serial.println("evapPump: OFF");
      digitalWrite(EVAP_PUMP_PIN, LOW);
    }
  } 
}
