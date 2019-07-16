// Objective: Use a NC momentary switch to turn an output high for a durations that 
// can be increased by each momentary switch activation.

// Config
#define PEDAL_PIN 2
int pedalDebounce = 300; // How much time (in ms) to wait to poll after each push
int pedalSeconds = 30; // How much time (in s) to add for each pedal push
#define USE_FLOAT true // Use float pin in logic to turn output HIGH
#define FLOAT_PIN 3
int floatDebounce = 1000; // How much time (in ms) to wait to poll
#define OUTPUT_PIN 13

// Vars
boolean previousPedalState = false;
boolean previousFloatState = false;
unsigned long previousPedalMillis = 0;      
unsigned long previousFloatMillis = 0;      
unsigned long previousOutputMillis = 0;      
int outputOnTime = 0;  // How long (in s) to turn the output io HIGH for
unsigned long currentMillis;

void setup() {
  Serial.begin(9600);
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
   if(currentMillis - previousPedalMillis > pedalDebounce) {
    boolean currentPedalState = digitalRead(PEDAL_PIN);
    if(currentPedalState != previousPedalState) {
      outputOnTime += pedalSeconds;
      previousPedalMillis = currentMillis;
      previousPedalState = currentPedalState;
    }
  } 
}

void pollFloat(){
    if(currentMillis - previousFloatMillis > floatDebounce) {
    boolean currentFloatState = digitalRead(FLOAT_PIN);
    if(currentFloatState != previousFloatState) {
      if(USE_FLOAT && !currentFloatState) outputOnTime = 0;
      previousFloatMillis = currentMillis;
      previousFloatState = currentFloatState;
    }
  } 
}

void output(){
  if(currentMillis - previousOutputMillis > 1000) {
    if(outputOnTime > 0) {
      Serial.println(outputOnTime);
      digitalWrite(OUTPUT_PIN, HIGH);
      outputOnTime = outputOnTime - 1;
      previousOutputMillis = currentMillis;
    } else {
      digitalWrite(OUTPUT_PIN, LOW);
    }
  } 
}
