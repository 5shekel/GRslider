// GRslider_v2.1 teensy
// classes

#include <AccelStepper.h>


#define DEBUG false

// http://arduino.cc/en/Tutorial/SerialEvent
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean stepper2go=false;


//GLOBAL STEPPER PROPERTIES
#define homePosition 1000
int stepper_speed = 3200; //up to 6400, 25600;

#define buttonPin 22

int stepper2_Speed = 500;
int stepper2_accl = 500;

#define step2DIR 21
#define step2STEP 20
#define limitPin2a 23
AccelStepper stepper2(1, step2STEP, step2DIR); //step / dir

int scalePos[] = {
  10, 63, 125, 180, 240};

void setup()
{
  Serial.begin(115200);
  inputString.reserve(200);

  pinMode(buttonPin, INPUT_PULLUP);   
  delay(100);
  ///wait here to start run becuase reset sucks on teensy
  while (digitalRead(buttonPin)) {
  }


  stepper2.setMaxSpeed(stepper2_Speed);
  pinMode(limitPin2a, INPUT_PULLUP); //arm the pullup 


  Serial.println("begin...");
  //run steppers until they hits the limit switch  	
  homeStepper2();    	

}

void loop()
{
  stepper2.runSpeedToPosition();
}

/////////// SERIAL CONTROL //////


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); 
    inputString += inChar;
    if (inChar == '|') {
      Serial.println(stepper2.currentPosition());
      //do something here
      //stepper2.setMaxSpeed(stepper2_Speed);
      //stepper2.setAcceleration(stepper2_accl);

      int gonext = scalePos[random(5)]- stepper2.currentPosition();
      //cheap trick to roll dice again if ZERO
      if (!gonext)
        gonext = scalePos[random(5)]- stepper2.currentPosition();

      stepper2.move(gonext);	
      //stepper2go=true;

      Serial.print(gonext);
      Serial.print(" : ");
      Serial.println(stepper2.distanceToGo());

      //reset stuff
      inputString = "";
    } 
  }
}
// better string split
// http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
//////////////////////////////////

////////// STEPPER SEQ ///////////

void homeStepper2()
{
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  Serial.println("stathome");
  stepper2.setSpeed(-500);
  while(digitalRead(limitPin2a))
    stepper2.runSpeed();

  stepper2.setCurrentPosition(0);

  Serial.println("steps:");
  Serial.println(stepper2.currentPosition());
}
//////////////////////////////////////












