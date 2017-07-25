#include <string.h>
#define LISTEN 1
#define SEND 0
#define IR_HIGH() TCCR0A = 1<<COM0A0 | 2<<WGM00
#define IR_LOW() TCCR0A = 2<<WGM00;
#define DELAY_ONE_MILLISECOND(); for(long i = 0; i < 194; i++) {__asm__("nop\n\t");}
#define DELAY_ONE_SECOND(); for(long i = 0; i < 145000; i++) {__asm__("nop\n\t");}

//function declarations
void shiftOutIR(char output);
void pushOut(byte data);
void setInterrupt1Sec();
void setInterrupt1Millisec();
void setInterrupt125Microsec();

//GLOBAL VARIABLES 
short tickCounter = -1;
byte data = 0;
int currentBit = 0;
int currentByte = -1;
int Mode = LISTEN;
int redState = LOW;
int greenState = LOW;
char flagBuffer[30] = "temporaryTestFlag";
int flagHash = 4443;
int done = false;
long secondTimer = 0;
long loopTimer = 0;

void setup() {
  cli();
  //Set each pin appropriately
  pinMode(3,OUTPUT); // Red LEDs
  pinMode(4,OUTPUT); // Green LEDs
  pinMode(1,INPUT);  // IR sensor
  pinMode(0,OUTPUT); // IR LED

  //Set up carrier frequency on timer0, which toggles PB0 every 13 clock cycles, or 36kHz ish
  TCCR0B = 1<<CS00;                  //Sets timer0 prescaler to 1. IMPORTANT NOTE: this will change how functions like delay work.
  TCCR0A = 1<<COM0A0 | 2<<WGM00;     //Waveform generation Mode is set to CTC (Clear Timer on Compare match) mode, Compare Output Mode is set to toggle the output when comparison match occurs 
  OCR0A = 13;                        //This sets the frequency to 36kHz
  IR_LOW();                          // OCR0A should be set to ((Clock Freq)/(2*Desired frequency)) - 1
                             
  //For debugging purposes, sets the mode
  if(Mode == SEND) {
    setInterrupt1Sec();
  } else if(Mode == LISTEN) {
    setInterrupt125Microsec();
    memset(flagBuffer, 0, sizeof(flagBuffer));
  }
  
  sei();  //interrupts enabled
}


void loop() {
  if(Mode == LISTEN){
    if(done && hash(flagBuffer) == flagHash) { 
      Mode = SEND;
      setInterrupt1Sec();
    } else {
      readError;
    }
      
    loopTimer++;
    if(loopTimer%5 == 0){
      if (greenState == LOW) {
        greenState = HIGH;
      } else {
        greenState = LOW;
      }
      digitalWrite(4, greenState);
    }
  
  }
  DELAY_ONE_MILLISECOND();

}

//This function is called every 1 second
ISR(TIMER1_COMPB_vect){
  if (redState == LOW) {
    redState = HIGH;
  } else {
    redState = LOW;
  }
  digitalWrite(3, redState);
  
  if(secondTimer%10 == 9){
    sendFlag(flagBuffer);
  }
  if(secondTimer > 60*7){
    Mode = LISTEN;
    setInterrupt125Microsec();
    memset(flagBuffer, 0, sizeof(flagBuffer));
    secondTimer = 0;
    done = false;         //the done flag should only be set when the flag is in memory
  }
  secondTimer++;
}


//Shift IR into result variable. This function is called by an interrupt every 125 microseconds
ISR(TIMER1_COMPA_vect){
  bool pinState = !digitalRead(1);                 //read sensor pin
  if(tickCounter >= 0 || pinState == HIGH){
    tickCounter++;                                  //If sensor pin is LOW and tickCounter is -1, start tickCounter
  }

  if(tickCounter > 8 && ((tickCounter&B00000111) == 3)){       //If (tickCounter)%8 == 3)
    if(currentBit == 8){
      tickCounter = -1;                              //byte has finished transmitting
      currentBit = 0;
      
      if(currentByte >= 0){
        flagBuffer[currentByte] = data;
        currentByte++;
      }
      if(data == '~'){
        currentByte = 0;
        memset(flagBuffer, 0, sizeof(flagBuffer));
        digitalWrite(3,HIGH);
      }
      if(data == '\0'){
        currentByte = -1;                           //string has finished transmitting
        done = true;
        digitalWrite(3,LOW);
      }
      data = 0;
    } else {
      bitWrite(data, currentBit, pinState);         //record current detected pin level
      currentBit++;                                  //increment current bit
    }
  }

}



//Sends one byte out of the IR LED, with one header bit, LSB first, takes about 10 milliseconds per byte.
void shiftOutIR(char output) {
  for(int i = -1; i < 8; i++){
    if(i == -1){
      IR_HIGH();
    }else if(bitRead(output, i) == 1){
      IR_HIGH();
    } else {
      IR_LOW();
    }
    DELAY_ONE_MILLISECOND();
  }
  IR_LOW(); 
  DELAY_ONE_MILLISECOND(); 
}

//Sends one byte out the Green LED, one second per bit. This function is for debugging.
void pushOut(byte data){
    digitalWrite(4,HIGH);
    DELAY_ONE_SECOND();
    for(int databit = 0; databit < 8; databit++){
      digitalWrite(4, bitRead(data,databit));
      DELAY_ONE_SECOND();  
    }
    digitalWrite(4,LOW);
}

void sendFlag(char* flag){
  shiftOutIR('~');
  for(int index = 0; flag[index] != NULL; index++){
    shiftOutIR(flag[index]);
  }
  shiftOutIR('\0');
}

int hash(char* flag){
  long accumulator = 0;
  for(int index = 0; flag[index] != NULL; index++){
    accumulator += flag[index]*(index+1);
  }
  return accumulator%11311;
}

int alt_hash(char* flag){
  int h = 0;
  for(int index = 0; flag[index] != NULL; index++){
    h = 33*h+flag[index];
  }
  return h;
}

//Setup timer1 to trigger an interrupt 1 time per second
void setInterrupt1Sec(){
  TCCR1 = 1<<CTC1 | 13<<CS10;
  OCR1B = 244;
  OCR1C = 244;
  TIMSK = 1<<OCIE1B;
}

//Setup timer1 to trigger an interrupt 1 time per millisecond
void setInterrupt1Millisec(){
  TCCR1 = 1<<CTC1 | 4<<CS10;
  OCR1A = 125;
  OCR1C = 125;
  TIMSK = 1<<OCIE1A;
}

//Setup timer1 to trigger an interrupt 8 times per millisecond
void setInterrupt125Microsec(){
  TCCR1 = 1<<CTC1 | 1<<CS10;
  OCR1A = 125;
  OCR1C = 125;
  TIMSK = 1<<OCIE1A;
}

void readError(){
  memset(flagBuffer, 0, sizeof(flagBuffer));
  currentByte = -1;
  currentBit = 0;
  done = false;
}


