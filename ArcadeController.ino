// ArcadeController
//
// To get multiple joysticks working in Raspbian/RetroPie, one needs to first remove the usbhid module:
// sudo rmmod usbhid
// Then, we add the module back, with a quirk telling it that our joystick is in fact multiple joysticks
// For an Arduino Leonardo, the command
// sudo modprobe usbhid quirks=0x2341:0x8036:0x40
// OR
// sudo modprobe usbhid quirks=0x2341:0x8037:0x40
// The general command is sudo modprobe usbhid quirks=0xVID:0xPID:0x40
// and you can find your device's VID and PID by running lsusb
// The above is taken from
// http://stackoverflow.com/questions/29358179/usb-possible-to-define-multiple-distinct-hid-joysticks-on-one-interface
//------------------------------------------------------------
#include <Joystick.h>
#include <Mouse.h>
#include <Encoder.h>


//Joystick
const byte JoyCount = 2;
const byte ButtonsJoy0 = 8;
const byte ButtonsJoy1 = 7;
Joystick_ Joystick[JoyCount] = {
  Joystick_(0x03, JOYSTICK_TYPE_GAMEPAD, ButtonsJoy0, 0, true, true, false, false, false, false, false, false, false, false, false),
  Joystick_(0x04, JOYSTICK_TYPE_GAMEPAD, ButtonsJoy1, 0, true, true, false, false, false, false, false, false, false, false, false),
};
//Joystick


//Spinner
Encoder Spinner(0, 1); //Spinner pins, must be interupts pins
long SpinnerPos  = -999;
byte RotaryDIV = 1;
long newPos;
//Spinner


//PinMatrix
enum Types {Button, Joy_Up, Joy_Down, Joy_Left, Joy_Right, Not_Use};

struct Indput {
  Types IndType;
  byte JoyNr;
  byte OutNr;
};

const byte TimetoShift = 3;
const byte Rows = 5;
const byte Cols = 5;
const byte RowPins[Rows] = {10, 16, 2, 3, 4}; //Row pinouts
const byte ColPins[Cols] = {5, 6, 7, 8, 9}; //Col pinouts
byte LastState[Rows][Cols];
byte Pressed;
byte State;

Indput JoySticks[Rows][Cols] = {
  {{Joy_Up, 0, }, {Joy_Down, 0, }, {Joy_Left, 0, }, {Joy_Right, 0, }, {Button, 0, 0}},
  {{Button, 0, 1}, {Button, 0, 2}, {Button, 0, 3}, {Button, 0, 4}, {Button, 0, 5}},
  {{Button, 0, 6}, {Button, 0, 7}, {Joy_Up, 1, }, {Joy_Down, 1, }, {Joy_Left, 1, }},
  {{Joy_Right, 1, }, {Button, 1, 0}, {Button, 1, 1}, {Button, 1, 2}, {Button, 1, 3}},
  {{Button, 1, 4}, {Button, 1, 5}, {Button, 1, 6}, {Not_Use, 1, 7}, {Not_Use, 1, 8}}
};
//PinMatrix




void setup() {
  //  Serial.begin(115200);
  memset(LastState, 0, sizeof(LastState)); //fill LastState with 0
  Mouse.begin(); //initalize mouse
  // Initialize Joystick Library
  for (int index = 0; index < JoyCount; index++)
  {
    Joystick[index].begin();
    Joystick[index].setXAxisRange(-1, 1);
    Joystick[index].setYAxisRange(-1, 1);
  }
  // Initialize Joystick Library
}


void ReadSpinner() {
  newPos = Spinner.read();
  if ((newPos - SpinnerPos < -RotaryDIV) or (newPos - SpinnerPos > RotaryDIV)) {
    Mouse.move(newPos - SpinnerPos, 0, 0);
    SpinnerPos = newPos;
  }
}


void ReadMatrix() {
  // iterate the columns
  for (int colIndex = 0; colIndex < Cols; colIndex++) {
    // col: set to output to low
    byte curCol = ColPins[colIndex];
    pinMode(curCol, OUTPUT);
    digitalWrite(curCol, LOW);
    delay(TimetoShift);
    // row: interate through the rows
    for (int rowIndex = 0; rowIndex < Rows; rowIndex++) {
      byte rowCol = RowPins[rowIndex];
      pinMode(rowCol, INPUT_PULLUP);
      byte State = digitalRead(rowCol);
      if (LastState[rowIndex][colIndex] != State) {
        switch (JoySticks[rowIndex][colIndex].IndType) {
          case Button:
            Joystick[JoySticks[rowIndex][colIndex].JoyNr].setButton(JoySticks[rowIndex][colIndex].OutNr, 1 - State);
            break;
          case Joy_Up:
            Joystick[JoySticks[rowIndex][colIndex].JoyNr].setYAxis(State - 1);
            break;
          case Joy_Down:
            Joystick[JoySticks[rowIndex][colIndex].JoyNr].setYAxis(1 - State);
            break;
          case Joy_Left:
            Joystick[JoySticks[rowIndex][colIndex].JoyNr].setXAxis(State - 1);
            break;
          case Joy_Right:
            Joystick[JoySticks[rowIndex][colIndex].JoyNr].setXAxis(1 - State);
            break;
        }
        pinMode(rowCol, INPUT);
        LastState[rowIndex][colIndex] = State;
      }
    }
    // disable the column
    pinMode(curCol, INPUT);
  }
}





void loop() {
  ReadMatrix();
  ReadSpinner();
}
