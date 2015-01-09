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
#define RELAY0  38                        
#define RELAY1  40                        
#define RELAY2  42                       
#define RELAY3  44                        
#define RELAY4  46
#define RELAY5  48
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



void setup(){
  //// RELAYS //////
    //Relays (solenoids)
  pinMode(RELAY0,OUTPUT);//relay0
  pinMode(RELAY1,OUTPUT);//relay1
  pinMode(RELAY2,OUTPUT);//relay2
  pinMode(RELAY3,OUTPUT);//relay3
  pinMode(RELAY4,OUTPUT);//relay4
  pinMode(RELAY5,OUTPUT);//relay5
  digitalWrite(RELAY0,LOW);//reset relay0
  digitalWrite(RELAY1,LOW);//reset relay1
  digitalWrite(RELAY2,LOW);//reset relay2
  digitalWrite(RELAY3,LOW);//reset relay3
  digitalWrite(RELAY4,LOW);//reset relay4
  digitalWrite(RELAY5,LOW);//reset relay5
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

////////// STEPPER SEQ ///////////

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
//////////////////////////////////////


void HandleNoteOn(byte channel, byte pitch, byte velocity) {

 //midiEvent=random(999999); // 

  // map the midi key (pitch) to a serialize number 
  // it makes it somewhat easier to add diffrent control 
  // like IR or analog, but its also a mess to remmber :/

  if(pitch == 83) event = 0; //home
  if(pitch==45) event = 1; //strummm

  // mapping solenoids knocks 
  if(pitch==36) event = 14;
  if(pitch==37) event = 15;
  if(pitch==38) event = 16;
  if(pitch==39) event = 17;
  if(pitch==40) event = 18;
  if(pitch==41) event = 19;


  //mapping frets , maping starts at 20
  if(pitch >= 47 && pitch <= 66)
    event = pitch - 27;  

//various velocity changes for fret an strum
  if(pitch == 72) event = 3; //strum speed +
  if(pitch == 74) event = 4; //strum speed -
  if(pitch == 76) event = 5; //slider speed +
  if(pitch == 77) event = 6; //slider speed -
  if(pitch == 79) event = 7; //slider accel +
  if(pitch == 81) event = 8; //slider accel -


  if(DEBUG) Serial << "key> " << pitch << "  vel> "<< velocity <<endl;
  switchesOn(event, velocity);
}

void HandleNoteOff(byte channel, byte pitch, byte velocity){
  // this part is DEADLY, needs to ID midi event s better
  // why no unique id for events?
   switchsOff(event, velocity);
}


void switchsOff(int I_event, int I_velocity){
  switch (I_event){
    case 0:
      break;

    default:
       //pick all other notes as knocks
        if(I_event==14) digitalWrite(RELAY0, LOW); //turn off relay0  
        if(I_event==15) digitalWrite(RELAY1, LOW); //turn off relay1
        if(I_event==16) digitalWrite(RELAY2, LOW); //turn off relay2
        if(I_event==17) digitalWrite(RELAY3, LOW); //turn off relay3
        if(I_event==18) digitalWrite(RELAY4, LOW); //turn off relay4
        if(I_event==19) digitalWrite(RELAY5, LOW); //turn off relay5  
      break;
  }
}

void switchesOn(int I_event, int I_velocity){
  //general switch function, this works with both MIDI and IR
      switch (I_event) {
            default: 
          //pick all other notes as fret or knocks
            if(I_event==14) digitalWrite(RELAY0, HIGH); //turn on relay0
            if(I_event==15) digitalWrite(RELAY1, HIGH); //turn on relay1
            if(I_event==16) digitalWrite(RELAY2, HIGH); //turn on relay2
            if(I_event==17) digitalWrite(RELAY3, HIGH); //turn on relay3
            if(I_event==18) digitalWrite(RELAY4, HIGH); //turn on relay4
            if(I_event==19) digitalWrite(RELAY5, HIGH); //turn on relay5 

            if(I_event >= 20 && I_event <= 44){
              goStepper2(I_event);
            }
            break;  

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
  stepper2.moveTo(scalePos[i_dest-20]);
}


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
//now there is a possiblity of giving a note off in a diffrent event. etc.
class midiEvent
{
  public;
    void gen(uint8_t* )
}