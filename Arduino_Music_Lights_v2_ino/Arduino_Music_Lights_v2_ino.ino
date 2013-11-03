//Arduino Music Lights v1
//by paplaukias
//
//uses the FFT library which can be found on http://arduino.cc/forum/index.php/topic,38153.0.html
//
//electronics@paplaukias.co.uk
//http://www.paplaukias.co.uk

#include <fix_fft.h>
//set AnalogPin for audio input
#define AUDIOPIN 0
//set the sensitivity for the bars
#define HIGHEST 5
//set the delay between refreshes for the chaser mode
#define SPEED 50

char im[128], data[128];
char data_avgs[8];
int i=0, val, bass;

//state machine vars
int state = 0;

//light mode indicators
int selectBtn = 2;
int pos3 = 6;
int pos2 = 7;
int pos1 = 8;
int posOff = 9;

//shift register pins
int latchPin = 3;
int clockPin = 4;
int dataPin = 5;

//button debounce variables
/*int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 50;
int buttonState;*/

//explicitly set the LED outputs
int level[9] = {0b00000000, 0b00000001, 0b00000011, 0b00000111, 0b00001111, 0b00011111, 0b00111111, 0b01111111, 0b11111111};
int chaser[9] = {0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};

void setup(){
  //buttons and LED to control the light modes
  pinMode(selectBtn, INPUT);
  pinMode(pos3, OUTPUT);
  pinMode(pos2, OUTPUT);
  pinMode(pos1, OUTPUT);
  pinMode(posOff, OUTPUT);

  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
}

void loop(){

//main state machine
switch(state){

//off mode
case 0:
  shiftOut(dataPin, clockPin, MSBFIRST, chaser[0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  digitalWrite(posOff, HIGH);
  digitalWrite(pos1, LOW);
  digitalWrite(pos2, LOW);
  digitalWrite(pos3, LOW);
  
  while(!digitalRead(selectBtn)){
    
    state = 0; 
  }
  delay(500);
  state = 1;
break;

//LED chaser mode
case 1:
  digitalWrite(posOff, LOW);
  digitalWrite(pos1, HIGH);
  digitalWrite(pos2, LOW);
  digitalWrite(pos3, LOW);
  
  while(!digitalRead(selectBtn)){
    for(i=0; i<8; i++){
      shiftOut(dataPin, clockPin, MSBFIRST, chaser[i]);
      digitalWrite(latchPin, HIGH);
      digitalWrite(latchPin, LOW);
      delay(SPEED);
      if(digitalRead(selectBtn))
        break;
    }
    
    for(i=8; i>0; i--){
      shiftOut(dataPin, clockPin, MSBFIRST, chaser[i]);
      digitalWrite(latchPin, HIGH);
      digitalWrite(latchPin, LOW);
      delay(SPEED);
      if(digitalRead(selectBtn))
        break;
    }
    state = 1; 
  }
  shiftOut(dataPin, clockPin, MSBFIRST, chaser[0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  delay(500);
  state = 2;
break;

//music sync mode
case 2:
  digitalWrite(posOff, LOW);
  digitalWrite(pos1, LOW);
  digitalWrite(pos2, HIGH);
  digitalWrite(pos3, LOW);

  while(!digitalRead(selectBtn)){
    //read data from the audio input and store it in an array
    for (i=0; i < 128; i++){                                   
      val = analogRead(AUDIOPIN);                                  
      data[i] = val;                                     
      im[i] = 0;                                                   
    };
    
    //use the FFT to convert audio data from
    //a voltage domain to a frequency domain
    fix_fft(data, im, 7, 0);

    for (i=0; i<64; i++){                                    
      data[i] = sqrt(data[i] * data[i] + im[i] * im[i]);  // this gets the absolute value of the values in the
                                                          //array, so we're only dealing with positive numbers
    };   

    //average a couple of frequencies together
    data_avgs[0] = data[0] + data[1];   //average together
    data_avgs[0] = map(data_avgs[0], 0, HIGHEST, 0, 8); //remap values for LED bars
    
    //check that the value doesn't exceed 8
    while(data_avgs[0] > 8){
     data_avgs[0] = data_avgs[0] - 8; 
    }
    
    int bass = data_avgs[0];
    Serial.println(data_avgs[0], DEC);
    
    //send the data you want to display to the shift register
    shiftOut(dataPin, clockPin, MSBFIRST, level[bass]);
    //output the data written to the shift register
    digitalWrite(latchPin, HIGH);
    digitalWrite(latchPin, LOW);
    delay(15);
    state = 2;   
  }
  shiftOut(dataPin, clockPin, MSBFIRST, chaser[0]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  delay(500);
  state = 0;
break;
}
}
