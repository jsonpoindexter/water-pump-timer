// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

// Config
#define PEDAL_PIN 2
#define PEDAL_SECONDS 30 // How much time (in s) to add for each pedal push
#define USE_FLOAT true // Use float pin in logic to turn output HIGH
#define FLOAT_PIN 3
#define OUTPUT_PIN 13
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers

// Vars
int pedalState;
int previousPedalState = LOW;
unsigned long lastPedalDebounceTime = 0;


int floatState; // HIGH == Low Water
int previousFloatState = LOW;
unsigned long lastFloatDebounceTime = 0;     


unsigned long previousOutputMillis = 0;      
int outputOnTime = 0;  // How long (in s) to turn the output io HIGH for
unsigned long currentMillis;

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
  output();
}

void pollPedal(){
  if ( USE_FLOAT && floatState == LOW) return;
  int currentPedalState = digitalRead(PEDAL_PIN);
  if (currentPedalState != previousPedalState) {
    lastPedalDebounceTime = millis();
  }

  if ((millis() - lastPedalDebounceTime) > DEBOUNCE_DELAY) {
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
    lastFloatDebounceTime = millis();
  }

  if ((millis() - lastFloatDebounceTime) > DEBOUNCE_DELAY) {
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
