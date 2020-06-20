int           blinkPin      = 0;
int           clockInPin    = 1;
unsigned long pulseWidth    = 5;
unsigned long debounceTime  = 5;
unsigned long previousTime  = 0;
unsigned long lastReadTime  = 0;
int           counter       = 0;
bool          lastGate      = false;
bool          shouldTrigger = false;


void setup()
{
  pinMode(blinkPin, OUTPUT);
  pinMode(clockInPin, INPUT);
}

void loop()
{
  unsigned long currentTime = millis();

  bool nextGate = digitalRead(clockInPin);
  bool isDebounced = (currentTime-lastReadTime) > debounceTime;
  bool inputHasChanged = !lastGate && nextGate;

  if(inputHasChanged && isDebounced){
    lastGate = nextGate;
    lastReadTime = currentTime;
    
    counter++;
    //shouldTrigger=!shouldTrigger;
    int rotation = 0;
    int hits = 3;
    int barLength = 8;
    shouldTrigger = euclid(counter,hits,barLength,rotation);

  }

  if(shouldTrigger){
    digitalWrite(blinkPin, HIGH);
    previousTime = millis();
    shouldTrigger = false;
  }

  if((currentTime - previousTime) > pulseWidth){
    digitalWrite(blinkPin, LOW);
  }
 
}

bool euclid(int count, int hits, int barLength, int rotation){
  return (((counter + rotation)*hits)%barLength)>hits;
}
