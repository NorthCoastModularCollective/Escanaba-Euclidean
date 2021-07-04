
#include "clock_core.h"
/* CONFIG */
const milliseconds PULSE_WIDTH = 10;
const milliseconds TIME_UNTIL_INTERNAL_CLOCK_MODE = 4000;
const ClockMode CLOCK_MODE_ON_STARTUP = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed
const unsigned short MAX_BEATS_IN_BAR = 16;



/* HARDWARE */
const int clockOutPin       = 1;
const int clockInPin        = 0;
const int swingKnobPin  = A1;
const int rotationKnobPin   = A3;
const int clockSpeedModifierKnobPin       = A2;

/* GLOBAL STATE */
milliseconds timeOfLastClockInChange;
milliseconds timeOfLastPulseOut;
ClockMode clockMode = CLOCK_MODE_ON_STARTUP; 
bool previousClockInputState = false;
InternalClock internalClock  = InternalClock {false, 0, 133,false};
ClockMultiplyDivideRanges clockSpeedModifier = ONE;
float swingAmount=0;



/* SHELL (global state and run loop) */

void setup()
{
  pinMode(clockOutPin, OUTPUT);
  pinMode(clockInPin, INPUT);
  digitalWrite(clockOutPin, LOW);

}

void loop()
{

  milliseconds currentTime = millis();

  bool stateOfClockInPin = readClockInput();

  tuple <bool, milliseconds> ifChangedAndTime = didClockInputChange(stateOfClockInPin, previousClockInputState, currentTime, timeOfLastClockInChange);
  bool clockInputChanged = ifChangedAndTime.first;
  timeOfLastClockInChange = ifChangedAndTime.second;

  clockMode = whichClockModeShouldBeSet(clockInputChanged, clockMode, currentTime, timeOfLastClockInChange, TIME_UNTIL_INTERNAL_CLOCK_MODE);
  
  bool isNewRisingClockEdge;

  swingAmount = readSwingInput();

  clockSpeedModifier = readClockSpeedModifier();

  switch(clockMode){
    case internal: {
      bool previousInternalClockState = internalClock.isClockHigh;
      internalClock.tempo = readTempoInput();
      internalClock = updateInternalClock(currentTime, internalClock, clockSpeedModifier, swingAmount); 
      isNewRisingClockEdge = detectNewRisingClockEdge(internalClock.isClockHigh, previousInternalClockState);
      break;
    }
    case external: {
      isNewRisingClockEdge = detectNewRisingClockEdge(stateOfClockInPin, previousClockInputState); 
      break;
    }
  }
  
  bool shouldTrigger = isNewRisingClockEdge;

  timeOfLastPulseOut = processTriggerOutput(shouldTrigger, timeOfLastPulseOut, currentTime, PULSE_WIDTH);
  previousClockInputState = stateOfClockInPin;
}

/* SHELL (IO) */

milliseconds processTriggerOutput(bool shouldTrigger, milliseconds timeOfLastPulseOut, milliseconds currentTime, milliseconds PULSE_WIDTH) {
  milliseconds timeOfPulseOutToReturn = timeOfLastPulseOut;
  if (shouldTrigger) {
    digitalWrite(clockOutPin, HIGH);
    timeOfPulseOutToReturn = currentTime;
    shouldTrigger=false;
  } else if ((currentTime - timeOfLastPulseOut) > PULSE_WIDTH) {

    digitalWrite(clockOutPin, LOW);
  }
  return timeOfPulseOutToReturn;
}

bool readClockInput() {
  return !digitalRead(clockInPin);
}

unsigned int readTempoInput(){
  // add debounce?
  int inputFromRotationPin = analogRead(rotationKnobPin);
  return mapTempoInputToTempoInBpm(inputFromRotationPin);  
}

float readSwingInput(){
  // add debounce?
  int inputFromRotationPin = analogRead(swingKnobPin);
  return map(inputFromRotationPin, 0, 1023, -1, 1);  
}



ClockMultiplyDivideRanges mapClockSpeedModifierInputToEnumValue(int inputFromPin){
  float stepSize = 1024.0/__SIZE__;
 // static_cast<ClockMultiplyDivideRanges>(floor(inputFromPin/stepSize));
//  return inputFromPin>512?THREE_DIV:EIGHT_MULT;
  if(inputFromPin<stepSize){
    return EIGHT_DIV;
  } 
  else if(inputFromPin<stepSize*2){
    return FOUR_DIV;
  }
  else if(inputFromPin<stepSize*3){
    return THREE_DIV;
  }
  else if(inputFromPin<stepSize*4){
    return TWO_DIV;
  }
  else if(inputFromPin<stepSize*5){
    return ONE_POINT_FIVE_DIV;
  }
  else if(inputFromPin<stepSize*6){
    return ONE;
  }
  else if(inputFromPin<stepSize*7){
    return ONE_POINT_FIVE_MULT;
  }
  else if(inputFromPin<stepSize*8){
    return TWO_MULT;
  }
  else if(inputFromPin<stepSize*9){
    return THREE_MULT;
  }
  else if(inputFromPin<stepSize*10){
    return FOUR_MULT;
  }
  else if(inputFromPin<stepSize*11){
    return EIGHT_MULT;
  }
}

ClockMultiplyDivideRanges readClockSpeedModifier(){
  // add debounce?
  int inputFromPin = analogRead(clockSpeedModifierKnobPin);
  return mapClockSpeedModifierInputToEnumValue(inputFromPin);  
}
