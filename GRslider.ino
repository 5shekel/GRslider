// GRslider for arduino mega 2560

#define DEBUG true

///// MIDI ///////
#include "MIDI.h"
#include "midi_Defs.h"
#include "midi_Message.h"
#include "midi_Namespace.h"
#include "midi_Settings.h"
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midiA);
////////////////////////

#include <AccelStepper.h>
#include "Streaming.h"


//////////////////////////
//Define Relay pins
int RELAYS[6] = {
  38,40,42,44,46,48};
//////////////////////////

//STEPPER PROPERTIES
//////// #1 //////////////
int stepper1_Speed = 400;
// micro stepping pin for fret (stepper 1)
int MS1_pin = 24;
int MS2_pin = 26;
int MS3_pin = 28;

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

int action; //see eventclass for more


void stepper1_action(){
  if(stepperMove)
  {
    if(!digitalRead(limitPin1a) && limit_A==1)
    {
      limit_A = 2;
      stepperMove = false;
      stepper1.setSpeed(stepper1_Speed);
      stepper1.disableOutputs();
    }
    else if(!digitalRead(limitPin1b) && limit_A==2)
    {
      limit_A = 1;
      stepperMove = false;
      stepper1.setSpeed(-stepper1_Speed);
      stepper1.disableOutputs();
    }
    stepper1.runSpeed();
  }
}

void handlePitchWheel(byte channel, int bend){
  bend = constrain(bend, 0, 8192); // "Bend" is a int value between -8192 .. 8192. "0" is the middle --> no change in pitch
  int iPos = map(bend, 0, 8192, 0, 792);  
  stepper2.moveTo(iPos);
  stepper2.setSpeed(stepper2_Speed);

  if(DEBUG) Serial << "channel, bend, iPos " << channel << " / "<< bend << " / "<< iPos <<endl;
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  pitchToaction(pitch, velocity);
  if(action!= 99) switchesOn(action);
  
  if(DEBUG) Serial << "ON: pitch/action/val> " << pitch << " / "<< action << " / "<< velocity <<endl;
}

void HandleNoteOff(byte channel, byte pitch, byte velocity){
  pitchToaction(pitch, velocity);
  //if(DEBUG) Serial << "OFF: pitch/action/val> " << pitch << " / "<< action << " / "<< velocity <<endl;
  if(action!= 99) switchsOff(action);
}

// map the midi key (pitch) to a serialize number 
void pitchToaction(int I_pitch, int I_velocity){
  if(I_pitch == 83) action = 0; //home

  if(I_pitch==45) { //strummm stepper 1
      stepper1.enableOutputs();
      stepper1_Speed = map(I_velocity, 0, 127, 50, 4000); //needs to check range
      stepperMove = 1; //see stepper1_action at loop()
    }

  // mapping solenoids knocks 
  if(I_pitch==36) action = 14;
  if(I_pitch==37) action = 15;
  if(I_pitch==38) action = 16;
  if(I_pitch==39) action = 17;
  if(I_pitch==40) action = 18;
  if(I_pitch==41) action = 19;


  //mapping frets , maping starts at 20
  //if(I_pitch >= 47 && I_pitch <= 65)    action = I_pitch - 27;  

  //various velocity changes for fret an strum
  if(I_pitch == 72) action = 3; //strum speed +
  if(I_pitch == 74) action = 4; //strum speed -
  if(I_pitch == 76) action = 5; //slider speed +
  if(I_pitch == 77) action = 6; //slider speed -
}

void switchsOff(int I_action){
  //pick all other notes as knocks
  if(I_action >= 14 && I_action <= 19) digitalWrite(RELAYS[I_action-14], LOW);
  action = 99; //reset key
}

void switchesOn(int I_action){
  switch (I_action) {
  default: 
    //pick all other notes as fret or knocks
    if(I_action >= 14 && I_action <= 19) digitalWrite(RELAYS[I_action-14], HIGH); 
    break;  

  case 0: 
    homeAll();
    if(DEBUG) Serial<<"homeAll"<<endl;
    break; 

  case 1: 

    break;

  case 2:

    break; 

  case 3: 
    stepper1_Speed_change(5);
    break; 

  case 4: 
    stepper1_Speed_change(-5);
    break;

  case 5: 
    stepper2_Speed_change(50);
    break; 

  case 6: 
    stepper2_Speed_change(-50);

    break;   
  } 
  action = 99; //reset key
}

void stepper1_Speed_change(int I_stpspd){
  if(stepper1_Speed > 0) stepper1_Speed = stepper1_Speed + I_stpspd;
  if(DEBUG) Serial<<"current speed 1: "<<stepper1_Speed<<endl;
}

void stepper2_Speed_change(int I_stpspd2){
  if(stepper2_Speed > 0) stepper2_Speed = stepper2_Speed + I_stpspd2;
  if(DEBUG) Serial<<"current speed 2: "<<stepper2_Speed<<endl;
}

void homeAll(){
  stepper1.enableOutputs();
  stepper2.enableOutputs();
  //run steppers until they hits the limit switch   
  homeStepper1();  
  homeStepper2();  
}

void sleepAll(){
  stepper1.disableOutputs();
  stepper2.disableOutputs();
  Serial.println("steppers sleep");
}

void homeStepper1(){
  //run stepper until it hits the limit switch
  //the while is BLOCKING so we cant use it in main loop
  if(DEBUG)Serial.println("home_01");

  stepper1.enableOutputs();
  stepper1.setSpeed(400); //override stepper1 speed, it might be slow
  while(digitalRead(limitPin1b))
    stepper1.runSpeed();

  // prime stepper for next action
  limit_A = 1;
  stepper1.setSpeed(-400);
  stepper1.disableOutputs();
}

void homeStepper2(){
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

  if(DEBUG)Serial.print("steps to home: "); //how many steps did we take to reach home
  if(DEBUG)Serial.println(stepper2.currentPosition());
  stepper2.setCurrentPosition(0);

  //stepper2.disableOutputs();
}



void setup(){
  //// RELAYS //////
  //Relays (solenoids)
  for(int iii=0;iii<=6;iii++){
    pinMode(RELAYS[iii], OUTPUT);
    digitalWrite(RELAYS[iii], LOW);
  }


  /////// stepper 1 ////////
  stepper1.setPinsInverted(14,0,1); //dir, step, enable
  stepper1.setMaxSpeed(stepper1_Speed);
  stepper1.setEnablePin(step1_ENABLE);
  stepper1.disableOutputs();

  pinMode(limitPin1a, INPUT_PULLUP); //arm the pullup for limit pin
  pinMode(limitPin1b, INPUT_PULLUP); //arm the pullup for limit pin

  pinMode(MS1_pin, OUTPUT);
  digitalWrite(MS1_pin, LOW);
  pinMode(MS2_pin, OUTPUT);
  digitalWrite(MS2_pin, LOW);
  pinMode(MS3_pin, OUTPUT);
  digitalWrite(MS3_pin, LOW);

  /////// stepper 2 ////////
  stepper2.setPinsInverted(1,0,1); //dir, step, enable
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setEnablePin(step2_ENABLE);
  stepper2.enableOutputs();

  pinMode(limitPin2a, INPUT_PULLUP); //arm the pullup for limit pin

  ////////////////////////////
  if(DEBUG)    Serial.begin(115200);


  //run steppers until they hits the limit switch   
  homeStepper1();
  homeStepper2();  

  /// MIDI ///
  midiA.setHandleNoteOn(HandleNoteOn);
  midiA.setHandleNoteOff(HandleNoteOff);
  midiA.setHandlePitchBend(handlePitchWheel);
  midiA.begin(MIDI_CHANNEL_OMNI);   
}

void loop(){
  midiA.read();
  stepper1_action(); 
  stepper2.runSpeedToPosition(); //see pitchWheel handle
  if(stepper2.distanceToGo()==0) stepper2.disableOutputs(); 
}




