int blinkPin = 0;
int clockInPin = 1;
int counter = 0;
bool lastGate= false;
bool shouldTrigger = false;
unsigned long pulseWidth = 5;
unsigned long previousTime;

void setup()
{
  pinMode(blinkPin, OUTPUT);
  pinMode(clockInPin, INPUT);
}

void loop()
{
  bool gateIn = digitalRead(clockInPin);
  digitalWrite(blinkPin,gateIn);
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
