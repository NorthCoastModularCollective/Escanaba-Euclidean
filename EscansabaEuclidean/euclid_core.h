#ifndef EUCLID_CORE_H // include guard
#define EUCLID_CORE_H

/*DATA TYPES*/
typedef unsigned long milliseconds;
struct EuclidRythmParameters {
  int barLengthInBeats; // rename barlen in beats??
  int beats; //rename to beats?
  int rotation;
  int counter;
};
enum ClockMode {external, internal};
struct InternalClock {
  bool isClockHigh;
  milliseconds timeOfLastPulse;
  int tempo; //should this be a float? what unit this in?
};
template <typename T1, typename T2>  struct tuple  {
  T1 a; // rename first
  T2 b; //rebane second
};

/*CORE (pure functions)*/
float convertBPMToPeriodInMillis(int bpm){
  float SecondsPerMinute = 60.0;
  float MillisPerSecond = 1000.0;
  return ((1.0/((float)bpm))*SecondsPerMinute*MillisPerSecond);  
}

InternalClock updateInternalClock(milliseconds currentTime, InternalClock clk){
  float noteDivision = 8.0;
  milliseconds PULSE_WIDTH = (milliseconds) (convertBPMToPeriodInMillis(clk.tempo)/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=PULSE_WIDTH){
    return InternalClock {!clk.isClockHigh, currentTime, clk.tempo};
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
                                    milliseconds TIME_UNTIL_INTERNAL_CLOCK_MODE
                                   ) {
      
 
  ClockMode clockModeToReturn = currentMode;
  milliseconds timeToReturn = timeOfLastClockChange;
  if (clockInputChanged) {
    clockModeToReturn = external;
    timeToReturn = currentTime;
  }
  if (currentTime - timeOfLastClockChange > TIME_UNTIL_INTERNAL_CLOCK_MODE) {
    clockModeToReturn = internal;
  }

  return clockModeToReturn;
}

bool detectNewRisingClockEdge(bool currentClockInputState, bool previousClockInputState) {
  bool hasChangedAndIsPositive = !previousClockInputState && currentClockInputState;
  return hasChangedAndIsPositive;
}

bool euclid(int count, int beats, int barLengthInBeats, int rotation) {
  return (((count + rotation) * beats) % barLengthInBeats) < beats;
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
    //should this be 1 - 40 so its not discontinuous??
    int minimumTempo = 1;
    int maximumTempo = 5;
    tempo = map(inputFromRotationPin,0,oneQuarterOfKnobRange-1,minimumTempo,maximumTempo);
  }
  return tempo;
}


#endif /* EUCLID_CORE_H */
