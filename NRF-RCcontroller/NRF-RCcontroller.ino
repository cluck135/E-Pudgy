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
uint8_t address[][6] = { "Motor", "1ctrl", "2ctrl" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
uint8_t radioNumber = 2;  // setting to 2ctrl role

// Used to control whether this node is sending or receiving
bool role = true;  // true = TX role, false = RX role

// For this
// NEW SKETCH
// s example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
int payload[] = { 100, 9, 9 };

int rightThrotPin = A5;
int leftThrotPin = A2;

int switchPin = 2;

int leftThrot;
int rightThrot;
int switchPos;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // setup our throttle
  pinMode(rightThrotPin, INPUT);
  pinMode(leftThrotPin, INPUT);

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
  digitalWrite(switchPin, HIGH);  // set HIGH so we can read the pull down resistor value of 1 come through
}  // setup

void loop() {

  // This device is a TX node
  leftThrot = analogRead(leftThrotPin);
  rightThrot = analogRead(rightThrotPin);
  switchPos = digitalRead(switchPin);
  Serial.println(leftThrot);
  Serial.println(rightThrot);
  Serial.println(switchPos);
  // delay(1000);
  leftThrot = map(leftThrot, 0, 1023, 1000, 2000);
  rightThrot = map(rightThrot, 0, 1023, 1000, 2000);

  int PWM[] = { leftThrot, rightThrot, !switchPos };

  // unsigned long start_timer = micros();          // start the timer
  bool report = radio.write(&PWM, sizeof(PWM));  // transmit & save the report
  // unsigned long end_timer = micros();            // end the timer

  if (report) {
    Serial.print(F("Transmission successful! "));  // payload was delivered
    Serial.print(F("Time to transmit = "));
    // Serial.print(end_timer - start_timer);  // print the timer result
    Serial.println(F(" us. Sent: "));
    Serial.println(PWM[0]);  // print payload sent
    Serial.println(PWM[1]);
    Serial.print("Switch Val: ");
    Serial.println(PWM[2]);
  } else {
    Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
  }

  // to make this example readable in the serial monitor
  //delay(1000);  // slow transmissions down by 1 second


  if (Serial.available()) {
    // change the role via the serial monitor

    char c = toupper(Serial.read());
    if (c == 'T' && !role) {
      // Become the TX node

      role = true;
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      radio.stopListening();

    } else if (c == 'R' && role) {
      // Become the RX node

      role = false;
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      radio.startListening();
    }
  }

}  // loop