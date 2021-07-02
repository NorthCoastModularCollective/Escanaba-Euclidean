#ifndef EUCLID_CORE_H // include guard
#define EUCLID_CORE_H

/*DATA TYPES*/
typedef unsigned long milliseconds_t;
typedef unsigned long bpm_t;
typedef unsigned long count_t;
typedef float period_t;

template <typename T1, typename T2>  struct tuple  {
  T1 first;
  T2 second;
};

enum ClockMode {external, internal};

struct InternalClock {
  bool clockSignalState;
  milliseconds_t timeOfLastPulse;
  bpm_t tempo; //should this be a float? what unit this in?
};

struct euclid_t {
  count_t barLengthInBeats;
  count_t beats;
  count_t rotation;
  count_t counter; 
  static tuple<bool,euclid_t> process(euclid_t e){
    e.counter++;
    const bool shouldTrigger = (((e.counter + e.rotation) * e.beats) % e.barLengthInBeats) < e.beats;
    return tuple<bool,euclid_t>{shouldTrigger,e};
  }
};


/*CORE (pure functions)*/
const inline period_t convertBPMToPeriodInMillis(const bpm_t& bpm){
  period_t SecondsPerMinute = 60.0;
  period_t MillisPerSecond = 1000.0;
  return ((1.0/period_t(bpm))*SecondsPerMinute*MillisPerSecond);  
}

const inline InternalClock updateInternalClock(const milliseconds_t& currentTime, const InternalClock& clk){
  const float noteDivision = 8.0;
  milliseconds_t PULSE_WIDTH = (milliseconds_t) (convertBPMToPeriodInMillis(clk.tempo)/noteDivision);
  if((currentTime-clk.timeOfLastPulse)>=PULSE_WIDTH){
    return InternalClock {!clk.clockSignalState, currentTime, clk.tempo};
  }
  return clk;
   
}

const inline tuple <bool, milliseconds_t>  didClockInputChange(
  const bool& stateOfClockInPin, 
  const bool& previousClockInputState, 
  const milliseconds_t& currentTime, 
  const milliseconds_t& timeOfLastClockChange
) {
  bool didChange = previousClockInputState != stateOfClockInPin;
  milliseconds_t timeToReturn = timeOfLastClockChange;
  if (didChange) {
    timeToReturn=currentTime;
  }
  
  return tuple<bool, milliseconds_t> {didChange, timeToReturn};
}

const inline ClockMode whichClockModeShouldBeSet(bool clockInputChanged,
                                    const ClockMode& currentMode, 
                                    const milliseconds_t& currentTime, 
                                    const milliseconds_t& timeOfLastClockChange, 
                                    const milliseconds_t& timeUntilInternalClockMode
                                   ) {
      
  ClockMode clockModeToReturn = currentMode;
  milliseconds_t timeToReturn = timeOfLastClockChange;
  if (clockInputChanged) {
    clockModeToReturn = external;
    timeToReturn = currentTime;
  }
  if (currentTime - timeOfLastClockChange > timeUntilInternalClockMode) {
    clockModeToReturn = internal;
  }

  return clockModeToReturn;
}

const inline bool detectNewRisingClockEdge(bool currentClockInputState, bool previousClockInputState) {
  bool hasChangedAndIsPositive = !previousClockInputState && currentClockInputState;
  return hasChangedAndIsPositive;
}


const inline int mapTempoInputToTempoInBpm (const int& inputFromRotationPin){
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
