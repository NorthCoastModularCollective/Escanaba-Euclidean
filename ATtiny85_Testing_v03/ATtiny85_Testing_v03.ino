int blinkPin = 0;
int clockInPin = 1;
int counter = 0;
bool lastGate= false;
bool nextGate = false;
bool shouldTrigger = false;
unsigned long pulseWidth = 20;
unsigned long previousTime;

void setup()
{
  pinMode(blinkPin, OUTPUT);
  pinMode(clockInPin, INPUT);
  digitalWrite(blinkPin,LOW);
}

void loop()
{
  unsigned long currentTime = millis();
  bool nextGate = !digitalRead(clockInPin);
  bool inputHasChanged = !lastGate && nextGate;
  if(inputHasChanged){
    lastGate = nextGate;
    shouldTrigger = true;
  }
  if(shouldTrigger){
    digitalWrite(blinkPin, HIGH);
    previousTime = millis();
    shouldTrigger = false;
  }

  if((currentTime - previousTime) > pulseWidth){
    
    digitalWrite(blinkPin, LOW);
  }
 // digitalWrite(blinkPin,inputHasChanged);
  /*
  bool nextGate = digitalRead(clockInPin);
  if(!lastGate && nextGate){
    counter++;
    shouldTrigger=!shouldTrigger;
  }
  lastGate = nextGate;

  if(shouldTrigger){
    digitalWrite(blinkPin, HIGH);
    previousTime = millis();
    shouldTrigger = false;
  }

  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - previousTime;

  if(elapsedTime > pulseWidth){
    digitalWrite(blinkPin, LOW);
  }
  */
}
