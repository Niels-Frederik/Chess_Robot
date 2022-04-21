#include <Servo.h>
// defines pins numbers
const int ns_stepPin = 3; 
const int ns_dirPin = 4; 
const int ew_stepPin = 6; 
const int ew_dirPin = 7;
const int s_switch = 12;
const int n_switch = 13;
//const int e_switch = 12;
const int w_switch = 11;
int ns_steps = 0;
int ew_steps = 0;
int ns_pos = 0;
int ew_pos = 0;
int grabberPin = 5;
Servo grabberServo; 

#define SOP '<'
#define EOP '>'
bool started = false;
bool ended = false;
char inData[80];
byte index;
 
void setup() {
  // Sets the two pins as Outputs
  pinMode(ns_stepPin,OUTPUT); 
  pinMode(ns_dirPin,OUTPUT);
  pinMode(ew_stepPin,OUTPUT); 
  pinMode(ew_dirPin,OUTPUT);
  pinMode(s_switch,INPUT_PULLUP);
  pinMode(n_switch,INPUT_PULLUP);
  pinMode(w_switch,INPUT_PULLUP);
  grabberServo.attach(grabberPin);
  Serial.begin(115200);
  //calibrate();
}

void loop()
{
  // Read all serial data available, as fast as possible
  while(Serial.available() > 0)
  {
    char inChar = Serial.read();
    if(inChar == SOP)
    {
       index = 0;
       inData[index] = '\0';
       started = true;
       ended = false;
    }
    else if(inChar == EOP)
    {
       ended = true;
       break;
    }
    else
    {
      if(index < 79)
      {
        inData[index] = inChar;
        index++;
        inData[index] = '\0';
      }
    }
  }

  // We are here either because all pending serial
  // data has been read OR because an end of
  // packet marker arrived. Which is it?
  if(started && ended)
  {
    // The end of packet marker arrived. Process the packet
    int instructions [3];
    int i = 0;
    char * pch;
    pch = strtok (inData," ");
    while (pch != NULL)
    {
      //printf ("%s\n",pch);
      instructions[i] = atoi(pch);
      pch = strtok (NULL, " ");
      i++;
    }

    execute_instruction(instructions); 

    // Reset for the next packet
    started = false;
    ended = false;
    index = 0;
    inData[index] = '\0';
  }
}

void execute_instruction(int* instruction)
{
  switch(instruction[0])
    {
      case 1:
        Serial.print("Moving");
        break;
      case 2:
        Serial.print("Grabbing");
        break;
      case 3:
        Serial.print("Releasing");
        break;
      case 4:
        Serial.print("Calibrated");
        break;
      case 5:
        Serial.print("Return to zero position");
        break;
      default:
        Serial.print("default");
        break;
    }
}

//======================================== INSTRUCTIONS ===========================================

void calibrate()
{
  while(digitalRead(s_switch) != 0)
  {
    moveSouth(10);
  }
  ns_pos = 0;
  
  while(digitalRead(w_switch) != 0)
  {
    moveWest(10);
  }
  ew_pos = 0;
}

void moveSouth(int steps)
{
  digitalWrite(ns_dirPin,LOW);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ns_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ns_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ns_pos-=steps;
}

void moveNorth(int steps)
{
  digitalWrite(ns_dirPin,HIGH);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ns_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ns_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ns_pos+=steps;
}

void moveEast(int steps)
{
  digitalWrite(ew_dirPin,HIGH);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ew_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ew_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ns_pos+=steps;
}

void moveWest(int steps)
{
  digitalWrite(ew_dirPin,LOW);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ew_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ew_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ew_pos+=steps;
}

void moveTo(int pos_ns, int pos_ew)
{
  int newPos_ns = abs(ns_pos - pos_ns);
  int newPos_ew = abs(ew_pos - pos_ew);
  
  if(ns_pos > pos_ns)
  {
    moveSouth(newPos_ns);
  } else if(ns_pos < pos_ns)
  {
    moveNorth(newPos_ns);
  }
  
  if(ew_pos > pos_ew)
  {
    moveWest(newPos_ew);
  } else if(ew_pos < pos_ew)
  {
    moveEast(newPos_ew);
  }
  ns_pos = pos_ns;
  ew_pos = pos_ew;
}

void lower_grabber()
{
  
}

void raise_grabber()
{
  
}

void close_grabber()
{
  grabberServo.write(0);
}

void open_grabber()
{
  grabberServo.write(80);
}
