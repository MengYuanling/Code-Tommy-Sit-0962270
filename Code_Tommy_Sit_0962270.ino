//up and down tact buttons
int btnDown = 2; 
int btnUp = 3;

//H-Bridge pins
const int BridgeInputPin2 = 4; //-bridge input pin 2
const int BridgeInputPin1 = 5; //-bridge input pin 1
const int BridgeEnablePin = 6; //-bridge enable pin

//The pin number of the PING)) sensor's output.
const int pingPin = 7;

//Pin Definitions for 7-segment LED, which displays the floor number or indicates
//when the car is moving.
int clock = 8; 
int latch = 9; 
int data = 10; 
          

//the given numbers are what to send to the shift register in order to make the 7-segment
const int latchPin = 12;//Pin connected to ST_CP of 74HC595
const int clockPin = 8;//Pin connected to SH_CP of 74HC595 
const int dataPin = 11; //Pin connected to DS of 74HC595 
/*=======================================================================================================
//display 0,1,2,3,4,5,6,7,8*/
Number 0 :  00000011     3 
Number 1 :  10101111     159 
Number 2 :  00100101     37
Number 3 :  00001101     13
Number 4 :  10011001     153
Number 5 :  01001001     73
Number 6 :  01000001     65
Number 7 :  00011111     31
Number 8 :  00000001     1


//this stores the current position of the car - this value is updated each time the main loop code executes
int currentPosition = -1; 

//floor do display in the7-segment LED.  This is stored becuase we don't want to continually calculate it. 
//as the elevator moves between floors, it will show the last floor reached until it stops, at which point it
//will update the number with the current floor. 
int lastKnownFloorNumber = 0;

//this is the current destination, in cms from the sensor
//the motor will run in the correct direction to reach this position, then it will stop.
int destinationPosition = -1;  
int lastDestinationReached = -1; 

//pin for the piezo buzzer
int speakerPin = 12;

void setup()
{
  //set up serial communication to receive debugging information
  Serial.begin(9600);
  
  //set up pins for the h-bridge
  pinMode(hBridgeInputPin1, OUTPUT); 
  pinMode(hBridgeInputPin2, OUTPUT); 
  
  digitalWrite(hBridgeInputPin1, LOW); 
  digitalWrite(hBridgeInputPin2, HIGH);
  
  //up and down button pins are used for input
  pinMode(btnUp, INPUT);
  pinMode(btnDown, INPUT);

  //set up pins for shift register - these are outputs 
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);
  
  pinMode(speakerPin, OUTPUT);
  
  //set the initial destination to be the first floor by default.  When the loop() function starts running,
  //it will move the car to the initial position if it's not already there.
  InitializeElevatorPosition(); 
}

void loop()
{
  currentPosition = GetCurrentCarPositionCms(); 
  
  ReadButtons(); 
 
  ControlMotor();  
}

void ReadButtons()
{
  //the car is moving - we don't read buttons while it's moving
  if (currentPosition != destinationPosition)
  {
    return; 
  }

  DisplayCurrentFloorNumber(); 
   
  //was the Up button pressed? If the car is stopped, adjust the destination distance, and run the motor to go up.
  if (digitalRead(btnUp) == LOW) 
   {
     if (currentPosition < topFloorPositionCm)
       destinationPosition += floorHeightCm; 
     
     return; 
  }

   //was the Down button pressed? If the car is stopped, reduce the destination distance and run the motor
   if (digitalRead(btnDown) == LOW) 
   {
      if (currentPosition > groundFloorPositionCm)
        destinationPosition -= floorHeightCm; 

      return; 
    }
}

//when the program starts, move it to the "ground" floor
void InitializeElevatorPosition()
{
  destinationPosition = groundFloorPositionCm;
}

//using the PING))), get the number of centimeters from the sensor
//to the bottom of the elevator car.
int GetCurrentCarPositionCms()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  return MicrosecondsToCentimeters(duration);
}

// Runs motor, if necessary, in the right direction to reach the 
// current destination.  Stops motor (or keeps motor stopped) if at the destination position
void ControlMotor()
{
  ////we're in the position we want - stop the motor, or leave it stopped.
  if (currentPosition == destinationPosition)
  {    
    // stop the car
    analogWrite(hBridgeEnablePin, 0);
    if (destinationPosition != lastDestinationReached)
    {
      delay(1000); 
      PlayTone(1915, 2000); 
      lastDestinationReached = destinationPosition;
      }
      
    return; 
  }

  if (currentPosition > destinationPosition) 
  {
    //destination is below the current position - run the motor clockwise to lower the car
    digitalWrite(hBridgeInputPin1, HIGH); 
    digitalWrite(hBridgeInputPin2, LOW); 
  }
  else 
  {
    //run motor clockwise - lower the car
    digitalWrite(hBridgeInputPin1, LOW); 
    digitalWrite(hBridgeInputPin2, HIGH); 
  }

  //make sure the motor is running
  //use the map() function to control the speed of the motor (between 0 and 255)
  analogWrite(hBridgeEnablePin, 255); 
}


void PlayTone(int tone, int duration) 
{
  for (long i = 0; i < duration * 1000L; i += tone * 2) 
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

int DisplayCurrentFloorNumber()
{

/*
* OutputFloorNumber() - sends the LED states set in ledStates to the 74HC595
* sequence
*/
void OutputFloorNumber(int value)
{
  //Pulls the chips latch low
  digitalWrite(latch, LOW);     

  //Shifts out the 8 bits to the shift register.  This is a built-in Arduino function.
  shiftOut(data, clock, MSBFIRST, ledNumbers[value]); 
  
