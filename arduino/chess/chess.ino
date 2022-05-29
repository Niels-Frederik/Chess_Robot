#include <Servo.h>
// defines pins numbers
const int ns_stepPin = 3; 
const int ns_dirPin = 4;
const int ns_sleepPin = 5;
const int ew_sleepPin = 8;
const int ew_stepPin = 6; 
const int ew_dirPin = 7;
const int s_switch = 2;
const int w_switch = 11;
int ns_steps = 0;
int ew_steps = 0;
int ns_pos = 0;
int ew_pos = 0;
int grabberPin = 10;
Servo grabberServo;
Servo z_axis;
int z_axis_pin = 9;

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
  pinMode(ns_sleepPin,OUTPUT);
  pinMode(ew_sleepPin,OUTPUT);
  pinMode(ew_stepPin,OUTPUT); 
  pinMode(ew_dirPin,OUTPUT);
  pinMode(s_switch,INPUT_PULLUP);
  pinMode(w_switch,INPUT_PULLUP);
  grabberServo.attach(grabberPin);
  z_axis.attach(z_axis_pin);
  Serial.begin(115200);
  raise_grabber();
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
      moveTo(instruction[1], instruction[2]);
      Serial.print("Moved");
      break;
    case 2:
      grab_piece();
      Serial.print("Grabbed");
      break;
    case 3:
      place_piece();
      Serial.print("Placed");
      break;
    case 4:
      calibrate();
      Serial.print("Calibrated");
      break;
    case 5:
      open_grabber();
      lower_grabber();
      //Serial.print("Calibrated");
      break;
    case 6:
      open_grabber();
      //lower_grabber();
      Serial.print("open grabber");
      break;
    case 7:
      close_grabber();
      //lower_grabber();
      Serial.print("close grabber");
      break;
    case 8:
      lower_grabber();
      //lower_grabber();
      Serial.print("close grabber");
      break;
    case 10:
      raise_grabber();
      //lower_grabber();
      Serial.print("close grabber");
      break;
    }
    
}

//======================================== INSTRUCTIONS ===========================================

void calibrate()
{
  digitalWrite(ns_sleepPin, HIGH);
  while(digitalRead(s_switch) != 0)
  {
    digitalWrite(ns_dirPin,LOW);
    for(int i = 0; i < 10; i++) {
      digitalWrite(ns_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ns_stepPin,LOW);
      delayMicroseconds(1000);
    }
  }
  digitalWrite(ns_sleepPin, LOW);
  ns_pos = 0;

  digitalWrite(ew_sleepPin, HIGH);
  while(digitalRead(w_switch) != 0)
  {
    digitalWrite(ew_dirPin,HIGH);
    for(int i = 0; i < 10; i++) {
      digitalWrite(ew_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ew_stepPin,LOW);
      delayMicroseconds(1000);
    }
  }
  digitalWrite(ew_sleepPin, LOW);
  ew_pos = 0;
}

void moveSouth(int steps)
{
  digitalWrite(ns_sleepPin, HIGH);
  digitalWrite(ns_dirPin,LOW);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ns_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ns_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ns_pos-=steps;
  digitalWrite(ns_sleepPin, LOW);
}

void moveNorth(int steps)
{
  digitalWrite(ns_sleepPin, HIGH);
  digitalWrite(ns_dirPin,HIGH);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ns_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ns_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ns_pos+=steps;
  digitalWrite(ns_sleepPin, LOW);
}

void moveWest(int steps)
{
  digitalWrite(ew_sleepPin, HIGH);
  digitalWrite(ew_dirPin,HIGH);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ew_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ew_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ew_pos-=steps;
  digitalWrite(ew_sleepPin, LOW);
}

void moveEast(int steps)
{
  digitalWrite(ew_sleepPin, HIGH);
  digitalWrite(ew_dirPin,LOW);
  for(int i = 0; i < steps; i++) {
      digitalWrite(ew_stepPin,HIGH);
      delayMicroseconds(1000);
      digitalWrite(ew_stepPin,LOW);
      delayMicroseconds(1000);
  }
  ew_pos+=steps;
  digitalWrite(ew_sleepPin, LOW);
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

void grab_piece()
{
  lower_grabber();
  delay(1500);
  close_grabber();
  delay(500);
  raise_grabber();
}

void place_piece()
{
  lower_grabber();
  delay(1500);
  open_grabber();
  delay(500);
  raise_grabber();
}

void lower_grabber()
{
  z_axis.write(180); 
}

void raise_grabber()
{
  z_axis.write(0); 
}

void close_grabber()
{
  grabberServo.write(0);
}

void open_grabber()
{
  grabberServo.write(70);
}
