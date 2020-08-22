const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;
int counter = 0;
bool previousGate = false;
//bool nextGate = false;
bool shouldTrigger = false;
unsigned long pulseWidth = 20;
unsigned long timeUntilInternalClockMode = 4000;
unsigned long timeOfLastClockInChange;
unsigned long timeOfLastPulseOut;
enum ClockMode {external, internal};
ClockMode clockMode = internal; //module starts up in internal clock mode... change this line to default the clock mode to external if needed


void setup()
{
  pinMode(clockOutPin, OUTPUT);
  pinMode(clockInPin, INPUT);
  digitalWrite(clockOutPin,LOW);
}

void loop()
{
  unsigned long currentTime = millis();
  //-----IS IT GETTING INPUT--------------------------//
  bool stateOfClockInPin = !digitalRead(clockInPin);
  //--------------------------------------------------//

  //------------risingEdge?------------------------------///
  //bool nextGate = stateOfClockInPin;
  
  bool inputHasChangedAndIsPositive = !previousGate && stateOfClockInPin;
//--------------------------------------------------------///

//-------------------which clock?---------------------------------//
  
  bool clockInputHasChanged = previousGate!=stateOfClockInPin;
  if(clockInputHasChanged){
    timeOfLastClockInChange = currentTime;
    clockMode = external;
  }
  if(currentTime-timeOfLastClockInChange>timeUntilInternalClockMode){
    clockMode = internal;
  }
  previousGate = stateOfClockInPin;
//---------------------------------------------------------//
  
  //-------------euclid & Should I send output--------------//
  if(inputHasChangedAndIsPositive){
    int barLength = map(analogRead(barLengthKnobPin),0,1023,1,16);
    int rotation = map(analogRead(rotationKnobPin),0,1023,0,16);
    int hits = map(analogRead(hitsKnobPin),0,1023,0,16);
    shouldTrigger = euclid(counter, hits, barLength, rotation);
    counter++;
  }

  if(shouldTrigger){
    digitalWrite(clockOutPin, HIGH);
    timeOfLastPulseOut = currentTime;
    shouldTrigger = false;
  } else if((currentTime - timeOfLastPulseOut) > pulseWidth){
    
    digitalWrite(clockOutPin, LOW);
  }
  //-------------------------------------//

}

bool euclid(int count, int hits, int barLength, int rotation){
  return (((counter + rotation)*hits)%barLength)<hits;
}
