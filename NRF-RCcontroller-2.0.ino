/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 *  Author: Brendan Doherty (2bndy5)
 */

#include<SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Wire.h>

#define JOYSTICK_ADDR 0x40
#define CE_PIN 9
#define CSN_PIN 10
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "pm117", "pc117"};
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
uint8_t radioNumber = 1;  // setting to 2ctrl role

String bind = address[radioNumber]; // setting up our bind address

// Used to control whether this node is sending or receiving
bool role = true;  // true = TX role, false = RX role

int payload[] = { 100, 9, 9 };

int switchPin = 5;
unsigned long buttonPressStart = 0; // Cruise Control Start Time
const unsigned long holdTime = 5000; // 5 seconds in milliseconds
bool inCruiseMode = false;
int cruiseLED = 2;

int ForwardBackward;
int ForwardBackwardLOCK;
int RightLeft;
int RightLeftLOCK;
int switchPos;

int LeftMotorSpeed = 1500;
int RightMotorSpeed = 1500;

int TransistorBasePin = 3;


void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  Wire.begin();

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }
  // To set the radioNumber via the Serial monitor on startup
  // Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  // while (!Serial.available()) {
  //   // wait for user input
  // }
  // char input = Serial.parseInt();
  // radioNumber = input == 1;
  // Serial.print(F("radioNumber = "));
  // Serial.println((int)radioNumber);

  // role variable is hardcoded to RX behavior, inform the user of this
  // Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[0]);  // using pipe 1
  // in the future adjust the above for another ReadingPipe from the RC
  
  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data
  digitalWrite(switchPin, LOW);  // set LOW to detect HIGH signal from 
}  // setup

void loop() {
  delay(500);

  Wire.beginTransmission(JOYSTICK_ADDR);
  Wire.write(0x00);  // Register to start reading from
  Wire.endTransmission(false);

  Wire.requestFrom(JOYSTICK_ADDR, 2);  // Request 2 bytes (X and Y axis data)

  int inputRead = digitalRead(5);

  int PWM[3];
  bool report;

  switchPos = digitalRead(switchPin);
   if (!inCruiseMode && switchPos) {
        // If button press just started, record the start time
        if (buttonPressStart == 0) {
            buttonPressStart = millis();
        }
        // Check if button is held for more than 5 seconds
        if (millis() - buttonPressStart >= holdTime && !inCruiseMode) {
            inCruiseMode = true; // Activate cruise mode
            digitalWrite(cruiseLED, HIGH); // Turn on cruise mode LED
            Serial.println("Cruise mode activated!");
            while (switchPos) {

              if (Wire.available() == 2) {
                  byte xData = Wire.read();  // Read X axis data
                  byte yData = Wire.read();  // Read Y axis data
                  //  X-axis (North-South)
                  if (xData <= 80) {  // East side
                      if (yData <= 80) {  // North-East 
                        LeftMotorSpeed = 2000; // set to static full throttle on left motor
                        RightMotorSpeed = map(xData, 0, 80, 2000, 1000);  // Map right motor speed 0-80 to 2000-1000
                      } else if (yData >= 175) {  // South-East side
                        LeftMotorSpeed = map(yData, 255, 175, 2000, 1000);  // Map left motor speed 255-175 to 2000-1000
                        RightMotorSpeed = 1000; // set to static full reverse on right motor
                      } 
                  } else if (xData >= 175) {  // West side
                      if (yData <= 80) {  // North-West side
                        RightMotorSpeed = 2000; // set to static full throttle on right motor
                        LeftMotorSpeed = map(yData, 0, 80, 1000, 2000);  // Map left motor speed 0-80 to 1000-2000
                      } else if (yData >= 176) {  // South-West side
                        RightMotorSpeed = map(xData, 255, 176, 1000, 2000);  // Map right motor speed 255-176 to 1000-2000
                        LeftMotorSpeed = 1000;  // set to static full reverse on left motor
                      }
                  }
                PWM[0] = RightMotorSpeed
                PWM[1] = LeftMotorSpeed
            }
              // blink the button led on
          }          
        }
    } else {
      buttonPressStart = 0;
    }
    // if user still happens to be holding button past 5 sec (which will happen) we need to be able to 
    // let go of joystick so it goes to a neutral position, but still stay at the LOCKED speed and rightLeft
    // perhaps we implement a timer that will start counting down to let the user have time to go back to neutral joystick position
    // or maybe the user is told to let go of joystick and then the button to allow it to go to neutral and be safe to stay
    // in cruise control. could add small time buffer after button release just to give some flexibility.
    // maybe have another button or something to add improved functionality?

    if (inCruiseMode && !switchPos) { // Cruise Control Constant Speed
      report = radio.write(&PWM, sizeof(PWM));
        if (report) {
          Serial.print(F("Transmission successful! "));  // payload was delivered
          Serial.print("Left Motor: ");
          Serial.println(PWM[0]);  // print payload sent
          Serial.print("Right Motor: ");
          Serial.println(PWM[1]);
        } else {
          Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
        }
    } else if (inCruiseMode && switchPos) {
      inCruiseMode = false;
      digitalWrite(cruiseLED, LOW);
    }

  // This device is a TX node
  if (!inCruiseMode) {
    if (Wire.available() == 2) {
          byte xData = Wire.read();  // Read X axis data
          byte yData = Wire.read();  // Read Y axis data

          //  X-axis (North-South)
          if (xData <= 80) {  // East side
              if (yData <= 80) {  // North-East 
                LeftMotorSpeed = 2000; // set to static full throttle on left motor
                RightMotorSpeed = map(xData, 0, 80, 2000, 1000);  // Map right motor speed 0-80 to 2000-1000
              } else if (yData >= 175) {  // South-East side
                LeftMotorSpeed = map(yData, 255, 175, 2000, 1000);  // Map left motor speed 255-175 to 2000-1000
                RightMotorSpeed = 1000; // set to static full reverse on right motor
              } 
          } else if (xData >= 175) {  // West side
              if (yData <= 80) {  // North-West side
                RightMotorSpeed = 2000; // set to static full throttle on right motor
                LeftMotorSpeed = map(yData, 0, 80, 1000, 2000);  // Map left motor speed 0-80 to 1000-2000
              } else if (yData >= 176) {  // South-West side
                RightMotorSpeed = map(xData, 255, 176, 1000, 2000);  // Map right motor speed 255-176 to 1000-2000
                LeftMotorSpeed = 1000;  // set to static full reverse on left motor
              }
          }

          // Print the raw values from Joystick 0-80 and 255-175
          // Serial.print("X Raw: ");
          // Serial.print(xData);
          Serial.print(" Right Motor: ");
          Serial.print(RightMotorSpeed);

          // Serial.print(" | Y Raw: ");
          // Serial.print(yData);
          Serial.print(" Left Motor: ");
          Serial.println(LeftMotorSpeed);
    }

    PWM[0] = RightMotorSpeed;
    PWM[1] = LeftMotorSpeed;

    bool report = radio.write(&PWM, sizeof(PWM));  // transmit & save the report

    if (report) {
      Serial.println(F("Transmission successful! "));  // payload was delivered
      Serial.println(PWM[0]);  // print payload sent
      Serial.println(PWM[1]);
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }
  }

}  // loop
