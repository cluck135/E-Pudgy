/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

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

// For this
// NEW SKETCH
// s example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
int payload[] = { 100, 9, 9 };

int switchPin = 5;
unsigned long buttonPressStart = 0; // Timer variable
const unsigned long holdTime = 5000; // 5 seconds in milliseconds
bool inCruiseMode = false;

int ForwardBackward;
int ForwardBackwardLOCK;
int RightLeft;
int RightLeftLOCK;
int switchPos;

int TransistorBasePin = 3;


void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // setup our throttle
  pinMode(ForwardBackwardPotPin, INPUT);
  pinMode(TurningPotPin, INPUT);

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // print example's introductory prompt
  // Serial.println(F("RF24/examples/GettingStarted"));

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
  digitalWrite(switchPin, LOW);  // set HIGH so we can read the pull down resistor value of 1 come through
}  // setup

void loop() {
   Wire.beginTransmission(JOYSTICK_ADDR);
  Wire.write(0x00);  // Register to start reading from (verify if this is correct)
  Wire.endTransmission(false);

  Wire.requestFrom(JOYSTICK_ADDR, 2);  // Request 2 bytes (X and Y axis data)

  if (Wire.available() == 2) {
    byte xData = Wire.read();  // Read X axis data
    byte yData = Wire.read();  // Read Y axis data

    int mappedX = 0;
    int mappedY = 0;

    // Map X-axis (North-South)
    if (xData <= 80) {  // North side
      mappedX = map(xData, 0, 80, 1000, 2000);  // Map 0-80 to 1000-2000
    } else if (xData >= 176) {  // South side
      mappedX = map(xData, 175, 255, 1000, 2000);  // Map 255-176 to 1000-2000
    } else {
      mappedX = 1500;  // Center position
    }

    // Map Y-axis (East-West)
    if (yData <= 80) {  // East side
      mappedY = map(yData, 0, 80, 1000, 2000);  // Map 0-80 to 1000-2000
    } else if (yData >= 176) {  // West side
      mappedY = map(yData, 255, 176, 1000, 2000);  // Map 255-176 to 1000-2000
    } else {
      mappedY = 1000;  // Center position
    }

    // Print the raw and mapped values for both axes
    Serial.print("X Raw: ");
    Serial.print(xData);
    Serial.print(" Mapped X: ");
    Serial.print(mappedX);

    Serial.print(" | Y Raw: ");
    Serial.print(yData);
    Serial.print(" Mapped Y: ");
    Serial.println(mappedY);
  }

  delay(100);  // Delay for readability

  int inputRead = digitalRead(5);
  if (inputRead == HIGH) {
    digitalWrite(2, HIGH);
  } 



  int PWM[3];
  bool report;

  switchPos = digitalRead(switchPin);
  Serial.println(switchPos);
   if (!inCruiseMode && switchPos) {
        // If button press just started, record the start time
        if (buttonPressStart == 0) {
            buttonPressStart = millis();
        }
        // Check if button is held for more than 5 seconds
        if (millis() - buttonPressStart >= holdTime && !inCruiseMode) {
            inCruiseMode = true;       // Activate cruise mode
            //digitalWrite(cruiseLED, HIGH); // Turn on cruise mode LED
            Serial.println("Cruise mode activated!");
            while (switchPos) {
              ForwardBackwardLOCK = analogRead(ForwardBackwardPotPin);
              RightLeftLOCK = analogRead(TurningPotPin);
              ForwardBackwardLOCK = map(ForwardBackwardLOCK, 0, 1023, 1000, 2000);
              RightLeftLOCK = map(RightLeftLOCK, 0, 1023, 1000, 2000);
              switchPos = digitalRead(switchPin);
              PWM[0] = ForwardBackwardLOCK;
              PWM[1] = RightLeftLOCK;
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

    if (inCruiseMode && !switchPos) { 
      PWM[2] = switchPos;
      report = radio.write(&PWM, sizeof(PWM));
        if (report) {
          Serial.print(F("Transmission successful! "));  // payload was delivered
          Serial.print("Forward Backward: ");
          Serial.println(PWM[0]);  // print payload sent
          Serial.print("Right Left: ");
          Serial.println(PWM[1]);
          Serial.print("Switch Val: ");
          Serial.println(PWM[2]);
        } else {
          Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
        }
    } else if (inCruiseMode && switchPos) {
      inCruiseMode = false;
    }

  // This device is a TX node
  if (!inCruiseMode) {
  ForwardBackward = analogRead(ForwardBackwardPotPin);
  RightLeft = analogRead(TurningPotPin);
  // Serial.println(ForwardBackward);
  // Serial.println(RightLeft);
  // Serial.println(switchPos);
  ForwardBackward = map(ForwardBackward, 0, 1023, 1000, 2000);
  RightLeft = map(RightLeft, 0, 1023, 1000, 2000);
  // Serial.print("ForwardBackward: ");
  // Serial.println(ForwardBackward);
  // Serial.print("RightLeft: ");
  // Serial.println(RightLeft);

  PWM[0] = ForwardBackward;
 // PWM[1] = RightLeft;
  //PWM[2] = switchPos;

  //unsigned long start_timer = micros();          // start the timer
  bool report = radio.write(&PWM, sizeof(PWM));  // transmit & save the report
  //unsigned long end_timer = micros();            // end the timer

    if (report) {
      Serial.println(F("Transmission successful! "));  // payload was delivered
      // Serial.print(F("Time to transmit = "));
      // Serial.print(end_timer - start_timer);  // print the timer result
      // Serial.println(F(" us. Sent: "));
      Serial.println(PWM[0]);  // print payload sent
      Serial.println(PWM[1]);
      // Serial.print("Switch Val: ");
      Serial.println(PWM[2]);
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }
  }
  // to make this example readable in the serial monitor
  //delay(1000);  // slow transmissions down by 1 second


  // if (Serial.available()) {
  //   // change the role via the serial monitor

  //   char c = toupper(Serial.read());
  //   if (c == 'T' && !role) {
  //     // Become the TX node

  //     role = true;
  //     Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
  //     radio.stopListening();

  //   } else if (c == 'R' && role) {
  //     // Become the RX node

  //     role = false;
  //     Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
  //     radio.startListening();
  //   }
  // }

}  // loop