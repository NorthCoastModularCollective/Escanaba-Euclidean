
#include "euclid_core.h"
/* CONFIG */
static const milliseconds_t PULSE_WIDTH = 10; 
static const milliseconds_t TIME_UNTIL_INTERNAL_CLOCK_MODE = 4000; 
static const ClockMode CLOCK_MODE_ON_STARTUP = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed

/* HARDWARE */
const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;

/* GLOBAL MUTABLE STATE */
milliseconds_t timeOfLastClockInChange;
milliseconds_t timeOfLastPulseOut;
euclid_t euclidRythmState;
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
  const milliseconds_t currentTime = millis();

  const bool isNewRisingClockEdge = setClockModeAndUpdateClock(currentTime);
  const bool shouldTrigger = handleEuclidAlgorithmAndUpdateEuclidParams(isNewRisingClockEdge);
  handleTriggerOutput(shouldTrigger, currentTime);
  
}

/* SHELL (IO) */

bool setClockModeAndUpdateClock(const milliseconds_t& currentTime){

  bool stateOfClockInPin = readClockInput();

  tuple <bool, milliseconds_t> ifChangedAndTime = didClockInputChange(stateOfClockInPin, previousClockInputState, currentTime, timeOfLastClockInChange);
  bool clockInputChanged = ifChangedAndTime.first;
  timeOfLastClockInChange = ifChangedAndTime.second;

  clockMode = whichClockModeShouldBeSet(clockInputChanged, clockMode, currentTime, timeOfLastClockInChange, TIME_UNTIL_INTERNAL_CLOCK_MODE);
  
  bool isNewRisingClockEdge;

  switch(clockMode){
    case internal: {
      bool previousInternalClockState = internalClock.clockSignalState;
      internalClock.tempo = readTempoInput();
      internalClock = updateInternalClock(currentTime, internalClock); 
      isNewRisingClockEdge = detectNewRisingClockEdge(internalClock.clockSignalState, previousInternalClockState);
      break;
    }
    case external: {
      isNewRisingClockEdge = detectNewRisingClockEdge(stateOfClockInPin, previousClockInputState); 
      break;
    }
  }
  previousClockInputState = stateOfClockInPin;

  return isNewRisingClockEdge;
}

bool handleEuclidAlgorithmAndUpdateEuclidParams(const bool& isNewRisingClockEdge){
  euclidRythmState = readEuclidParams( clockMode, isNewRisingClockEdge, euclidRythmState);
  const tuple<bool, euclid_t> shouldTriggerAndEuclidState = euclid_t::process(euclidRythmState);
  euclidRythmState = shouldTriggerAndEuclidState.second;
  bool shouldTrigger = shouldTriggerAndEuclidState.first;
  shouldTrigger &= isNewRisingClockEdge;
  return shouldTrigger;
}

void handleTriggerOutput(const bool& shouldTrigger, const milliseconds_t& currentTime) {
  milliseconds_t timeOfPulseOutToReturn = timeOfLastPulseOut;
  if (shouldTrigger) {
    digitalWrite(clockOutPin, HIGH);
    timeOfPulseOutToReturn = currentTime;
  } else if ((currentTime - timeOfLastPulseOut) > PULSE_WIDTH) {

    digitalWrite(clockOutPin, LOW);
  }
  timeOfLastPulseOut = timeOfPulseOutToReturn;
}

bool readClockInput() {
  return !digitalRead(clockInPin);
}

euclid_t readEuclidParams(const ClockMode& mode, const bool& isNewRisingClockEdge, euclid_t params) {

  if (isNewRisingClockEdge) {
    params.barLengthInBeats = map(analogRead(barLengthKnobPin), 0, 1023, 1, 16);
    params.beats = map(analogRead(hitsKnobPin), 0, 1023, 0, 16);
    // params.counter = euclidRythmParameters.counter + 1;  
    
    if(mode==external){
      //should this map based on number of beats in a bar
      params.rotation = map(analogRead(rotationKnobPin), 0, 1023, 0, 16);            
    }
  }
  return params;
}

unsigned int readTempoInput(){
  // add debounce?
  const int inputFromRotationPin = analogRead(rotationKnobPin);

  return mapTempoInputToTempoInBpm(inputFromRotationPin);  
}
