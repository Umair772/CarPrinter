typedef int var;     // handles JavaScript var type

#include <Servo.h>


// setup servo
int servoPin = 3;
int PEN_DOWN = 90; // angle of servo when pen is down
int PEN_UP = 0;   // angle of servo when pen is up
Servo penServo;

//declaring flags for voice commands
bool drawSqr = false;
bool Stop = false;
bool go = false;

//flag to start drawing an image.
bool drawImg = false;

//declaring flags for drawing a pixel or start a new line
//bool sendNextPixelValue = false; //return signal to indicate whether the pixel is drawn.
bool draw = false;
bool nDraw = false;
bool newLine = false;
String input = "";
bool doneS = false;
bool doneSendingData = false;

//Counters to count the number of steps taken in the x and y direction.
int xSteps = 0; //counts the number of steps taken in the x-direction.
int ySteps = 0; //counts the number of steps taken in the y-direction.

//variables for the dimentsions of the paper(mm)
//int width = 126;
//int height = 126;


float wheel_dia=65; //    # mm (increase = spiral out) // changes this value for more precise movement(If the measured distance is too long, increase wheel_dia and do the opposite if too short).
float wheel_base=200; //    # mm (increase = spiral in, ccw) //Change this value for more precise turning(If the robot is turning too sharply (box is rotating clockwise), decrease wheel_base value. Do the opposite if not turning too sharply
int steps_rev=512; //        # 512 for 64x gearbox, 128 for 16x gearbox
int delay_time=1; //         # time between steps in ms

#define DIR_PIN 7
#define DIR_PIN2 4
#define STEP_PIN 6

// Stepper sequence org->pink->blue->yel
int L_stepper_pins[] = {12, 10, 9, 11};
int R_stepper_pins[] = {4, 6, 7, 5};

int fwd_mask[][4] =  {{1, 0, 1, 0},
                      {0, 1, 1, 0},
                      {0, 1, 0, 1},
                      {1, 0, 0, 1}};

int rev_mask[][4] =  {{1, 0, 0, 1},
                      {0, 1, 0, 1},
                      {0, 1, 1, 0},
                      {1, 0, 1, 0}};


void setup() {
  randomSeed(analogRead(1)); 
  Serial.begin(9600);
  drawImg = true;
  for(int pin=0; pin<4; pin++){
    pinMode(L_stepper_pins[pin], OUTPUT);
    digitalWrite(L_stepper_pins[pin], LOW);
    pinMode(R_stepper_pins[pin], OUTPUT);
    digitalWrite(R_stepper_pins[pin], LOW);
  }
  penServo.attach(servoPin);
  //Serial.println("setup");
  penup();
  Serial.println("Begin"); //Pi starts sending the data to print
  Serial.flush();
  
  delay(1000);
}

void loop(){
  //If the voice command for drawing a square is passed.
// if(drawSqr) { //Add commands for the servo.
//   drawSqr = false;
//   drawSquare();
// }
//
//  if(Stop) { //The stepper motors are released when the command for stop is recived.
//    Stop = false;
//    done();
//  }

//For drawing an image, if draw is true we draw a pixel else we move forward by 1mm.
  if(drawImg) {
    drawImg = false;
    drawImage();
  }
        
  done();      // releases stepper motor
  //while(1);
}

//draw image method
void drawImage() {
  while(!doneSendingData) {
      //Locking program in this while loop until data is being sent.
      while (Serial.available()) { //We exit the loop when data is avaliable?
         char readval = Serial.read();
         if (readval != '\n') {
            input += readval;
         }
         if (readval == '\n') {
            doneS = true;
         }
      }
      if (doneS) {
        parse();
        doneS = false;
      }
      //Serial.print(input);
      //parse();
      if (newLine) { 
          ySteps++;
          if (ySteps % 2 == 0) {
      //Calling the manuver method, paramters passed in are the tilting angle, the speed and the pins.
              manuver(20, .02, 2, 1);
              delay(1000);
              rotate(-8,.1);
              delay(1000);
              manuver(-20, .01, 0, 3);
              delay(1000);
           } else if(ySteps % 2 == 1) {
              manuver(-20, .01, 0, 3);
              delay(1000);
              rotate(8,.1);
              delay(1000);
              manuver(20, .02, 2, 1);
              delay(1000);
           }
           Serial.write("d");
      } else {
            if (draw) { 
              //Servo down(place pixel)
              pendown();
              penup();
               delay(1000); //Wait for the srvo to finish.  
               if (ySteps % 2 == 0) {
                  xSteps++;
                  rotate(8,0.25);//Rotate forward by a step.
                  delay(1000);
                  Serial.write("d");
                } else {
                  xSteps--;
                  rotate(-8,0.25);//Rotate backward by a step.
                  delay(1000);
                  Serial.write('d');
            
                }
             } else if(nDraw) {
                if (ySteps % 2 == 0) {
                  xSteps++;
                  rotate(8,0.25);//Rotate forward by a step.
                  delay(1000);
                  Serial.write("d");
                } else {
                  xSteps--;
                  rotate(-8,0.25);//Rotate backward by a step.
                  delay(1000);
                  Serial.write('d');
                 
                }
             }
         }
         //input = "";
    }
}
            
//Parse the data as we recieve it from the pi to perform the tasks.           

void parse() {
  if (input == "draw") {
    draw = true;
    newLine = false;
    nDraw = false;
  } else if(input == "nDraw") {
    nDraw = true;
    draw = false;
    newLine = false;
  }else if(input == "newline") {
    draw = false;
    newLine = true;
    nDraw = false; 
  } else if(input == "done") {
    doneSendingData = true;
  }
  //clear the buffer
  input = "";
  
}


//Rotates a certain angle in the CW or CCW direction depending on which motor is activated.
//Each motor is activated by specifying their pins.
void manuver(float deg, float speed,int DIR,int STEP){ 
  //rotate a specific number of degrees (negitive for reverse movement) 
  //speed is any number from .01 -> 1 with 1 being fastest – Slower is stronger
  int dir = (deg > 0)? HIGH:LOW;
  digitalWrite(R_stepper_pins[DIR], dir);

  int steps = abs(deg)*(1/0.225);
  float usDelay = (1/speed) * 70;

  for(int step=0; step < steps; step++) {
     digitalWrite(R_stepper_pins[STEP], HIGH); 
     delayMicroseconds(usDelay); 
     digitalWrite(R_stepper_pins[STEP], LOW); 
     delayMicroseconds(usDelay); 
  }
}

//rotate a specific number of microsteps (8 microsteps per step) – (negitive for reverse movement)
//speed is any number from .01 -> 1 with 1 being fastest – Slower is stronger
void rotate(int steps, float speed){

  int dir = (steps > 0)? HIGH:LOW;
  steps = abs(steps);

  digitalWrite(DIR_PIN, dir);
  digitalWrite(DIR_PIN2, dir);

  float usDelay = (1/speed) * 70;

  for(int i=0; i < steps; i++){ 
    for(int mask=0; mask<4; mask++){
      for(int pins = 0; pins < 4;pins++) {
        digitalWrite(L_stepper_pins[pins], rev_mask[mask][pins]); 
        delayMicroseconds(usDelay); 
        digitalWrite(R_stepper_pins[pins], fwd_mask[mask][pins]); 
        delayMicroseconds(usDelay);
      }
    }  
  } 
} 

//draw square method
//void drawSquare() {
//  for (int turn = 0; turn < 4; turn++) {
//    for (int line = 0; line < 1; line++) {
//      forward(100);
//    }
//    right(90);
//    
//  }
//}




// ----- HELPER FUNCTIONS -----------
int step(float distance){
  int steps = distance * steps_rev / (wheel_dia * 3.1412); //24.61
  return steps;  
}


void forward(float distance){
  int steps = step(distance);
  //Serial.println(steps);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(L_stepper_pins[pin], rev_mask[mask][pin]);
        digitalWrite(R_stepper_pins[pin], fwd_mask[mask][pin]);
      }
      delay(delay_time);
    } 
  }
}


void backward(float distance){
  int steps = step(distance);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(L_stepper_pins[pin], fwd_mask[mask][pin]);
        digitalWrite(R_stepper_pins[pin], rev_mask[mask][pin]);
      }
      delay(delay_time);
    } 
  }
}


void right(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_base * 3.1412 * rotation;
  int steps = step(distance);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(R_stepper_pins[pin], rev_mask[mask][pin]);
        digitalWrite(L_stepper_pins[pin], rev_mask[mask][pin]);
      }
      delay(delay_time);
    } 
  }   
}


void left(float degrees){
  float rotation = degrees / 360.0;
  float distance = wheel_base * 3.1412 * rotation;
  int steps = step(distance);
  for(int step=0; step<steps; step++){
    for(int mask=0; mask<4; mask++){
      for(int pin=0; pin<4; pin++){
        digitalWrite(R_stepper_pins[pin], fwd_mask[mask][pin]);
        digitalWrite(L_stepper_pins[pin], fwd_mask[mask][pin]);
      }
      delay(delay_time);
    } 
  }   
}


void done(){ // unlock stepper to save battery
  for(int mask=0; mask<4; mask++){
    for(int pin=0; pin<4; pin++){
      digitalWrite(R_stepper_pins[pin], LOW);
      digitalWrite(L_stepper_pins[pin], LOW);
    }
    delay(delay_time);
  }
}


void penup(){
  delay(250);
  // Serial.println("PEN_UP()");
  penServo.write(PEN_UP);
  delay(250);
}


void pendown(){
  delay(250);  
  // Serial.println("PEN_DOWN()");
  penServo.write(PEN_DOWN);
  delay(250);
}


//void circle(float radius){
//  circle(radius, 360);
//}
//
//
//void circle(float radius, float extent){
//  int sides = 1 + int(4 + abs(radius) / 12.0);
//  circle(radius, extent, sides);
//}
//
//
//void circle(float radius, float extent, int sides){
//  // based on Python's Turtle circle implementation
//  float frac = abs(extent) / 360;
//  float w = 1.0 * extent / sides;
//  float w2 = 0.5 * w;
//  float l = 2.0 * radius * sin(w2 * 3.1412 / 180);
//  if (radius < 0){
//    l = -l;
//    w = -w;
//    w2 = -w2;
//  }
//  left(w2);
//  Serial.print("circle: ");
//  Serial.print("frac=");
//  Serial.print(frac);
//  Serial.print(" sides=");
//  Serial.print(sides);
//  Serial.print(" l=");
//  Serial.print(l);
//  Serial.print(" w=");
//  Serial.print(w);
//  Serial.print(" w2=");
//  Serial.println(w2);
//
//  for(int i=0; i<sides; i++){
//    forward(l);
//    left(w);
//  }
//  right(w2);
//}
