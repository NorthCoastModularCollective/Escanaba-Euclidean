
/*DATA TYPES*/
struct EuclidRythmParameters {
  int barLength;
  int hits;
  int rotation;
  int phase;
};
enum ClockMode {external, internal};
struct InternalClock {
  bool state;
  unsigned long timeOfLastPulse;
  int tempo;
};

/* CONFIG */
unsigned long pulseWidth = 20;
unsigned long timeUntilInternalClockMode = 4000;
ClockMode clockModeOnStartup = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed

/* HARDWARE */
const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;

/* GLOBAL STATE */
unsigned long timeOfLastClockInChange;
unsigned long timeOfLastInternalClock;
unsigned long timeOfLastPulseOut;
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
  unsigned long currentTime = millis();

  bool stateOfClockInPin = readClockInput();

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

  if(isNewRisingClockEdge){
    euclidRythmParameters.phase = euclidRythmParameters.phase + 1;  
  }

  bool clockInputChanged = didClockInputChange(stateOfClockInPin, previousClockInputState);
  if (clockInputChanged) {
    timeOfLastClockInChange = currentTime;
  }
  clockMode = whichClockModeShouldBeSet(clockInputChanged, clockMode, currentTime, timeOfLastClockInChange, timeUntilInternalClockMode);

  euclidRythmParameters = readEuclidInputs( clockMode, isNewRisingClockEdge, euclidRythmParameters);

  bool shouldTrigger = isNewRisingClockEdge && euclid(euclidRythmParameters.phase, euclidRythmParameters.hits, euclidRythmParameters.barLength, euclidRythmParameters.rotation);

  timeOfLastPulseOut = processTriggerOutput(shouldTrigger, timeOfLastPulseOut, currentTime, pulseWidth);

  previousClockInputState = stateOfClockInPin;


}

/* SHELL (IO) */

unsigned long processTriggerOutput(bool shouldTrigger, unsigned long timeOfLastPulseOut, unsigned long currentTime, unsigned long pulseWidth) {
  unsigned long timeOfPulseOutToReturn = timeOfLastPulseOut;
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

EuclidRythmParameters readEuclidInputs( ClockMode mode, bool isNewRisingClockEdge, EuclidRythmParameters previousParams) {
  EuclidRythmParameters params = previousParams;
  if (isNewRisingClockEdge) {
    params.barLength = map(analogRead(barLengthKnobPin), 0, 1023, 1, 16);
    params.hits = map(analogRead(hitsKnobPin), 0, 1023, 0, 16);
    
    if(mode==external){
      params.rotation = map(analogRead(rotationKnobPin), 0, 1023, 0, 16);            
    }
  }
  return params;
}

unsigned long readTempoInput(){
  return map(analogRead(rotationKnobPin), 0, 1023, 40, 240);  
}

/*CORE (pure functions)*/
InternalClock updateInternalClock(unsigned long currentTime, InternalClock clk){
  float noteDivision = 8.0;
  unsigned long pulseWidth = (unsigned long) (convertBPMToPeriodInMillis(clk.tempo)/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=pulseWidth){
    return InternalClock {!clk.state, currentTime, clk.tempo};
  }
  return clk;
}

float convertBPMToPeriodInMillis(int bpm){
  float SecondsPerMinute = 60.0;
  float MillisPerSecond = 1000.0;
  return ((1.0/((float)bpm))*SecondsPerMinute*MillisPerSecond);  
}

ClockMode whichClockModeShouldBeSet(bool clockInputHasChanged, ClockMode currentMode, unsigned long currentTime, unsigned long timeOfLastClockChange, unsigned long timeUntilInternalClockMode) {

  ClockMode clockModeToReturn = currentMode;
  if (clockInputHasChanged) {
    clockModeToReturn = external;
  }
  if (currentTime - timeOfLastClockInChange > timeUntilInternalClockMode) {
    clockModeToReturn = internal;
  }

  return clockModeToReturn;
}

bool didClockInputChange(bool stateOfClockInPin, bool previousClockInputState) {
  return previousClockInputState != stateOfClockInPin;
}

bool detectNewRisingClockEdge(bool currentClockInputState, bool previousClockInputState) {
  bool hasChangedAndIsPositive = !previousClockInputState && currentClockInputState;
  return hasChangedAndIsPositive;
}

bool euclid(int count, int hits, int barLength, int rotation) {
  return (((count + rotation) * hits) % barLength) < hits;
}
