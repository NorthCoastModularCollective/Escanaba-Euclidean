
#include "euclid_core.h"
/* CONFIG */
milliseconds pulseWidth = 5;
milliseconds timeUntilInternalClockMode = 4000;
ClockMode clockModeOnStartup = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed

/* HARDWARE */
const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;

/* GLOBAL STATE */
milliseconds timeOfLastClockInChange;
milliseconds timeOfLastPulseOut;
EuclidRythmParameters euclidRythmParameters;
ClockMode clockMode = clockModeOnStartup; 
bool previousClockInputState = false;
InternalClock internalClock  = InternalClock {false, 0, 133};

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
  bool clockInputChanged = ifChangedAndTime.a;
  timeOfLastClockInChange = ifChangedAndTime.b;

  clockMode = whichClockModeShouldBeSet(clockInputChanged, clockMode, currentTime, timeOfLastClockInChange, timeUntilInternalClockMode);
  previousClockInputState = stateOfClockInPin;

  bool isNewRisingClockEdge;

  switch(clockMode){
    case internal: {
      bool previousInternalClockState = internalClock.state;
      internalClock.tempo = readTempoInput();
      internalClock = updateInternalClock(currentTime, internalClock); 
      isNewRisingClockEdge = detectNewRisingClockEdge(internalClock.state, previousInternalClockState);
      break;
    }
    default: {
      isNewRisingClockEdge = detectNewRisingClockEdge(stateOfClockInPin, previousClockInputState); 
      break;
    }
  }
  
  euclidRythmParameters = updateEuclidParams( clockMode, isNewRisingClockEdge, euclidRythmParameters);

  bool shouldTrigger = isNewRisingClockEdge && euclid(euclidRythmParameters.phase, euclidRythmParameters.hits, euclidRythmParameters.barLength, euclidRythmParameters.rotation);

  timeOfLastPulseOut = processTriggerOutput(shouldTrigger, timeOfLastPulseOut, currentTime, pulseWidth);

}

/* SHELL (IO) */

milliseconds processTriggerOutput(bool shouldTrigger, milliseconds timeOfLastPulseOut, milliseconds currentTime, milliseconds pulseWidth) {
  milliseconds timeOfPulseOutToReturn = timeOfLastPulseOut;
  if (shouldTrigger) {
    digitalWrite(clockOutPin, HIGH);
    timeOfPulseOutToReturn = currentTime;
  } else if ((currentTime - timeOfLastPulseOut) > pulseWidth) {

    digitalWrite(clockOutPin, LOW);
  }
  return timeOfPulseOutToReturn;
}

bool readClockInput() {
  return !digitalRead(clockInPin);
}

EuclidRythmParameters updateEuclidParams( ClockMode mode, bool isNewRisingClockEdge, EuclidRythmParameters previousParams) {
  EuclidRythmParameters params = previousParams;
  if (isNewRisingClockEdge) {
    params.barLength = map(analogRead(barLengthKnobPin), 0, 1023, 1, 16);
    params.hits = map(analogRead(hitsKnobPin), 0, 1023, 0, 16);
    params.phase = euclidRythmParameters.phase + 1;  
    
    if(mode==external){
      params.rotation = map(analogRead(rotationKnobPin), 0, 1023, 0, 16);            
    }
  }
  return params;
}

unsigned int readTempoInput(){
  int inputFromRotationPin = analogRead(rotationKnobPin);
 

  return mapTempoInputToTempoInBpm(inputFromRotationPin);  
}
