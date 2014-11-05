// GRslider_v2.1 teensy
// classes

#include <AccelStepper.h>
#include <CmdMessenger.h>  // CmdMessenger

#define DEBUG true

//GLOBAL STEPPER PROPERTIES

int stepper2_Speed = 5000;
int stepper2_accl = 500;

#define step2DIR 21
#define step2STEP 20
#define limitPin2a 23
AccelStepper stepper2(1, step2STEP, step2DIR); //step / dir

#define arrLen 5
int scalePos[arrLen] = {
  10, 63, 125, 180, 240};

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
  kGo           , // Command to move stepper
};
// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kCommandList, OnCommandList);
  cmdMessenger.attach(kSetSpeed, OnSetSpeed);
  cmdMessenger.attach(kSetAccel, OnSetaccel);
  cmdMessenger.attach(kGo, OnGo);
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
  Serial.println(" 1,<set speed>;     - Set speed. 100-25600");
  Serial.print  (" 2,<set accelaration>; - Set acceleration 100-25600 "); 
  Serial.println(" 3;                  - go stepper");
  showVals();
}

void OnSetaccel(){
  stepper2_accl = cmdMessenger.readInt16Arg(); 
  showVals();
}

void OnSetSpeed(){
  stepper2_Speed = cmdMessenger.readInt16Arg(); 
  showVals();
}

void OnGo(){
  runStepper2();
}

void showVals(){
  Serial.print("current speed: ");
  Serial.println(stepper2_Speed);
  Serial.print("current accel: ");
  Serial.println(stepper2_accl); 
}

void runStepper2(){
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setAcceleration(stepper2_accl);
  int dest = scalePos[random(arrLen-1)];

  if(DEBUG) Serial.println(dest);
  stepper2.moveTo(dest);
}


void setup()
{
  if(DEBUG){
    Serial.begin(115200);
    //we need this to let it sit and wait for a serial connection from teensy
    while (Serial.available() <= 0) {
      Serial.println("waiting for terminal... send me something"); 
      delay(300);
    }
    cmdMessenger.printLfCr();   // Adds newline to every command 
    attachCommandCallbacks();// Attach my application's user-defined callback methods
    ShowCommands(); // Show command list
  }


  stepper2.setMaxSpeed(stepper2_Speed);
  pinMode(limitPin2a, INPUT_PULLUP); //arm the pullup 

  //run steppers until they hits the limit switch   
  homeStepper2();     

}

void loop()
{
  // Process incoming serial data, and perform callbacks
  if(DEBUG) cmdMessenger.feedinSerialData();

  stepper2.run();

}

/////////// SERIAL CONTROL //////


////////// STEPPER SEQ ///////////

void homeStepper2()
{
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  if(DEBUG)Serial.println("stathome");
  stepper2.setSpeed(-500);
  while(digitalRead(limitPin2a))
    stepper2.runSpeed();

  if(DEBUG)Serial.println("steps:"); //how many steps did we take to reach home
  stepper2.setCurrentPosition(0);
  if(DEBUG)Serial.println(stepper2.currentPosition());
}
//////////////////////////////////////
























