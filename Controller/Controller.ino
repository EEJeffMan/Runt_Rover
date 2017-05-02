/*
 * Title: Controller
 * 
 * Author: EEJeffMan
 * 
 * Description: Receiving control command from joystick, encoding it into a byte and sending it to Xbee via UART (software serial)
 * 
 *    Hardware:
 *        Controller: Arduino pro 328 3.3V
 *        Sparkfun xbee shield
 *        Prallax analog joystick soldered to xbee shield
 *        Xbee series 1
 *        400mAh LiPO battery
 *        
 *    Connections:
 *        Left/right joystick output: A0
 *        Up/down joystick output: A1
 *        Data out to xbee: D3
 *        
 *    NOTE: switch on xbee shield should be in the position "DLINE" instead of "UART".
 *    
 *    NOTE: Originally created 1/14/13
 * 
 */

 #include <SoftwareSerial.h>
 
 #define		LeftTurnThreshold		350//150
 #define		RightTurnThreshold		650//450
 #define		BackThreshold			350//150
 #define		ForwardThreshold		650//450
 #define       Xpin     0//5
 #define       Ypin     1//4
 #define       TXpin    3
 #define       RXpin    2
 
 #define    ForwardOnly    8
 #define    ForwardLeft    9
 #define    ForwardRight   10
 #define    LeftOnly       1
 #define    RightOnly      2
 #define    BackLeft       5
 #define    BackRight      6
 #define    BackOnly       4
 #define    None           0
 
 struct OUTPUT_BITS{
     unsigned Left		:1;
     unsigned Right		:1;
     unsigned Back		:1;
     unsigned Forward	:1;
     unsigned			:4;
 };
union OUTPUTS{
    int All;
    struct OUTPUT_BITS bits;
 };

/*
1 = Left
2= Right
3= N/A
4= Back
5=Back Left
6= Back Right
7= N/A
8= Forward
9 = Forward Left
10 = Forward Right
11 = N/A
*/

union OUTPUTS Outputs;

unsigned int Xnum,Ynum;
unsigned MoveMode = 0;
unsigned PrevMode = 0;

SoftwareSerial mySerial(RXpin, TXpin);

void setup()
{
  mySerial.begin(9600);
  Serial.begin(9600);
}

void loop()
{

  while(1)
  {
    
    delay(50);
    digitalWrite(13, true); // Flash a light to show signs of life
    Xnum = analogRead(Xpin);
    Ynum = analogRead(Ypin);
    
    PrevMode = Outputs.All;
    
    //assumes voltages are low to the left and down on the joystick...
    if(Xnum < LeftTurnThreshold)
    {
        Outputs.bits.Left = 1;
        Outputs.bits.Right = 0;
    }
    else
    {
        Outputs.bits.Left = 0;
    }
    if(Xnum > RightTurnThreshold)
    {
        Outputs.bits.Right = 1;
        Outputs.bits.Left = 0;
    }
    else
    {
        Outputs.bits.Right = 0;
    }
    if(Ynum < BackThreshold)
    {
        Outputs.bits.Back = 1;
        Outputs.bits.Forward = 0;
    }
    else
    {
        Outputs.bits.Back = 0;
    }
    if(Ynum > ForwardThreshold)
    {
        Outputs.bits.Forward = 1;
        Outputs.bits.Back = 0;
    }
    else
    {
        Outputs.bits.Forward = 0;
    }
    
    MoveMode = Outputs.All;
    
    //debugging
    //Serial.print("Output=");
    //Serial.println(Outputs.All);
    
  //  if(!(MoveMode == PrevMode))
    //{
      //Serial.println(Outputs.All);
      mySerial.write(Outputs.All+48);
          Serial.print("X=");
    Serial.println(Xnum);
    Serial.print("Y=");
    Serial.println(Ynum);
      Serial.print("Output=");
      
      switch(Outputs.All)
      {
          case None:
            Serial.println("None");
            break;
          case ForwardOnly:
            Serial.println("Forward");
            break;
          case ForwardLeft:
            Serial.println("Forward Left");
            break;
          case ForwardRight:
            Serial.println("Forward Right");
            break;
          case LeftOnly:
            Serial.println("Left");
            break;
          case RightOnly:
            Serial.println("Right");
            break;
          case BackLeft:
            Serial.println("Back Left");
            break;
          case BackRight:
            Serial.println("Back Right");
            break;
          case BackOnly:
            Serial.println("Back");
            break;
          default:
            Serial.println("Error");
            break;
      }
    //}
	
	//mySerial.write(Outputs.All+48);
    
    digitalWrite(13, false); // Flash a light to show signs of life

  }
}
