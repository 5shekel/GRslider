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
int RELAY[6] = {38,40, 42, 44, 46, 48};
//////////////////////////

//STEPPER PROPERTIES
//////// #1 //////////////
int stepper1_Speed = 400;

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

int scalePos[19] = {
  0, 44, 88, 132, 176, 220, 264, 308, 352, 396, 
  440, 484, 528, 572, 616, 660, 704, 748, 792};
///////////////////////////////

///////////////////////////////
// how often you want the serial input to be checked.
int measurementInterval = 50;
// when the serial input was last checked.
long lastMeasurementTime = 0L;

int action; 


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


void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  pitchToAction(pitch);
  if(DEBUG) Serial << "ON: key> " << pitch << "  vel> "<< velocity <<endl;
  switchesOn(action, velocity);
}

void HandleNoteOff(byte channel, byte pitch, byte velocity){
  pitchToAction(pitch);
  if(DEBUG) Serial << "OFF: key> " << pitch << "  vel> "<< velocity <<endl;
  switchsOff(action, velocity);
}

// map the midi key (pitch) to a serialize number 
void pitchToAction(int I_pitch){
  //midiEvent=random(999999); // 
  // it makes it somewhat easier to add diffrent control 
  // like IR or analog, but its also a mess to remmber :/

  if(I_pitch == 83) action = 0; //home
  if(I_pitch==45) action = 1; //strummm

  // mapping solenoids knocks 
  if(I_pitch >= RELAY[0] && I_pitch <= RELAY[5])  action = I_pitch - 22;

  //mapping frets , maping starts at 20
  if(I_pitch >= 47 && I_pitch <= 66)    action = I_pitch - 27;  

  //various velocity changes for fret an strum
  if(I_pitch == 72) action = 3; //strum speed +
  if(I_pitch == 74) action = 4; //strum speed -
  if(I_pitch == 76) action = 5; //slider speed +
  if(I_pitch == 77) action = 6; //slider speed -
  if(I_pitch == 79) action = 7; //slider accel +
  if(I_pitch == 81) action = 8; //slider accel -
}


void switchsOff(int I_action, int I_velocity){
       //pick all other notes as knocks
       if(I_action >= 14 && I_action <= 19) digitalWrite(RELAY[action-14], LOW);

}

void switchesOn(int I_action, int I_velocity){
  //general switch function, this works with both MIDI and IR
  //pick all other notes as fret or knocks
  if(I_action >= 14 && I_action <= 19) digitalWrite(RELAY[action-14], HIGH);
  if(I_action >= 20 && I_action <= 44) goStepper2(I_action);

      switch (I_action) {
          case 0: 
            homeAll();
            if(DEBUG) Serial<<"homeAll"<<endl;
            break; 
          
          case 1: 
            stepper1.enableOutputs();
            stepperMove = 1;
            if(DEBUG) Serial<<"stepper1_action"<<endl;
            break;
          
          case 2:
            break; 
          
          case 3: 
            stepper1_Speed += 50;
            if(DEBUG) Serial<<"current speed 1: "<<stepper1_Speed<<endl;
            break; 
          
          case 4: 
            stepper1_Speed -= 50;
            if(DEBUG) Serial<<"current speed 1: "<<stepper1_Speed<<endl;
            break;
          
          case 5: 
            stepper2_Speed += 100;
            if(DEBUG) Serial<<"current speed 2: "<<stepper2_Speed<<endl;
            break; 
          
          case 6: 
            stepper2_Speed -= 100;
            if(DEBUG) Serial<<"current speed 2: "<<stepper2_Speed<<endl;
            break;   

          case 7: 
            stepper2_accl += 100;
            if(DEBUG) Serial<<"current accel 2: "<<stepper2_accl<<endl;
            break; 
          
          case 8: 
            stepper2_accl -= 100;
            if(DEBUG) Serial<<"current accel 2: "<<stepper2_accl<<endl;
            break; 
  } 
}


void goStepper2(int i_dest){
  //takes the various cntrols for the slider stepper2
  // and starts to moves the stepper
  stepper2.enableOutputs();
  stepper2.setMaxSpeed(stepper2_Speed);
  stepper2.setAcceleration(stepper2_accl);
  //i refer here to two arrays discrbing the same memebers :/
  i_dest = (sizeof(scalePos)/sizeof(int)) - 1;
  stepper2.moveTo(scalePos[i_dest]);
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
  stepper2.disableOutputs();
}



void setup(){
  //// RELAYS //////
    //Relays (solenoids)
    for(int iii=0; iii<=6; iii++){
      pinMode(RELAY[iii], OUTPUT);
      digitalWrite(RELAY[iii], LOW);
    }
  //////////////////////////////


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

  ////////////////////////////
  if(DEBUG)    Serial.begin(115200);


  //run steppers until they hits the limit switch   
  homeStepper1();
  homeStepper2();  

  /// MIDI ///
  midiA.setHandleNoteOn(HandleNoteOn);
  midiA.setHandleNoteOff(HandleNoteOff);
  midiA.begin(MIDI_CHANNEL_OMNI);   
}

void loop(){
  midiA.read();
  stepper1_action(); 
  stepper2.run(); // slider stepper 02
}


//////////////////////////////////////


class UuidClass
{
  // from TrueRandomClass::memfill
  public:
    void memfill(char* location, int size){
        for (;size--;) *location++ = random(1); //randomByte();
    };

    void uuid(uint8_t* uuidLocation)
    {
      // Generate a Version 4 UUID according to RFC4122
      memfill((char*)uuidLocation,16);

      // Although the UUID contains 128 bits, only 122 of those are random.
      // The other 6 bits are fixed, to indicate a version number.
      uuidLocation[6] = 0x40 | (0x0F & uuidLocation[6]); 
      uuidLocation[8] = 0x80 | (0x3F & uuidLocation[8]);
    };
    //or inside an external void UuidClass::uuid(uint8_t* uuidLocation) {..}
}; 


//we are starting a tiny class for this to manage them better
//now there is a possibliteventy of giving a note off in a diffrent event. etc.
class midiEvent
{
  public:
  void gen(uint8_t* event, uint8_t* key, uint8_t* velocity ){};
};