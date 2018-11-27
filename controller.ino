//Final controller code
//12-15-17

#include<ESP8266WiFi.h>
#include<WiFiUDP.h>

#define sensor A0
#define driveStick D5
#define turnStick D6
#define sideSlapStick D0
#define topSlapStick D1

//WIFI Setup
const char* ssid; 
const char* password; 

const int send_packetSize = 8;  

char sendBuffer[send_packetSize];

IPAddress ipSendto(192, 168, 1, 74); //Specify the receiver's IP Addres 

unsigned int udpRemotePort=2001;

WiFiUDP udpSender;

//Control variables
int driveCommand; 
int turnCommand;
int sideMeleeCommand; 
int topMeleeCommand; 
int sensorCounter; 


int fields[4] = {0,0,0,0};

int packetValue[8];

void setup(){
  Serial.begin(115200);  
  ssid = "Go Diego Go!"; //"BlueTeam";
  password = "metateamsix";

  //ADC pin
  pinMode(sensor,INPUT);

  //Joysticks
  pinMode(driveStick,OUTPUT); 
  pinMode(turnStick,OUTPUT);
  pinMode(sideSlapStick,OUTPUT);
  pinMode(topSlapStick,OUTPUT);

  //Wifi
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  WiFi.config(IPAddress(192, 168, 1, 47),IPAddress(192, 168, 1, 1),IPAddress(255, 255, 255, 0));

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
    
   Serial.println("WiFi connected");
   udpSender.begin(udpRemotePort);
}

void loop() {
  readSensors(); 
  sendPacket();
  delay(50);
}

void readSensors(){  //read drive joystick
    digitalWrite(driveStick,HIGH);
    driveCommand = map(analogRead(sensor),0,859,0,99);
    digitalWrite(driveStick,LOW);

  // read the turning joystick
    digitalWrite(turnStick,HIGH);
  //  Serial.println("Raw value of turn");
   // Serial.println(analogRead(sensor)); 
    turnCommand = map(analogRead(sensor),0,850,0,99);
    digitalWrite(turnStick,LOW);

 // read the side melee joystick
    digitalWrite(sideSlapStick,HIGH);
    sideMeleeCommand = map(analogRead(sensor),34,855,0,99);
    digitalWrite(sideSlapStick,LOW);

 //read the top melee joystick 
    digitalWrite(topSlapStick,HIGH);
    topMeleeCommand = map(analogRead(sensor),35,859,0,99);
    digitalWrite(topSlapStick,LOW);

  //create a packet value by adding up the values from speed command, turning command, and melee commands

  //Each command is placed into the array of commands
  fields[0] = driveCommand;
  fields[1] = turnCommand;
  fields[2] = sideMeleeCommand;
  fields[3] = topMeleeCommand;


  //For each field, a packet field is added which ensures all values 0 - 99 are split into two-digit char arrays
  int j; 
  for(int i = 0; i < 4; i++){
    
     j = i*2; 
     
    //Turns values like '9' into '09' so that the packet format is not disrupted
     if (fields[i] < 10){
      packetValue[j]  = 0; 
      packetValue[j+1] = fields[i];
     }
     else{
      packetValue[j] = fields[i]/10;
      packetValue[j+1] = fields[i]%10;
     }
  } 
}

//Send packet 5 times
void sendPacket(){
for(int i = 0; i < 5; i++){
    sprintf(sendBuffer,"%c%c%c%c%c%c%c%c",(char)packetValue[0],(char)packetValue[1],(char)packetValue[2],(char)packetValue[3],(char)packetValue[4],(char)packetValue[5],(char)packetValue[6],(char)packetValue[7]);               
    udpSender.beginPacket(ipSendto, udpRemotePort);
    udpSender.write(sendBuffer, sizeof(sendBuffer));
    udpSender.endPacket();
    
    int speedVal = map(driveCommand, 1, 99, 220, 1018);
    Serial.println("Initial speedVal");
    Serial.println(speedVal);
    Serial.println("drive:");
    Serial.println(driveCommand);
    Serial.println("turn:");
    Serial.println(turnCommand);
    Serial.println("side melee:");
    Serial.println(sideMeleeCommand);
    Serial.println("top melee:");
    Serial.println(topMeleeCommand);
    delay(50);
}
}

