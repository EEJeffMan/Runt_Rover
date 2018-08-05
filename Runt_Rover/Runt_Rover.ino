/*
 * Title: Runt_Rover
 * 
 * Author: EEJeffMan
 * 
 * Description: Receives joystick direction data from xbee, and drives runt rover motors.
 * 
 *    Hardware:
 *        Controller: Arduino pro mini 3.3V
 *        Receiver: Xbee series 1 w/xbee explorer regulated
 *        Motor driver: DROK DC-DC w/dual L298N Motor Driver Module
 *        
 *    Motor Driver Module output connections to motors:
 *        OUT1 = back left blk
 *        OUT2 = back left red
 *        OUT3 = front left blk
 *        OUT4 = front left red
 *        OUT5 = front right blk
 *        OUT6 = front right red
 *        OUT7 = rear right blk
 *        OUT8 = rear right red
 *        
 *    Arduino I/O:
 *        D2-9: Digital outputs to IN1-8, Motor Driver Module
 *        D10: SW serial RX from xbee explorer, 9600 baud 8N1
 *        
 *    Flow:
 *        Read xbee data
 *        Determine desired motor drive
 *        Update drive to motors
 * 
 */

#include <SoftwareSerial.h>

#define XBEE_COMMAND_STOP           0
#define XBEE_COMMAND_FORWARD        8
#define XBEE_COMMAND_FORWARD_LEFT   9
#define XBEE_COMMAND_FORWARD_RIGHT  10
#define XBEE_COMMAND_REVERSE        4
#define XBEE_COMMAND_REVERSE_LEFT   5
#define XBEE_COMMAND_REVERSE_RIGHT  6
#define XBEE_COMMAND_TURN_LEFT      1
#define XBEE_COMMAND_TURN_RIGHT     2

#define FULL_SPEED                  255
#define TURN_SPEED                  50

#define LEFT_MOTORS                 0
#define RIGHT_MOTORS                1

#define MOTOR_OFF                   11
#define MOTOR_FORWARD_FULL          22
#define MOTOR_FORWARD_TURN          33
#define MOTOR_REVERSE_FULL          44
#define MOTOR_REVERSE_TURN          55
#define MOTOR_RAMP_STEP             5

#define BUTTON_MASK                 0x10
#define JOYSTICK_MASK               0x0F

SoftwareSerial xbee(10,A4);   //RX, TX

/*
 * array 0 = left, array 1 = right
 * 
 * PWM can only be on pins 3, 5, 6, 9, and 10, use all but 10 for PWM:
 * 2 = front left a
 * 3 = front left en
 * 4 = front right a
 * 5 = front right en
 * 6 = rear right en
 * 7 = rear right a 
 * 8 = rear left a
 * 9 = rear left en
 * A0 = front left b
 * A1 = front right b
 * A2 = rear right b
 * A3 = rear left b
 */
unsigned int front_motor_a[2] = {2, 4};
unsigned int front_motor_b[2] = {A0, A1};
unsigned int rear_motor_a[2] = {8, 7};
unsigned int rear_motor_b[2] = {A3, A2};
unsigned int front_motor_en[2] = {3, 5};
unsigned int rear_motor_en[2] = {9, 6};

unsigned int motor_state[2];
int motor_drive_target[2];
int motor_drive_present[2];

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  // xbee data input
  xbee.begin(9600);

  // motor drive outputs
  pinMode(front_motor_a[LEFT_MOTORS], OUTPUT);
  pinMode(front_motor_a[RIGHT_MOTORS], OUTPUT);
  pinMode(front_motor_b[LEFT_MOTORS], OUTPUT);
  pinMode(front_motor_b[RIGHT_MOTORS], OUTPUT);
  pinMode(rear_motor_a[LEFT_MOTORS], OUTPUT);
  pinMode(rear_motor_a[RIGHT_MOTORS], OUTPUT);
  pinMode(rear_motor_b[LEFT_MOTORS], OUTPUT);
  pinMode(rear_motor_b[RIGHT_MOTORS], OUTPUT);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.println("Begin.");
}

void loop() {
  // put your main code here, to run repeatedly:
    unsigned int i;
    char xbee_command;

    delay(50);    // TODO: Replace this with something more deterministic, perhaps based on a timer.

    // read xbee data and update motor states
    xbee_command = xbee.read();
    Serial.print("Command = ");
    Serial.println(xbee_command, DEC);

    //debugging
    //xbee_command = XBEE_COMMAND_FORWARD;
    if(xbee_command == -1)
    {
      Serial.println("No data received");
      xbee_command = XBEE_COMMAND_STOP;
    }
    else
    {
      xbee_command -= 48;
    }

    if(xbee_command & BUTTON_MASK)
    {
      Serial.println("Button Press.");
      digitalWrite(13, HIGH);
    }
    else
    {
      digitalWrite(13, LOW);
    }
    
    switch(xbee_command & JOYSTICK_MASK)
    {
      case XBEE_COMMAND_STOP:
        // stop position: all off
        motor_state[LEFT_MOTORS] = MOTOR_OFF;
        motor_state[RIGHT_MOTORS] = MOTOR_OFF;
        Serial.println("xbee command stop");
      break;

      case XBEE_COMMAND_FORWARD:
        // forward position: all forward full speed
        motor_state[LEFT_MOTORS] = MOTOR_FORWARD_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_FORWARD_FULL;
        Serial.println("xbee command forward");
      break;

      case XBEE_COMMAND_FORWARD_LEFT:
        // forward left: left motors forward turn speed, right motors forward full speed
        motor_state[LEFT_MOTORS] = MOTOR_FORWARD_TURN;
        motor_state[RIGHT_MOTORS] = MOTOR_FORWARD_FULL;
        Serial.println("xbee command forward left");
      break;
      
      case XBEE_COMMAND_FORWARD_RIGHT:
        // forward right: left motors forward full speed, right motors forward turn speed
        motor_state[LEFT_MOTORS] = MOTOR_FORWARD_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_FORWARD_TURN;
        Serial.println("xbee command forward right");
      break;
      
      case XBEE_COMMAND_REVERSE:
        // reverse position: all reverse full speed
        motor_state[LEFT_MOTORS] = MOTOR_REVERSE_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_REVERSE_FULL;
        Serial.println("xbee command reverse");
      break;
      
      case XBEE_COMMAND_REVERSE_LEFT:
        // reverse left: left motors reverse turn speed, right motors reverse full speed
        motor_state[LEFT_MOTORS] = MOTOR_REVERSE_TURN;
        motor_state[RIGHT_MOTORS] = MOTOR_REVERSE_FULL;
        Serial.println("xbee command reverse left");
      break;
      
      case XBEE_COMMAND_REVERSE_RIGHT:
        // reverse right: left motors reverse full speed, right motors reverse turn speed
        motor_state[LEFT_MOTORS] = MOTOR_REVERSE_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_REVERSE_TURN;
        Serial.println("xbee command reverse right");
      break;
      
      case XBEE_COMMAND_TURN_LEFT:
        // turn left position: left motors reverse full speed, right motors forward full speed
        motor_state[LEFT_MOTORS] = MOTOR_REVERSE_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_FORWARD_FULL;
        Serial.println("xbee command turn left");
      break;
      
      case XBEE_COMMAND_TURN_RIGHT:
        // turn right position: left motors forward full speed, right motors reverse full speed
        motor_state[LEFT_MOTORS] = MOTOR_FORWARD_FULL;
        motor_state[RIGHT_MOTORS] = MOTOR_REVERSE_FULL;
        Serial.println("xbee command turn right");
      break;

      default:
        // default is stop: all off
        motor_state[LEFT_MOTORS] = MOTOR_OFF;
        motor_state[RIGHT_MOTORS] = MOTOR_OFF;
        Serial.println("Xbee command error");
      break;
    }

    // read motor states and drive motors    
    for(i = 0; i < 2; i++)
    {
      if (abs(motor_drive_present[i] - motor_drive_target[i]) < 7)
      {
        // if close to target, set to target
        motor_drive_present[i] = motor_drive_target[i];
      }
      else
      {
        // move PWM closer to target
        if (motor_drive_present[i] > motor_drive_target[i])
        {
          // PWM is greater than target, decrease PWM
          motor_drive_present[i] -= MOTOR_RAMP_STEP;
        }
        else
        {
          // PWM is less than target, increase PWM
          motor_drive_present[i] += MOTOR_RAMP_STEP;    
        }
      }
      
      switch(motor_state[i])
      {
        // drive motors: forward: a = high, b = low; reverse: a = low, b = high; off: both low
        case MOTOR_OFF:
          digitalWrite(front_motor_a[i], LOW);
          digitalWrite(front_motor_b[i], LOW);
          digitalWrite(rear_motor_a[i], LOW);
          digitalWrite(rear_motor_b[i], LOW);
          analogWrite(front_motor_en[i], 0);
          analogWrite(rear_motor_en[i], 0);
          motor_drive_present[i] = 0;
        break;

        case MOTOR_FORWARD_FULL:
          digitalWrite(front_motor_a[i], HIGH);
          digitalWrite(front_motor_b[i], LOW);
          digitalWrite(rear_motor_a[i], HIGH);
          digitalWrite(rear_motor_b[i], LOW);
          //analogWrite(front_motor_en[i], FULL_SPEED);
          //analogWrite(rear_motor_en[i], FULL_SPEED);
          analogWrite(front_motor_en[i], motor_drive_present[i]);
          analogWrite(rear_motor_en[i], motor_drive_present[i]);
          motor_drive_target[i] = FULL_SPEED;
        break;

        case MOTOR_FORWARD_TURN:
          digitalWrite(front_motor_a[i], HIGH);
          digitalWrite(front_motor_b[i], LOW);
          digitalWrite(rear_motor_a[i], HIGH);
          digitalWrite(rear_motor_b[i], LOW);
          analogWrite(front_motor_en[i], TURN_SPEED);
          analogWrite(rear_motor_en[i], TURN_SPEED);
          motor_drive_target[i] = TURN_SPEED;
        break;

        case MOTOR_REVERSE_FULL:
          digitalWrite(front_motor_a[i], LOW);
          digitalWrite(front_motor_b[i], HIGH);
          digitalWrite(rear_motor_a[i], LOW);
          digitalWrite(rear_motor_b[i], HIGH);
          //analogWrite(front_motor_en[i], FULL_SPEED);
          //analogWrite(rear_motor_en[i], FULL_SPEED);
          analogWrite(front_motor_en[i], motor_drive_present[i]);
          analogWrite(rear_motor_en[i], motor_drive_present[i]);          
          motor_drive_target[i] = FULL_SPEED;
        break;

        case MOTOR_REVERSE_TURN:
          digitalWrite(front_motor_a[i], LOW);
          digitalWrite(front_motor_b[i], HIGH);
          digitalWrite(rear_motor_a[i], LOW);
          digitalWrite(rear_motor_b[i], HIGH);
          analogWrite(front_motor_en[i], TURN_SPEED);
          analogWrite(rear_motor_en[i], TURN_SPEED);
          motor_drive_target[i] = TURN_SPEED;
        break;

        default:
          digitalWrite(front_motor_a[i], LOW);
          digitalWrite(front_motor_b[i], LOW);
          digitalWrite(rear_motor_a[i], LOW);
          digitalWrite(rear_motor_b[i], LOW);
          analogWrite(front_motor_en[i], 0);
          analogWrite(rear_motor_en[i], 0);
          motor_drive_target[i] = 0;
        break;
      }
    }
}
