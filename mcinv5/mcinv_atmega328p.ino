#include <avr/io.h>
#include <avr/interrupt.h>
#define thr 256

//L-VCC P-CHANNEL 11
//N-VCC P-CHANNEL 12
//L-GND N-CHANNEL 10
//N-GND N-CHANNEL 9
//FAN PWM 8  - can be used for communication
//FAN TACH 5 - can be used for communication
//PHASE LOCK A0

#define GF_out 6  //outputs HIGH
#define GF_in 7   //checks if HIGH, if yes, grid forming mode

int lookUp1[] = {50 ,100 ,151 ,201 ,250 ,300 ,349 ,398 ,446 ,494 ,542 ,589 ,635 ,681 ,726 ,771 ,814 ,857 ,899 ,940 ,981 ,1020 ,1058 ,1095 ,1131 ,1166 ,1200 ,1233 ,1264 ,1294 ,1323 ,1351 ,1377 ,1402 ,1426 ,1448 ,1468 ,1488 ,1505 ,1522 ,1536 ,1550 ,1561 ,1572 ,1580 ,1587 ,1593 ,1597 ,1599 ,1600 ,1599 ,1597 ,1593 ,1587 ,1580 ,1572 ,1561 ,1550 ,1536 ,1522 ,1505 ,1488 ,1468 ,1448 ,1426 ,1402 ,1377 ,1351 ,1323 ,1294 ,1264 ,1233 ,1200 ,1166 ,1131 ,1095 ,1058 ,1020 ,981 ,940 ,899 ,857 ,814 ,771 ,726 ,681 ,635 ,589 ,542 ,494 ,446 ,398 ,349 ,300 ,250 ,201 ,151 ,100 ,50 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0};
int lookUp2[] = {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,50 ,100 ,151 ,201 ,250 ,300 ,349 ,398 ,446 ,494 ,542 ,589 ,635 ,681 ,726 ,771 ,814 ,857 ,899 ,940 ,981 ,1020 ,1058 ,1095 ,1131 ,1166 ,1200 ,1233 ,1264 ,1294 ,1323 ,1351 ,1377 ,1402 ,1426 ,1448 ,1468 ,1488 ,1505 ,1522 ,1536 ,1550 ,1561 ,1572 ,1580 ,1587 ,1593 ,1597 ,1599 ,1600 ,1599 ,1597 ,1593 ,1587 ,1580 ,1572 ,1561 ,1550 ,1536 ,1522 ,1505 ,1488 ,1468 ,1448 ,1426 ,1402 ,1377 ,1351 ,1323 ,1294 ,1264 ,1233 ,1200 ,1166 ,1131 ,1095 ,1058 ,1020 ,981 ,940 ,899 ,857 ,814 ,771 ,726 ,681 ,635 ,589 ,542 ,494 ,446 ,398 ,349 ,300 ,250 ,201 ,151 ,100 ,50 ,0};
int num=99;
bool GF=false;

//GRID TIED
bool active=false;

//NRID FORMING
unsigned long t;
int maxx=-100;
int readd;

void setup() {
  cli();
  //set parameters for operating mode check
  pinMode(GF_in, INPUT);
  pinMode(GF_out, OUTPUT);
  digitalWrite(GF_out, HIGH);
  //SET PIN IO
  DDRB = 0b00000110; // Set PB1 and PB2 as outputs.
  pinMode(11,OUTPUT);//L-VCC P-CHANNEL
  pinMode(12,OUTPUT);//N-VCC P-CHANNEL
  pinMode(8,OUTPUT);//fan pwm out
  pinMode(5,INPUT);//fan tach in
  pinMode(A0, INPUT);//phase lock input
  digitalWrite(12, LOW);//makes sure N-VCC is not active so avoid shorts while phase lock reading
  digitalWrite(9, HIGH);//connects N to GND for phase lock reading
  digitalWrite(8, HIGH);//fan set to 100% pwm
  delay(10);

  TCCR1A = 0b10100010;
  TCCR1B = 0b00011001;
  TIMSK1 = 0b00000001;
  ICR1   = 1600;     // Period for 16MHz crystal, for a switching frequency of 100KHz for 200 subdevisions per 50Hz sin wave cycle.
  //determine operating mode
  GF=digitalRead(GF_in)==HIGH;
  
  if(GF){//grid forming mode
    delay(20*random(1,100));//random delay between 1 and 100 cycles
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
      //in phase with grid at the 180 degree mark
    }
  }else{//grid tied mode
    do{}while(analogRead(A0)<thr && 0);//WAIT FOR GRID
    do{}while(analogRead(A0)>1 && 0);//WAIT FOR PHASE LOCK
  }
  TCNT1 = 0;
  sei();
}

void loop() {
  // put your main code here, to run repeatedly:

}

ISR(TIMER1_OVF_vect){
  static char trig;
  if(!GF && 0){
    if(num==0){
      do{}while(analogRead(A0)>0);
    }
  }
  // change duty-cycle every period.
  OCR1A = lookUp1[num];
  OCR1B = lookUp2[num];
  if(lookUp1[num]>0){
    digitalWrite(12, LOW);
    digitalWrite(11, HIGH);
  }else{
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
  }
    
  if(++num >= 200){ // Pre-increment num then check it's below 200.
    num = 0;       // Reset num.
    trig = trig^0b00000001;
  }
}
