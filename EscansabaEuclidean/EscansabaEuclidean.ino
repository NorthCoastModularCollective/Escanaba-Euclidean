
#include "euclid_core.h"
/* CONFIG */
const milliseconds PULSE_WIDTH = 10; //const
const milliseconds TIME_UNTIL_INTERNAL_CLOCK_MODE = 4000; //const
const ClockMode CLOCK_MODE_ON_STARTUP = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed

/* HARDWARE */
const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;

/* GLOBAL STATE */
milliseconds timeOfLastClockInChange;
milliseconds timeOfLastPulseOut;
EuclidRythmParameters euclidRhythmParameters;
ClockMode clockMode = CLOCK_MODE_ON_STARTUP; 
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

  //-------------
  milliseconds currentTime = millis();

  bool stateOfClockInPin = readClockInput();

  tuple <bool, milliseconds> ifChangedAndTime = didClockInputChange(stateOfClockInPin, previousClockInputState, currentTime, timeOfLastClockInChange);
  bool clockInputChanged = ifChangedAndTime.a;
  timeOfLastClockInChange = ifChangedAndTime.b;

  clockMode = whichClockModeShouldBeSet(clockInputChanged, clockMode, currentTime, timeOfLastClockInChange, TIME_UNTIL_INTERNAL_CLOCK_MODE);
  
//--------------
//---------------
  bool isNewRisingClockEdge;

  switch(clockMode){
    case internal: {
      bool previousInternalClockState = internalClock.isClockHigh;
      internalClock.tempo = readTempoInput();
      internalClock = updateInternalClock(currentTime, internalClock); 
      isNewRisingClockEdge = detectNewRisingClockEdge(internalClock.isClockHigh, previousInternalClockState);
      break;
    }
    default: {
      // case external
      isNewRisingClockEdge = detectNewRisingClockEdge(stateOfClockInPin, previousClockInputState); 
      break;
    }
  }
  //--------------------
  euclidRhythmParameters = updateEuclidParams( clockMode, isNewRisingClockEdge, euclidRhythmParameters);

  bool shouldTrigger = isNewRisingClockEdge && euclid(euclidRhythmParameters.counter, euclidRhythmParameters.beats, euclidRhythmParameters.barLengthInBeats, euclidRhythmParameters.rotation);

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

EuclidRythmParameters updateEuclidParams( ClockMode mode, bool isNewRisingClockEdge, EuclidRythmParameters previousParams) {
  EuclidRythmParameters params = previousParams;
  if (isNewRisingClockEdge) {
    params.barLengthInBeats = map(analogRead(barLengthKnobPin), 0, 1023, 1, 16);
    params.beats = map(analogRead(hitsKnobPin), 0, 1023, 0, 16);
    params.counter = euclidRhythmParameters.counter + 1;  
    
    if(mode==external){
      //should this map based on number of beats in a bar
      params.rotation = map(analogRead(rotationKnobPin), 0, 1023, 0, 16);            
    }
  }
  return params;
}

unsigned int readTempoInput(){
  // add debounce?
  int inputFromRotationPin = analogRead(rotationKnobPin);
 

  return mapTempoInputToTempoInBpm(inputFromRotationPin);  
}
