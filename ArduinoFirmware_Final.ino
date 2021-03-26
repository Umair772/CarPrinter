




typedef int var;     // handles JavaScript var type

#include <Servo.h>


// setup servo
int servoPin = 8;
int PEN_DOWN = 175; // angle of servo when pen is down
int PEN_UP = 90;   // angle of servo when pen is up
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


float wheel_dia=69; //    # mm (increase = spiral out) // changes this value for more precise movement(If the measured distance is too long, increase wheel_dia and do the opposite if too short).
float wheel_base=110; //    # mm (increase = spiral in, ccw) //Change this value for more precise turning(If the robot is turning too sharply (box is rotating clockwise), decrease wheel_base value. Do the opposite if not turning too sharply
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
  Serial.println("Begin"); //Pi starts sending the data to print
  Serial.flush();
  
  delay(1000);
}

void loop(){

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
          done();
          delay(10000);
          Serial.write("d");
      } else {
            if (draw) { 
              //Servo down(place pixel)
                pendown();
                penup();
                delay(500); 
                  xSteps++;
                  rotate(8,0.25);//Rotate forward by a step.
                  delay(200);
                  Serial.write("d");
             } else if(nDraw) {
                  xSteps++;
                  rotate(8,0.25);//Rotate forward by a step.
                  delay(200);
                  Serial.write("d");
             } 
             
        }
    }
 }

            
//Parse the data as we recieve it from the pi to perform the tasks.           
void parse() {
  if (input == "n") {
    draw = false;
    newLine = true;
    nDraw = false;
  } else if(input == "nDraw") {
      nDraw = true;
      draw = false;
      newLine = false;
  }else if(input == "draw") {
    draw = true;
    newLine = false;
    nDraw = false; 
  }else if(input == "done") {
    doneSendingData = true;
  }
  //clear the buffer
  input = "";
  
}


//rotate a specific number of microsteps (8 microsteps per step) – (negitive for reverse movement)
//speed is any number from .01 -> 1 with 1 being fastest – Slower is stronger
void rotate(int steps, float speed){
  int dir = (steps > 0)? HIGH:LOW;
  digitalWrite(DIR_PIN, dir);
  digitalWrite(DIR_PIN2, dir);
  float usDelay = (1/speed) * 70;
  if (steps > 0) {
    steps = abs(steps);
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
  } else {
    steps = abs(steps);
    for(int i=0; i < steps; i++){ 
      for(int mask=0; mask<4; mask++){
        for(int pins = 0; pins < 4;pins++) {
          digitalWrite(R_stepper_pins[pins], fwd_mask[mask][pins]); 
          delayMicroseconds(usDelay); 
          digitalWrite(R_stepper_pins[pins], rev_mask[mask][pins]); 
          delayMicroseconds(usDelay);
        }
      }
     
    } 
  }
}

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
  //Serial.println("PEN_UP()");
  penServo.write(PEN_UP);
  delay(250);
}


void pendown(){
  delay(250);  
  //Serial.println("PEN_DOWN()");
  penServo.write(PEN_DOWN);
  delay(250);
}

