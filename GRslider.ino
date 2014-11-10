// GRslider_v2.1 arduino mega
// classes

#include <AccelStepper.h>
#include <CmdMessenger.h>  // CmdMessenger

#define DEBUG true

//GLOBAL STEPPER PROPERTIES

int stepper2_Speed = 5000;
int stepper2_accl = 4500;

#define limitPin2a 9
#define step2STEP 10
#define step2DIR 11
#define step2ENABLE 12
AccelStepper stepper2(1, step2STEP, step2DIR); //step / dir

#define arrLen 5
int scalePos[arrLen] = {
  100, 225, 425, 650, 850};
///////////////////////////////
// how often you want the serial input to be checked.
int measurementInterval = 50;
// when the serial input was last checked.
long lastMeasurementTime = 0L;
int sensorPin = A5;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
int knobGo = 0;

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
  kGo           , // Command to move stepper
  kHome           , // Command to home stepper
  kSleep          , //comand to send motor to sleep
  kKnob           , //translate knob to motion
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
  cmdMessenger.attach(kHome, home2);
  cmdMessenger.attach(kSleep, sleep2);
  cmdMessenger.attach(kKnob, knobMotion);
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
  Serial.println(" 4;                  - home stepper");
  Serial.println(" 5;                  - sleep stepper");
  Serial.println(" 6;                  - knob movement");
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
  if(knobGo) home2();
  stepper2.enableOutputs();
  randStepper2();
}

void home2(){
  knobGo=0; //stop knob if its on
  stepper2.enableOutputs();
  //run steppers until they hits the limit switch   
  homeStepper2();  
}

void sleep2(){
  knobGo=0;
  stepper2.disableOutputs();
  Serial.println("stepper 2 sleep");
}

void knobMotion(){
  knobGo = 1;
  stepper2.enableOutputs();
}

void showVals(){
  Serial.print("current speed: ");
  Serial.println(stepper2_Speed);
  Serial.print("current accel: ");
  Serial.println(stepper2_accl); 
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

  stepper2.setPinsInverted(1,0,1); //dir, step, enable
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setEnablePin(step2ENABLE);
  stepper2.enableOutputs();
  
  pinMode(limitPin2a, INPUT_PULLUP); //arm the pullup for limit pin

  //run steppers until they hits the limit switch   
  homeStepper2();     

}

void loop()
{
  // Process incoming serial data, and perform callbacks
  if(DEBUG) cmdMessenger.feedinSerialData();

  //if mode 6, translate analog pin(A5) to motion
  if(knobGo){
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

  stepper2.run();

}

/////////// SERIAL CONTROL //////


////////// STEPPER SEQ ///////////

void homeStepper2()
{
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  if(DEBUG)Serial.println("stathome");

  stepper2.setCurrentPosition(0);
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(10000);
  stepper2.moveTo(-4000);
  while(digitalRead(limitPin2a)){
    //stepper2.runSpeed();
    stepper2.run();
  }

  if(DEBUG)Serial.println("steps:"); //how many steps did we take to reach home
  if(DEBUG)Serial.println(stepper2.currentPosition());
  stepper2.setCurrentPosition(0);
  stepper2.disableOutputs();
}
//////////////////////////////////////
























