//Final Car Code ESP8266
//12-10-2017

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Servo.h>

//Melee servo motor pins
#define side 1
#define top 3

Servo sideServo;  // create servo object to control a servo
Servo topServo;


//Reversed polarity on Right Drive motor

//Pins for H-bridge connections 
#define motorLeft D0
#define motor_left_direction1 D1
#define motor_left_direction2 D2
#define motorRight D4
#define motor_right_direction1 D5
#define motor_right_direction2 D6
#define sensor A0

//Pin for killSwitch to turn off Teensy
#define killSwitch D3 

//Pins for health bar 
const int data_pin = D8;
const int clock_pin = D7;

//WiFi information
const char* ssid = "Go Diego Go!";//= "Mechatronics"; //"modlab1"; //Go Diego Go!
const char* password = "metateamsix";// = "YayFunFun"; // "ESAP2017";// metateamsix

//Public port is for checking game state broadcast
unsigned int publicPort=2390; 

//Private port is for checking control instructions
unsigned int privatePort = 2001; 

//String from controller
String controlString; 
//String from packet broadcast
String gameStateString; 

//Car control packet
const int control_packet_size = 8;
char controlBuffer[control_packet_size];

//Game state packet
const int game_state_size = 22;
char gameBuffer[game_state_size];

//Separate WifiUDP objects to handle simultaneous packet receiving
WiFiUDP gameStateReceiver;
WiFiUDP controlReceiver; 

//Default Values from controller - these are set such that the initial state is no action
int driveCommand = 43; 
int turnVal = 47;
int sideMeleeCommand = 40; 
int topMeleeCommand = 40; 


//Health values from game state. We only care about these values since we are robot 3
int r3;
int b3; 

//1 if Red, 2 if Blue
int myColor = 2; 
int myHealth = 50; 

void setup(){
  Serial.begin(115200);

  //Setup motor pins
  pinMode(motorLeft,OUTPUT);
  pinMode(motorRight,OUTPUT);
  pinMode(motor_right_direction1, OUTPUT);
  pinMode(motor_right_direction2, OUTPUT);
  pinMode(motor_left_direction1, OUTPUT);
  pinMode(motor_left_direction2, OUTPUT);

  //Set all H-bridge values to Low initially 
  digitalWrite(motorLeft, LOW); 
  digitalWrite(motorRight, LOW); 
  digitalWrite(motor_left_direction1,LOW);
  digitalWrite(motor_left_direction2,LOW);
  digitalWrite(motor_right_direction1,LOW);
  digitalWrite(motor_right_direction2,LOW);

  //Setup for health bar
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);

  //Setup for killswitch 
  pinMode(killSwitch, OUTPUT);
  digitalWrite(killSwitch, LOW);
  
  delay(10); 

  //Setup for melee servos
  pinMode(side, OUTPUT);
  pinMode(top, OUTPUT);
  sideServo.attach(side);  // attaches the servo on pin 9 to the servo object
  delay(10); 
  topServo.attach(top);

  //Wifi Connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(IPAddress(192, 168, 1, 74),IPAddress(192, 168, 1, 1),IPAddress(255, 255, 255, 0));

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
    
  Serial.println("WiFi connected");

  //Start reading on the respective ports
  gameStateReceiver.begin(publicPort);
  controlReceiver.begin(privatePort); 
  
}

void loop(){

  //Check values from controller
  checkController();

  //Translate these to commands
  convertToCommands();

  //Check health values from the game broadcast
  checkGameState();

  //Update health values accordingly
  updateHealth();
  
}

//Check if a packet has been recieved and save this to a local buffer
void checkController() {
  delay(10);
  int c = controlReceiver.parsePacket();
  Serial.println("Control");
  Serial.println(c);
  if(c) {
  controlReceiver.read(controlBuffer, control_packet_size);
  String myData= "";
  for(int i= 0; i < control_packet_size; i++) {
    myData += (char)controlBuffer[i];
   }
  controlString = myData; 
  }
}

//Parse the 5 byte command based on the format from the controller code, and map these to useful motor controls
void convertToCommands(){
  driveCommand = (unsigned int)controlString[0]*10 + (unsigned int)controlString[1];
  turnVal = (unsigned int)controlString[2]*10 + (unsigned int)controlString[3];
  sideMeleeCommand = (unsigned int)controlString[4]*10 + (unsigned int)controlString[5];
  topMeleeCommand = (unsigned int)controlString[6]*10 + (unsigned int)controlString[7];

  controlMotors(driveCommand,turnVal);
  controlMelee(sideMeleeCommand,topMeleeCommand); 

  Serial.println("Control: "); 
  Serial.print(controlString);
  Serial.print("Drive Command: "); 
  Serial.println(driveCommand); 
  Serial.print("Turn Command: "); 
  Serial.println(turnVal); 
  Serial.print("Side Melee: "); 
  Serial.println(sideMeleeCommand); 

}


//Check if a packet has been recieved and save this to a local buffer
void checkGameState() {
  delay(10);
  int c = gameStateReceiver.parsePacket();
  Serial.print("Game State");
  Serial.println(c);
  if(c) {
  gameStateReceiver.read(gameBuffer, game_state_size);
  String myData= "";
  for(int i= 0; i < game_state_size; i++) {
    myData += (char)gameBuffer[i];
   }
  gameStateString = myData; 
  }
}

//Update values based on most recent broadcast
void updateHealth(){
  r3 = ((int)gameStateString[7]-48)*10+ ((int)gameStateString[8]-48); 
  b3 = ((int)gameStateString[18]-48)*10 + ((int)gameStateString[19]-48);

  if(myColor == 1){ //Red
    myHealth = r3;
    //set health status bar
    health_status(r3);
  }
  else if(myColor == 2){ //Blue
    myHealth = b3; 
    //set health status bar
    health_status(b3);
  }
  
  Serial.print("Game State: "); 
  Serial.println(gameStateString); 
  Serial.print("Red Health: "); 
  Serial.println(r3); 
  Serial.print("Blue Health: "); 
  Serial.println(b3);

  //If the game state says we are dead, initiate Teensy kill
  if(myHealth == 0){
    killSequence();
  }

  //Otherwise, don't kill the Teensy
  else digitalWrite(killSwitch, LOW);
  delay(50);
}

//Send 3.3V over the D3 pin, which the Teensy sees as a signal to shut down all LEDs
void killSequence(){
  digitalWrite(killSwitch, HIGH); 
}

//True when the drive motors are supposed to be off
boolean motorStopped = true; 

//Control the driving motors
void controlMotors(int speedCommand, int turnVal){

  //Set direction on H-Bridge
  if (speedCommand < 43){ //backwards
    digitalWrite(motor_left_direction1,LOW);
    digitalWrite(motor_left_direction2,HIGH);
    digitalWrite(motor_right_direction1,HIGH);
    digitalWrite(motor_right_direction2,LOW);
  }
  if(speedCommand >= 43){ //forwards
   digitalWrite(motor_left_direction1,HIGH);
   digitalWrite(motor_left_direction2,LOW);
   digitalWrite(motor_right_direction1,LOW);
   digitalWrite(motor_right_direction2,HIGH);
  }
  
  //Control Rear Motors

//Scale the speed command from the controller to values compatible with the analogWrite command. 
//220 is set to be the lowest value because this is the minimum value at which the motor starts to turn. 
int speedVal = map(speedCommand, 0, 99, 220, 1018);

 //no movement
  if(speedVal > 550 && speedVal <570){
    motorStopped = true;
    analogWrite(motorLeft, 100);
    analogWrite(motorRight, 100);
  }

  //Based on the direction value, re-map this range from the top half so that the full range of speeds is accessed
  if(speedVal>=570){
    motorStopped = false; 
    speedVal = map(speedVal, 560, 1020, 220, 1020);
  }

  //Re-map the range from the bottom so that the full range of speeds is accessed
  if(speedVal<=550){
    motorStopped = false;
    speedVal = map(speedVal, 210, 540, 220, 1020);
    speedVal = 1240 - speedVal;
  }

  //Differential drive. One motor is on, while the other is off.
  
  //Turn Left
  if(turnVal < 45 && !motorStopped){ 
    analogWrite(motorRight,speedVal);
    analogWrite(motorLeft,100);   }
  
  //Turn Right
  if(turnVal > 49 && !motorStopped){
    analogWrite(motorLeft,speedVal);
    analogWrite(motorRight,100); 
   }
  //No turning
  if(turnVal <= 49 && turnVal >= 45){

    if(!motorStopped){
      analogWrite(motorRight,speedVal);
      analogWrite(motorLeft,speedVal);
    }
    
    Serial.print("Drive: ");
    Serial.println(speedVal);
  }
}

//Control the servo motors for melee weapons
void controlMelee(int sideCommand, int topCommand){

  //Checks for minimum movement of joystick, and then maps the values to degrees
  if(sideCommand >= 40){
    topCommand = sideCommand;
    sideCommand = map(sideCommand, 40, 99, 20, 100);
    sideServo.write(sideCommand);
    Serial.println("SIDE");
    Serial.println(sideCommand); 
  }
  
  if(topCommand >= 40){
    topCommand = map(topCommand, 40, 99, 25, 100);
    topCommand = 125 - topCommand;
    topServo.write(topCommand );
    Serial.println("TOP");
    Serial.println(sideCommand - 10); 
  }
  
  delay(10); 
}


//Based on the current health level, lights up a sequential number of bars on the health bar
void health_status(int health){

  Serial.println("MY HEALTH");
  Serial.println(health);
  if(health <= 0){
      shiftOut(data_pin, clock_pin, MSBFIRST, B00000000);
    }
  else if(health <= 20){
      shiftOut(data_pin, clock_pin, MSBFIRST, B00010000);
    }
  else if(health <= 40){
      shiftOut(data_pin, clock_pin, MSBFIRST, B00011000);
    }
  else if(health <= 60){
      shiftOut(data_pin, clock_pin, MSBFIRST, B00011100);
    }
  else if(health <= 80){
      shiftOut(data_pin, clock_pin, MSBFIRST, B00011110);
    }
  else{
      shiftOut(data_pin, clock_pin, MSBFIRST, B00011111);
    }
  delay(1);
  
}







