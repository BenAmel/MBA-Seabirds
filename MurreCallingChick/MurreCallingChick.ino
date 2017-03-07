#include "linearTrackActuators.h"
#include "rotaryControl.h"

///////////////////////////////////////////////////
//                  Declarations                 //
///////////////////////////////////////////////////

//declare the actuator objects. The arguments are as follows:

//   actuator ( pulseOutPin, directionControlPin, enablePin, leftLimitSwitch, rightLimitSwitch )

actuator chick(3,5,6,16,15);

// declare the rotary control object. Arguments are as follows

//    rotaryControl( pin number for input ,
//                   number of counts before triggering (default is 10 (5 rotations)),
//                   timeout before resetting counts (default is 1000 ms)){

rotaryControl crank(14,10,1000);

///////////////////////////////////////////////////
//              Configurable options             //
///////////////////////////////////////////////////

// Set the delay time at the end of the jump, specified in milliseconds
int endDelay = 3000;

// Set the delay time at the end of the cycle before reset, specified in milliseconds
int homeDelay = 2000;

// Time that the visitor has to crank before triggering in milliseconds
// If set to any number besides zero, visitor still has to crank
// a minimum of the counts specified above

int minTriggerTime = 3000;

// Movement profile for the chick's hop.
// Default produces a forward motion with 
// slight periodic acceleration and deceleration.
//
// The oscillator period is 1.256 seconds, but
// with the 8 multiplier in the operand of the
// cosine, it has a period of .157 seconds. 
//
//To slow or speed the oscillation, simply add a multiplier 
// to the operand of the trig functions.
// Multiply by >1 to increase the speed, <1 to decrease.
//
// Units are inches per second.

float hopProfile (float osc){
  return sin(osc*5)*3;
  //return sin(osc*2)*2 + sin((osc*2)*3)*8/3; // a slower, more irregular hop
}

float outProfile (float osc){
  return 22;
}

float fallProfile (float osc){
  return 6;
}

///////////////////////////////////////////////////
// You should not need to edit below this point  //
///////////////////////////////////////////////////

int visHall = 14;

// Stores the total length of the chick jump, in inches:
float totalTravel = 6;

// scale factor for inches to steps, measured empirically.
int stepsPerInch = 1000/1.66;

signed int hopSteps (float osc){
  return hopProfile(osc)*stepsPerInch;
}

signed int outSteps (float osc){
  return outProfile(osc)*stepsPerInch;
}

signed int fallSteps (float osc){
  return fallProfile(osc)*stepsPerInch;
}

bool home = true;
bool hopping = false;
bool playing = false;

int playerTrigger = 7;
int playerSignal = 17;

void jumpEnd(){
  // sets how long the motor can run without reaching a stop. Set to 0 to run indefinitely.
  chick.runTimeout = 6000;
  //Serial.println(millis()-tmr,DEC);
  delay(endDelay);
  chick.registerEndCB(wait);
  Serial.println("Return!");
  chick.setSpeedProfile(fallSteps,true);
  chick.run(0);
}

void jump(){
  // sets how long the motor can run without reaching a stop. Set to 0 to run indefinitely.
  chick.runTimeout = 3000;
  hopping = false;
  chick.registerEndCB(jumpEnd);
  chick.setSpeedProfile(outSteps,true);
  Serial.println("Jump!");
  chick.run(1);
}

void hop(){
  chick.setSpeedProfile(hopSteps,true);
  chick.registerEndCB(wait);
  chick.run(1);
}

void startHopping(){
  Serial.println("Start Hopping");

  // sets how long the motor can run without reaching a stop. Set to 0 to run indefinitely.
  chick.runTimeout = 0;
  
  chick.registerEndCB(hop);
  chick.runByCounts(250,1);
  hopping = true;
  home = false;
}

// what happens when the parent returns to center.
void wait(){
  // set the home flag, to let the program know we're home.
  Serial.println("Returned home");
  delay(homeDelay);
  home = true;
}

// function to call when the crank times out 
void resetChick(){
  if((home == false) && hopping){
    Serial.println("Resetting Chick");

    // sets how long the motor can run without reaching a stop. Set to 0 to run indefinitely.
    chick.runTimeout = 3000;
  
    //return home.
    chick.registerEndCB(wait);
    chick.setSpeedProfile(fallSteps,true);
    chick.run(0);
  }
}

void triggerAudio(){
  if(digitalRead(playerSignal)){
    digitalWrite(playerTrigger,HIGH);
    delay(25);
    digitalWrite(playerTrigger,LOW);
    delay(25);
  }
}

// setup function, which is run only once at the program start.
void setup() {
  Serial.begin(9600);

  pinMode(playerTrigger,OUTPUT);
  pinMode(playerSignal,INPUT_PULLUP);
  
  crank.startCB = startHopping;
  crank.triggerCB = jump;
  crank.resetCB = resetChick;
  crank.everyCountCB = triggerAudio;

  crank.triggerTime = minTriggerTime;
  
  delay(5000);
}

// The main loop, that is running constantly.
void loop() {
  chick.idle();

  //if we are at the home position (down) or we're hopping
  if(home || hopping){
    //watch the crank input.
    crank.idle();
  }

}
