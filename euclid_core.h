#ifndef EUCLID_CORE_H // include guard
#define EUCLID_CORE_H

/*DATA TYPES*/
typedef unsigned long milliseconds;
struct EuclidRythmParameters {
  int barLength;
  int hits;
  int rotation;
  int phase;
};
enum ClockMode {external, internal};
struct InternalClock {
  bool state;
  milliseconds timeOfLastPulse;
  int tempo;
};
template <typename T1, typename T2>  struct tuple  {
  T1 a;
  T2 b;
};

/*CORE (pure functions)*/
float convertBPMToPeriodInMillis(int bpm){
  float SecondsPerMinute = 60.0;
  float MillisPerSecond = 1000.0;
  return ((1.0/((float)bpm))*SecondsPerMinute*MillisPerSecond);  
}

InternalClock updateInternalClock(milliseconds currentTime, InternalClock clk){
  float noteDivision = 8.0;
  milliseconds pulseWidth = (milliseconds) (convertBPMToPeriodInMillis(clk.tempo)/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=pulseWidth){
    return InternalClock {!clk.state, currentTime, clk.tempo};
  }
  return clk;
   
}

tuple <bool, milliseconds>  didClockInputChange(bool stateOfClockInPin, bool previousClockInputState, milliseconds currentTime, milliseconds timeOfLastClockChange) {
  bool didChange = previousClockInputState != stateOfClockInPin;
  milliseconds timeToReturn = timeOfLastClockChange;
  if (didChange) {
    timeToReturn=currentTime;
  }
  
  return tuple<bool, milliseconds> {didChange, timeToReturn};
}

ClockMode whichClockModeShouldBeSet(bool clockInputChanged,
                                    ClockMode currentMode, 
                                    milliseconds currentTime, 
                                    milliseconds timeOfLastClockChange, 
                                    milliseconds timeUntilInternalClockMode
                                   ) {
      
 
  ClockMode clockModeToReturn = currentMode;
  milliseconds timeToReturn = timeOfLastClockChange;
  if (clockInputChanged) {
    clockModeToReturn = external;
    timeToReturn = currentTime;
  }
  if (currentTime - timeOfLastClockChange > timeUntilInternalClockMode) {
    clockModeToReturn = internal;
  }

  return clockModeToReturn;
}

bool detectNewRisingClockEdge(bool currentClockInputState, bool previousClockInputState) {
  bool hasChangedAndIsPositive = !previousClockInputState && currentClockInputState;
  return hasChangedAndIsPositive;
}

bool euclid(int count, int hits, int barLength, int rotation) {
  return (((count + rotation) * hits) % barLength) < hits;
}

int mapTempoInputToTempoInBpm (int inputFromRotationPin){
  const int oneQuarterOfKnobRange = 256;
  long tempo;
  if(inputFromRotationPin>=3*oneQuarterOfKnobRange){
    int minimumTempo = 240;
    int maximumTempo = 750;
    tempo = map(inputFromRotationPin,3*oneQuarterOfKnobRange,4*oneQuarterOfKnobRange-1,minimumTempo,maximumTempo);
  }else if(inputFromRotationPin>=oneQuarterOfKnobRange && inputFromRotationPin<(3*oneQuarterOfKnobRange)){
    int minimumTempo = 40;
    int maximumTempo = 240;
    tempo = map(inputFromRotationPin,oneQuarterOfKnobRange,3*oneQuarterOfKnobRange-1,minimumTempo,maximumTempo);
  }else{
    int minimumTempo = 1;
    int maximumTempo = 5;
    tempo = map(inputFromRotationPin,0,oneQuarterOfKnobRange-1,minimumTempo,maximumTempo);
  }
  return tempo;
}


#endif /* EUCLID_CORE_H */
