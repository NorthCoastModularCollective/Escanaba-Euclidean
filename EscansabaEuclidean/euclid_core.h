#ifndef EUCLID_CORE_H // include guard
#define EUCLID_CORE_H

/*DATA TYPES*/
typedef unsigned long milliseconds;
struct EuclidRhythmParameters {
  unsigned short barLengthInBeats; // rename barlen in beats??
  unsigned short beats; //rename to beats?
  unsigned short rotation;
  unsigned short counter;
};
enum ClockMode {external, internal};
struct InternalClock {
  bool isClockHigh;
  milliseconds timeOfLastPulse;
  int tempo; //should this be a float? what unit this in?
};
template <typename T1, typename T2>  
struct tuple  {
  T1 first;
  T2 second;
};

/*CORE (pure functions)*/
inline const float convertBPMToPeriodInMillis(const int& bpm){
  float SecondsPerMinute = 60.0;
  float MillisPerSecond = 1000.0;
  return ((1.0/((float)bpm))*SecondsPerMinute*MillisPerSecond);  
}

inline const InternalClock updateInternalClock(
  const milliseconds& currentTime, 
  const InternalClock& clk
){
  float noteDivision = 8.0;
  milliseconds PULSE_WIDTH = (milliseconds) (convertBPMToPeriodInMillis(clk.tempo)/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=PULSE_WIDTH){
    return InternalClock {!clk.isClockHigh, currentTime, clk.tempo};
  }
  return clk;
   
}

inline const tuple <bool, milliseconds>  didClockInputChange(
  const bool& stateOfClockInPin, 
  const bool& previousClockInputState, 
  const milliseconds& currentTime, 
  const milliseconds& timeOfLastClockChange
){
  bool didChange = previousClockInputState != stateOfClockInPin;
  milliseconds timeToReturn = timeOfLastClockChange;
  if (didChange) {
    timeToReturn=currentTime;
  }
  
  return tuple<bool, milliseconds> {didChange, timeToReturn};
}

inline const ClockMode whichClockModeShouldBeSet(
  const bool& clockInputChanged,
  const ClockMode& currentMode, 
  const milliseconds& currentTime, 
  const milliseconds& timeOfLastClockChange, 
  const milliseconds& TIME_UNTIL_INTERNAL_CLOCK_MODE
){
 
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

inline const bool detectNewRisingClockEdge(
  const bool& currentClockInputState, 
  const bool& previousClockInputState
){
  bool hasChangedAndIsPositive = !previousClockInputState && currentClockInputState;
  return hasChangedAndIsPositive;
}

inline const bool euclid(
  const unsigned short& count,
  const unsigned short& beats,
  const unsigned short& barLengthInBeats,
  const unsigned short& rotation
){
  return (((count + rotation) * beats) % barLengthInBeats) < beats;
}

inline const int mapTempoInputToTempoInBpm (const int& inputFromRotationPin){
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
