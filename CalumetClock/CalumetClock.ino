
#include "clock_core.h"
/* CONFIG */
const milliseconds PULSE_WIDTH = 10;
const milliseconds TIME_UNTIL_INTERNAL_CLOCK_MODE = 4000;
const ClockMode CLOCK_MODE_ON_STARTUP = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed
const unsigned short MAX_BEATS_IN_BAR = 16;



/* HARDWARE */
const int clockOutPin               = 1;
const int clockInPin                = 0;
const int swingKnobPin              = A1;
const int rotationKnobPin           = A3;
const int clockSpeedModifierKnobPin = A2;

/* GLOBAL STATE */
milliseconds timeOfLastClockInChange;
milliseconds timeOfLastPulseOut;
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
  clockSpeedModifier = readClockSpeedModifier();
  swingAmount = readSwingInput();
  internalClock.basePeriod = convertBPMToPeriodInMillis(readTempoInput());
  
  bool previousInternalClockState = internalClock.isClockHigh;

  internalClock = updateInternalClock(currentTime, internalClock, clockSpeedModifier, swingAmount);
  bool shouldTrigger = detectNewRisingClockEdge(internalClock.isClockHigh, previousInternalClockState);
      
  timeOfLastPulseOut = processTriggerOutput(shouldTrigger, timeOfLastPulseOut, currentTime, PULSE_WIDTH);

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

float fmap(long x, long in_min, long in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float readSwingInput(){
  // add debounce?
  int input = analogRead(swingKnobPin);

  float swing = 0;
  if(input < 490){
    swing = fmap(input, 0, 490, -0.8, 0);
  } else if (input > 526){
    swing = fmap(input, 526, 1023, 0, 0.8);
  }
  return swing;
}



ClockMultiplyDivideRanges mapClockSpeedModifierInputToEnumValue(int inputFromPin){
  float stepSize = 1024.0/__SIZE__;
 // static_cast<ClockMultiplyDivideRanges>(floor(inputFromPin/stepSize));

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
