
#include "clock_core.h"
/* CONFIG */
const milliseconds PULSE_WIDTH = 10;
const milliseconds TIME_UNTIL_INTERNAL_CLOCK_MODE = 4000;
const ClockMode CLOCK_MODE_ON_STARTUP = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed
const unsigned short MAX_BEATS_IN_BAR = 16;

/* HARDWARE */
const int triggerOutPin           = 1;
const int triggerInPin            = 0;
const int numberOfRepeatsKnobPin  = A1;
const int spacingKnobPin          = A2;
const int gravityKnobPin          = A3;

/* GLOBAL STATE */
milliseconds timeOfLastTriggerInChange;
milliseconds timeOfLastPulseOut;
bool previousTriggerInputState = false;
unsigned int numberOfRepeatsCounter = 0;
unsigned long timeOfNextTriggerInMillis = 200;
bool isBursting = false;
unsigned int numberOfRepeats=0;
unsigned int spacingInMillis=0;


/* SHELL (global state and run loop) */
void setup()
{
  pinMode(triggerOutPin, OUTPUT);
  pinMode(triggerInPin, INPUT);
  digitalWrite(triggerOutPin, LOW);

}

void loop()
{
  const auto currentTime = millis();
  
  const unsigned int initialSpacingInMillis = readSpacingInput();
  const bool stateOfTriggerInPin = readTriggerInput();
  const float gravity = readGravityInput();
  const auto hasNewTriggerIn = detectNewRisingClockEdge(stateOfTriggerInPin, previousTriggerInputState); 
  bool shouldTrigger = false;
  if(hasNewTriggerIn){
    numberOfRepeats = readNumberRepeatsInput();
    timeOfNextTriggerInMillis = currentTime + initialSpacingInMillis;
    numberOfRepeatsCounter = 0;
    shouldTrigger = true;
    isBursting = true;
    spacingInMillis = initialSpacingInMillis;
  } else if (isBursting && numberOfRepeatsCounter<numberOfRepeats && currentTime >= timeOfNextTriggerInMillis){
    timeOfNextTriggerInMillis = currentTime + spacingInMillis;
    numberOfRepeatsCounter++;
    shouldTrigger = true;
    spacingInMillis*=gravity;
  }

  timeOfLastPulseOut = processTriggerOutput(shouldTrigger, timeOfLastPulseOut, currentTime, PULSE_WIDTH);
  previousTriggerInputState = stateOfTriggerInPin;
}

/* SHELL (IO) */

milliseconds processTriggerOutput(bool shouldTrigger, milliseconds timeOfLastPulseOut, milliseconds currentTime, milliseconds PULSE_WIDTH) {
  milliseconds timeOfPulseOutToReturn = timeOfLastPulseOut;
  if (shouldTrigger) {
    digitalWrite(triggerOutPin, HIGH);
    timeOfPulseOutToReturn = currentTime;
    shouldTrigger=false;
  } else if ((currentTime - timeOfLastPulseOut) > PULSE_WIDTH) {

    digitalWrite(triggerOutPin, LOW);
  }

  return timeOfPulseOutToReturn;
}

bool readTriggerInput() {
  return !digitalRead(triggerInPin);
}

float fmap(long x, long in_min, long in_max, float out_min, float out_max) {
  return float(x - in_min) * float(out_max - out_min) / float(in_max - in_min) + out_min;
}

unsigned int readNumberRepeatsInput(){
  // add debounce?
  return map(analogRead(numberOfRepeatsKnobPin), 0, 1023, 0, 23);  
}

unsigned int readSpacingInput(){
  // add debounce?
  return map(analogRead(spacingKnobPin), 0, 1023, 30, 2000); 
}

float readGravityInput(){
  // add debounce?
  auto input = analogRead(gravityKnobPin);
  float gravity = 1;
  if(input < 490){
    gravity = fmap(input, 0, 490, 0.6, 0.99);
  } else if (input > 526){
    gravity = fmap(input, 526, 1023, 1.01, 1.5);
  }

  return gravity;  
}
