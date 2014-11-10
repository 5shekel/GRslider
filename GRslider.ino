// GRslider_v2.1 arduino mega
// classes

#include <AccelStepper.h>
#include <CmdMessenger.h>  // CmdMessenger

#define DEBUG true

//STEPPER PROPERTIES

//////// #1 //////////////
int stepper1_Speed = 5000;
int stepper1_accl = 4500;

#define limitPin1a 5
#define limitPin1b 4
#define step1_STEP 7
#define step1_DIR 8
#define step1_ENABLE 6
int limit_A = 0; //which limit pin are we tochin, 1 is home 2 is other
boolean stepperMove = false;
AccelStepper stepper1(1, step1_STEP, step1_DIR); //step / dir
///////////////////////////////

//////// #2 //////////////
int stepper2_Speed = 5000;
int stepper2_accl = 4500;

#define limitPin2a 3
#define step2_STEP 10
#define step2_DIR 11
#define step2_ENABLE 12
AccelStepper stepper2(1, step2_STEP, step2_DIR); //step / dir

#define arrLen 5
int scalePos[arrLen] = {
  100, 225, 425, 650, 850};
///////////////////////////////

///////////////////////////////
// how often you want the serial input to be checked.
int measurementInterval = 50;
// when the serial input was last checked.
long lastMeasurementTime = 0L;
int sensorPin = A5;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
int knobGo = 0;

// smoothing stage, via analog/smoothing example
const int numReadings = 5;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

//////////////////////////////////////////////////////////////////
// Attach a new CmdMessenger object to the default Serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial);
// This is the list of recognized commands.  
// In order to receive, attach a callback function to these events
enum
{
  kCommandList         , // Command to request list of available commands
  kSetSpeed              , // Command to set speed
  kSetAccel   , // Command to set acceleartion
  kGoStepper2           , // Command to move stepper
  kHome           , // Command to home stepper
  kSleep          , //comand to send motor to sleep
  kKnob           , //translate knob to motion
  kGoStepper1           , //pluck string
};
// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kCommandList, OnCommandList);
  cmdMessenger.attach(kSetSpeed, OnSetSpeed);
  cmdMessenger.attach(kSetAccel, OnSetaccel);
  cmdMessenger.attach(kGoStepper2, onGo2);
  cmdMessenger.attach(kHome, homeAll);
  cmdMessenger.attach(kSleep, sleepAll);
  cmdMessenger.attach(kKnob,  knobEnable);
  cmdMessenger.attach(kGoStepper1, onGo1);
}
// Called when a received command has no attached function
void OnUnknownCommand()
{
  Serial.println("This command is unknown!");
  ShowCommands();
}

// Callback function that shows a list of commands
void OnCommandList()
{
  ShowCommands();
}

// Show available commands
void ShowCommands() 
{
  Serial.println("Available commands");
  Serial.println(" 0;                 - This command list");
  Serial.println(" 1,<set speed stepper>;     - Set speed. 100-25600");
  Serial.print  (" 2,<set accelaration>; - Set acceleration 100-25600 "); 
  Serial.println(" 3;                  - go stepper 2");
  Serial.println(" 4;                  - home stepper");
  Serial.println(" 5;                  - sleep stepper");
  Serial.println(" 6;                  - knob controller");
  Serial.println(" 7;                  - pick movement (stepper 1)");
  showVals();
}

void OnSetaccel(){
  stepper1_accl = cmdMessenger.readInt16Arg(); 
  stepper2_accl = cmdMessenger.readInt16Arg(); 
  showVals();
}

void OnSetSpeed(){
  stepper1_Speed = cmdMessenger.readInt16Arg(); 
  stepper2_Speed = cmdMessenger.readInt16Arg(); 
  showVals();
}

void onGo1()
{
  //we call this to initiate the stepper 1 action
 // stepper1.enableOutputs();
  stepperMove = 1;
  stepper1.enableOutputs();
}

void onGo2(){
  //if before we had know, first go home then start the random stepping
  if(knobGo) homeAll();
  stepper2.enableOutputs();
  randStepper2();
}

void homeAll(){
  knobGo=0; //stop knob if its on
  stepper1.enableOutputs();
  stepper2.enableOutputs();
  //run steppers until they hits the limit switch   
  homeStepper1();  
  homeStepper2();  
}

void sleepAll(){
  knobGo=0;
  stepper1.disableOutputs();
  stepper2.disableOutputs();
  Serial.println("steppers sleep");
}

void knobEnable(){
  knobGo = 1;
}

void knobControl(){
    stepper2.enableOutputs();
    stepper2.setMaxSpeed(1000);
    stepper2.setAcceleration(10000);

    if (millis() > (lastMeasurementTime + measurementInterval)) {
      sensorValue = analogRead(sensorPin);
      sensorValue = map(sensorValue, 0 ,1024, 0, 800);
      
      total= total - readings[readIndex];         
      readings[readIndex] = sensorValue;
      total= total + readings[readIndex];       
      readIndex = readIndex + 1;                    
      if (readIndex >= numReadings)              
        readIndex = 0;                           

      average = total / numReadings; 

      stepper2.moveTo(average);
      Serial.println(average);

      lastMeasurementTime = millis();
    }
}

void showVals(){
  Serial.print("current speed 1: ");
  Serial.println(stepper1_Speed);
  Serial.print("current accel 1: ");
  Serial.println(stepper1_accl);   
  Serial.println(" ");
  Serial.print("current speed 2: ");
  Serial.println(stepper2_Speed);
  Serial.print("current accel 2: ");
  Serial.println(stepper2_accl); 
}


void stepper1_action(){
  if(stepperMove)
  {
    if(!digitalRead(limitPin1a) && limit_A==1)
    {
      limit_A = 2;
      stepperMove = false;
      stepper1.setSpeed(400);
      Serial.println("stop on limit_a");
      stepper1.disableOutputs();
    }
    else if(!digitalRead(limitPin1b) && limit_A==2)
    {
      limit_A = 1;
      stepperMove = false;
      stepper1.setSpeed(-400);
      Serial.println("stop on limit_b");
      stepper1.disableOutputs();
    }
    stepper1.runSpeed();
  }
}

void randStepper2(){
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setAcceleration(stepper2_accl);
  int dest = scalePos[random(arrLen-1)];

  if(DEBUG) Serial.println(dest);
  stepper2.moveTo(dest);
}


void setup()
{

  /////// stepper 1 ////////
  stepper1.setPinsInverted(14,0,1); //dir, step, enable
  stepper1.setMaxSpeed(stepper1_Speed);
  stepper1.setEnablePin(step1_ENABLE);
  stepper1.disableOutputs();
  
  pinMode(limitPin1a, INPUT_PULLUP); //arm the pullup for limit pin
  pinMode(limitPin1b, INPUT_PULLUP); //arm the pullup for limit pin

  /////// stepper 2 ////////
  stepper2.setPinsInverted(1,0,1); //dir, step, enable
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setEnablePin(step2_ENABLE);
  stepper2.disableOutputs();
  
  pinMode(limitPin2a, INPUT_PULLUP); //arm the pullup for limit pin

  if(DEBUG){
    Serial.begin(115200);
    //we need this to let it sit and wait for a serial connection from teensy
    while (Serial.available() <= 0) {
      Serial.println("waiting for terminal..."); 
      delay(300);
    }
    cmdMessenger.printLfCr();   // Adds newline to every command 
    attachCommandCallbacks();// Attach my application's user-defined callback methods
    ShowCommands(); // Show command list
  }

    // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;  

  //run steppers until they hits the limit switch   
  homeStepper1();
  homeStepper2();     

}

void loop()
{
  // Process incoming serial data, and perform callbacks
  if(DEBUG) cmdMessenger.feedinSerialData();

  //if mode 6, translate analog pin(A5) to motion
  if(knobGo) knobControl();
  
  stepper1_action();
  stepper2.run(); // slider stepper 02
}

/////////// SERIAL CONTROL //////


////////// STEPPER SEQ ///////////

void homeStepper1()
{
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  if(DEBUG)Serial.println("home_01");

  stepper1.enableOutputs();
  stepper1.setSpeed(400);
  while(digitalRead(limitPin1b))
    stepper1.runSpeed();

  if(DEBUG)Serial.println("steps:"); //how many steps did we take to reach home
  if(DEBUG)Serial.println(stepper1.currentPosition());

  // prime stepper for the action
  // for some reason runSpeed() doesnt work, so we do use acceleration.
  limit_A = 1;
  stepper1.setSpeed(-400);
  stepper1.disableOutputs();
}

void homeStepper2()
{
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  if(DEBUG)Serial.println("home_02");

  stepper2.enableOutputs();
  stepper2.setCurrentPosition(0);
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(10000);
  stepper2.moveTo(-4000);
  while(digitalRead(limitPin2a)){
    stepper2.run();
  }

  if(DEBUG)Serial.println("steps:"); //how many steps did we take to reach home
  if(DEBUG)Serial.println(stepper2.currentPosition());
  stepper2.setCurrentPosition(0);
  stepper2.disableOutputs();
}
//////////////////////////////////////
























