#include <string.h>
#define RECEIVE 1
#define SEND 0
#define SEND_TIME 60*7
#define RED 3
#define GREEN 4
#define IR_sense 1
#define IR_LED 0
#define IR_HIGH() TCCR0A = 1<<COM0A0 | 2<<WGM00
#define IR_LOW() TCCR0A = 2<<WGM00;
#define DELAY_ONE_MILLISECOND(); for(long i = 0; i < 194; i++) {__asm__("nop\n\t");}

//function declarations
void shiftOutIR(char output);
void setInterrupt1Sec();
void setInterrupt125Microsec();

//GLOBAL VARIABLES 
int Mode = RECEIVE;  
char flagBuffer[30] = "\0";
unsigned long flagHash = 1616977757;
int done = false;

void setup() {
    cli();
    //Set each pin appropriately
    pinMode(RED,OUTPUT); // Red LEDs
    pinMode(GREEN,OUTPUT); // Green LEDs
    pinMode(IR_sense,INPUT);  // IR sensor
    pinMode(IR_LED,OUTPUT); // IR LED

    //Set up carrier frequency on timer0, which toggles PB0 every 13 clock cycles, or 36kHz ish
    //Sets timer0 prescaler to 1. IMPORTANT NOTE: this will change how functions like delay work.
    TCCR0B = 1<<CS00;                  
    //Waveform generation Mode is set to CTC (Clear Timer on Compare match) mode, 
    //Compare Output Mode is set to toggle the output when comparison match occurs 
    TCCR0A = 1<<COM0A0 | 2<<WGM00;     
    //This sets the frequency to 36kHz
    //OCR0A should be set to ((Clock Freq)/(2*Desired frequency)) - 1
    OCR0A = 13;                        
    IR_LOW();                          

    //sets the mode
    if(Mode == SEND) {
        setInterrupt1Sec();
    } else if(Mode == RECEIVE) {
        setInterrupt125Microsec();
        memset(flagBuffer, 0, sizeof(flagBuffer));
    }

    sei();  //interrupts enabled
}


void loop() {
    static int greenState = LOW;
    static long loopTimer = 0;

    if(Mode == RECEIVE){
        if(done && hash(flagBuffer) == flagHash) { 
            Mode = SEND;
            setInterrupt1Sec();
            digitalWrite(GREEN, LOW);
        } else {
            done = false;
        }

        loopTimer++;
        if(loopTimer%5 == 0){
            if (greenState == LOW) {
                greenState = HIGH;
            } else {
                greenState = LOW;
            }
            digitalWrite(GREEN, greenState);
        }
    }
    DELAY_ONE_MILLISECOND();

}

//This function is called every 1 second
ISR(TIMER1_COMPB_vect){
    static int redState = LOW;
    static long secondTimer = 0;
    if (redState == LOW) {
        redState = HIGH;
    } else {
        redState = LOW;
    }
    digitalWrite(RED, redState);

    if(secondTimer%10 == 9){
        sendFlag(flagBuffer);
    }
    secondTimer++;

    //Stops sending after 7 mins ish
    if(secondTimer > SEND_TIME){
        Mode = RECEIVE;
        digitalWrite(RED, LOW);
        setInterrupt125Microsec();
        memset(flagBuffer, 0, sizeof(flagBuffer));
        //the done flag should only be set when the flag is in memory
        done = false;         
        secondTimer = 0;
    }
}

//Shift IR into buffer. This function is called by an interrupt every 125 microseconds
ISR(TIMER1_COMPA_vect){
    static short tickCounter = -1;
    static byte buffer = 0;
    static int currentBit = 0;
    static int currentByte = -1;

    //read sensor pin
    bool pinState = !digitalRead(IR_sense);

    if(tickCounter >= 0 || pinState == HIGH){
        //If sensor pin is LOW and tickCounter is -1, start tickCounter
        //Otherwise increment tickCounter if it's >= 0
        tickCounter++;          
    }

    //If (tickCounter)%8 == 3)
    if(tickCounter > 8 && ((tickCounter&B00000111) == 3)){       
        if(currentBit == 8){
            //byte has finished transmitting
            //reset in preparation for next byte
            tickCounter = -1;                              
            currentBit = 0;

            if(currentByte >= 0){
                flagBuffer[currentByte] = buffer;
                currentByte++;
            }
            if(buffer == '~'){
                done = false;
                currentByte = 0;
                memset(flagBuffer, 0, sizeof(flagBuffer));
                digitalWrite(RED,HIGH);
            } else if(buffer == '\0'){
                //string has finished transmitting
                currentByte = -1;                           
                done = true;
                digitalWrite(RED,LOW);
            }
            buffer = 0;
        } else {
            //record current detected pin level
            bitWrite(buffer, currentBit, pinState);         
            //increment current bit
            currentBit++;                                  
        }
    }

}



//Sends one byte out of the IR LED, with one header bit, LSB first, takes about 10ms per byte.
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

void sendFlag(char* flag){
    shiftOutIR('~');
    for(int index = 0; flag[index] != NULL; index++){
        shiftOutIR(flag[index]);
    }
    shiftOutIR('\0');
}

unsigned long hash(char* flag){
    digitalWrite(RED,HIGH);
    digitalWrite(GREEN,HIGH);
    unsigned long h = 0;
    for(int index = 0; flag[index] != NULL; index++){
        h = 33*h+flag[index];
    }
    digitalWrite(RED,LOW);
    digitalWrite(GREEN,LOW);
    return h;
}

//Setup timer1 to trigger an interrupt 1 time per second
void setInterrupt1Sec(){
    TCCR1 = 1<<CTC1 | 13<<CS10;
    OCR1B = 244;
    OCR1C = 244;
    TIMSK = 1<<OCIE1B;
}

//Setup timer1 to trigger an interrupt 8 times per millisecond
void setInterrupt125Microsec(){
    TCCR1 = 1<<CTC1 | 1<<CS10;
    OCR1A = 125;
    OCR1C = 125;
    TIMSK = 1<<OCIE1A;
}
