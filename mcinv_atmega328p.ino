/*
Project Code: MCINV
Author: MCH170
Created: 15/1/2025
Version: 1.0
Project Website: https://www.mch170.com/projects/mcinv
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define thr 512   //threshhold for peak detection 1023=5V

//L-VCC P-CHANNEL 11
//N-VCC P-CHANNEL 12
//L-GND N-CHANNEL 10
//N-GND N-CHANNEL 9
//FAN PWM 8  - can be used for communication
//FAN TACH 5 - can be used for communication
//PHASE LOCK A0

#define GF_out 6  //outputs HIGH
#define GF_in 7   //checks if HIGH, if yes, grid forming mode

bool GF=false;
bool phase=1;     //for square wave operation, 1 is positive phase, 0 is negative phase

//GRID TIED
bool active=false;

//GRID FORMING
unsigned long t;
int maxx=-100;
int readd;

void setup() {
  //set parameters for operating mode check
  pinMode(GF_in, INPUT);
  pinMode(GF_out, OUTPUT);
  digitalWrite(GF_out, HIGH);
  delay(10);
  GF=digitalRead(GF_in);//chech if Grid-Following mode
  digitalWrite(GF_out, LOW);
  //SET PIN IO 
  pinMode(9,OUTPUT);//N-GND N-CHANNEL
  pinMode(10,OUTPUT);//L-GND N-CHANNEL
  pinMode(11,OUTPUT);//L-VCC P-CHANNEL
  pinMode(12,OUTPUT);//N-VCC P-CHANNEL
  pinMode(8,OUTPUT);//fan pwm out
  pinMode(5,INPUT);//fan tach in
  pinMode(A0, INPUT);//phase lock input
  //digitalWrite(12, LOW);//makes sure N-VCC is not active so avoid shorts while phase lock reading
  //digitalWrite(9, HIGH);//connects N to GND for phase lock reading
  digitalWrite(8, HIGH);//fan set to 100% pwm
  delay(10);

  
  if(GF){//grid forming mode
    //Checking if there is a preexisting grid
    delay(20*random(10,100));//random delay between 10 and 100 cycles
    t=millis();
    //DETECT IF GRID EXISTS
    while(millis()-t<25){//detect grid peak
      readd=analogRead(A0);
      if(readd>maxx){
        maxx=readd;
      }
    }
    if(maxx>=thr){//GRID DOES EXIST
      do{}while(analogRead(A0)<thr);//wait for positive halfsine
      do{}while(analogRead(A0)>1);//wait for 0
      //in phase with grid at the 180 degree phase
      
    }
    
  }else{//grid tied mode
    do{}while(analogRead(A0)<thr);//WAIT FOR GRID
    do{}while(analogRead(A0)>1);//WAIT FOR PHASE LOCK
    //in phase with grid at the 180 degree phase
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  t=millis();
  do{}while(millis()-t<10);//WAIT FOR PHASE LOCK
  apply();
}

void apply(){
  phase=!phase;
  if(phase){
    digitalWrite(12, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, HIGH);
    digitalWrite(9, HIGH);
  }else{
    digitalWrite(11, LOW);
    digitalWrite(9, LOW);
    digitalWrite(12, HIGH);
    digitalWrite(10, HIGH);
  }
}
