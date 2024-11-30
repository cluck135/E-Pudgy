/*
 * See documentation at https://nRF24.github.io/RF24
 * See License information at root directory of this library
 * Author: Brendan Doherty (2bndy5)
 */

#include <SPI.h>
// #include "printf.h"
#include "RF24.h"
#include <Servo.h>

// pins for Nano v3 sets 
#define CE_PIN 9
#define CSN_PIN 10
// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "pm117", "pc117"};
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
String bind = "pc117";

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 0;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = false;  // true = TX role, false = RX role

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
int payload[] = {100, 9, 9};

// boat motor pwm pins
int rightMotorPin = 3;
int leftMotorPin = 5;

int pwmLeftMotor;
int pwmRightMotor;

// control box Servos
Servo rightEsc;
Servo leftEsc;

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // print example's introductory prompt
  //Serial.println(F("RF24/examples/GettingStarted"));

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0
  // REMOVING WRITING FOR THE CONTROL BOX SINCE IT SHOULD ONLY RECEIVE
  // Serial.println((char*)address[!radioNumber]);
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[1]);  // using pipe 1
  radio.openReadingPipe(2, address[2]);  // using pipe 2

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }
 // radio.startListening();

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data
  rightEsc.attach(rightMotorPin);
  leftEsc.attach(leftMotorPin);
}  // setup

void loop() {

  uint8_t address[][6] = { "pm117", "pc117"};

    uint8_t pipe;
    int PWM[] = {};
    if (radio.available(&pipe)) {// is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&PWM, bytes);    // fetch payload from FIFO
    //   for (uint8_t i = 0; i < bytes; i++) {
    //     Serial.print("PWM[");
    //     Serial.print(i);
    //     Serial.print("]: ");
    //     Serial.println(PWM[i]);
    // }
      // if(PWM[2]){
      //   bind = pipe;
      // }
    
    //if(bind == pipe){
     // Serial.println(bind);
      // Serial.print("Bytes: ");
      // Serial.println(bytes);
      Serial.print("Left Throttle: ");
      Serial.println(PWM[0]);
      Serial.print("Right Throttle: ");
      Serial.println(PWM[1]);
      Serial.print("Switch Value: ");
      Serial.println(PWM[2]);

      rightEsc.writeMicroseconds(PWM[1]); // Send PWM signal to ESC1
      leftEsc.writeMicroseconds(PWM[0]); // Send PWM signal to ESC2
   // }
   
    
  } else {
    // Serial.println("no radio");
  } // radio.available()
}  // loop