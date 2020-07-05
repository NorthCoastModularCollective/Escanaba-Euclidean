const int clockOutPin       = 1;
const int clockInPin        = 0;
const int barLengthKnobPin  = A1;
const int rotationKnobPin   = A3;
const int hitsKnobPin       = A2;
int counter = 0;
bool lastGate = false;
bool nextGate = false;
bool shouldTrigger = false;
unsigned long pulseWidth = 20;
unsigned long previousTime;

void setup()
{
  pinMode(clockOutPin, OUTPUT);
  pinMode(clockInPin, INPUT);
  digitalWrite(clockOutPin,LOW);
}

void loop()
{
  unsigned long currentTime = millis();
  bool nextGate = digitalRead(clockInPin);
  bool inputHasChangedAndIsPositive = !lastGate && nextGate;
  lastGate = nextGate;
  if(inputHasChangedAndIsPositive){
    int barLength = map(analogRead(barLengthKnobPin),0,1023,0,16);
    int rotation = map(analogRead(rotationKnobPin),0,1023,0,16);
    int hits = map(analogRead(hitsKnobPin),0,1023,0,16);
    shouldTrigger = euclid(counter, hits, barLength, rotation);
    counter++;
  }
  if(shouldTrigger){
    digitalWrite(clockOutPin, HIGH);
    previousTime = currentTime;
    shouldTrigger = false;
  } else if((currentTime - previousTime) > pulseWidth){
    
    digitalWrite(clockOutPin, LOW);
  }

}

bool euclid(int count, int hits, int barLength, int rotation){
  return (((counter + rotation)*hits)%barLength)>hits;
}
