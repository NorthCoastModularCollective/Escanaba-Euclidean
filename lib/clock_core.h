#ifndef CLOCK_CORE_H // include guard
#define CLOCK_CORE_H

#include "tuple.h"

typedef unsigned long milliseconds;

enum ClockMode {external, internal};

struct InternalClock {
  bool isClockHigh;
  milliseconds timeOfLastPulse;
  unsigned int basePeriod; //should this be a float? what unit this in?
  unsigned long counter;
  const bool isOffbeat(){
    return counter%2!=0;
  }
};

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

inline const float convertBPMToPeriodInMillis(const int& bpm){
  float SecondsPerMinute = 60.0;
  float MillisPerSecond = 1000.0;
  return ((1.0/((float)bpm))*SecondsPerMinute*MillisPerSecond);  
}

inline const InternalClock updateInternalClock(
  const milliseconds& currentTime, 
  InternalClock clk
){
  float noteDivision = 8.0;
  milliseconds period = (milliseconds) (clk.basePeriod/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=period){
    clk.isClockHigh = !clk.isClockHigh;
    clk.timeOfLastPulse = currentTime;
  }
  return clk;
   
}

enum ClockMultiplyDivideRanges {
  EIGHT_DIV,
  FOUR_DIV,
  THREE_DIV,
  TWO_DIV,
  ONE_POINT_FIVE_DIV,
  ONE,
  ONE_POINT_FIVE_MULT,
  TWO_MULT,
  THREE_MULT,
  FOUR_MULT,
  EIGHT_MULT,
  __SIZE__ 
};

inline const float computeNoteDivision(const ClockMultiplyDivideRanges& clockModifier){
  float noteDivision=8.0;
  switch (clockModifier)
  {
  case EIGHT_DIV:
    noteDivision /= 8.0;
    break;
  case FOUR_DIV:
    noteDivision /= 4.0;
    break;
  case THREE_DIV:
    noteDivision /= 3.0;
    break;
  case TWO_DIV:
    noteDivision /= 2.0;
    break;
  case ONE_POINT_FIVE_DIV:
    noteDivision /= 1.5;
    break;
  case ONE_POINT_FIVE_MULT:
    noteDivision *= 1.5;
    break;
  case TWO_MULT:
    noteDivision *= 2.0;
    break;
  case THREE_MULT:
   noteDivision *= 3.0;
    break;
  case FOUR_MULT:
    noteDivision *= 4.0;
    break;
  case EIGHT_MULT:
    noteDivision *= 8.0;
    break;
  }
  return noteDivision;
}

inline const InternalClock updateInternalClock(
  const milliseconds& currentTime, 
  InternalClock clk,
  const ClockMultiplyDivideRanges& clockModifier,
  const float& swingAmount
){
  float noteDivision = computeNoteDivision(clockModifier);
  float tempoAsPeriod = clk.basePeriod;
  milliseconds period = clk.isOffbeat()
                        ?(tempoAsPeriod/noteDivision) * (1.0 + swingAmount)
                        :(tempoAsPeriod/noteDivision) * (1.0 - swingAmount);
  
  //add swing offset if on offbeat
  if((currentTime-clk.timeOfLastPulse)>=period){
    clk.isClockHigh = !clk.isClockHigh;
    clk.timeOfLastPulse = currentTime;
    if (clk.isClockHigh) clk.counter++;

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
    int maximumTempo = 40;
    tempo = map(inputFromRotationPin,0,oneQuarterOfKnobRange-1,minimumTempo,maximumTempo);
  }
  return tempo;
}

#endif /* CLOCK_CORE_H */
