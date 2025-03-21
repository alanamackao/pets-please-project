/*
Dog Pet

Intended to be integrated with a toy dog who will ask for pets.
When a motion-sensor senses motion, the dog will press its head forward. 
When somebody puts their hand over the dog's head, the dog's tail will begin to wag.
When they take their hand away, the dog will reset and wait a few seconds before
looking for people to ask for attention again.

Toy dog electronic components include 2 servos, a small breadboard-mountable 
PIR sensor, a photocell, and an Arduino Uno.
*/

#include <Servo.h>  // includes servo library to drive servo motors
/* FROM ADAFRUIT: 
Position "90" (1.5ms pulse) is stop, 
"180" (2ms pulse) is full speed forward, "0" (1ms pulse) is full speed backward.

(servo library sets speeds, instead of normal servo library position setting)
*/

// variable initialization ----------------------------------------------------

// photocell variables
int photoInPin = 5;     // analog input pin number used (A5)
int photoVal = 0;       // variable to store the value read by the ADC 
  // (analog-to-digital converter), Vout from photocell voltage divider with a 22kOhm resistor
int photoThresh = 350;  // threshold for the photocell reading
  // determines the cutoff for when photocell is considered "covered"

// servo variables
Servo neckServo;          // creates a servo object to control a servo
int forwardPosTime = 100; // duration in milliseconds to move neck servo to put head forward
Servo tailServo;          // second servo object
int wagTime = 100;        // duration in milliseconds to move tail servo to wag

// PIR (passive infrared) motion sensor variables
int nosePin = 12;       // digital input pin for the PIR sensor, acts as the dog's nose
int noseReading = LOW;  // start with no motion

// tracking variables
bool photoCovered = false;  // a boolean variable to track when photocell is covered
// bool wasPet = false;        // will track if the photocell was covered after being uncovered
bool sniff = true;          // bool to track if dog should check motion or not
bool relaxed = true;        // bool to track if dog is in relaxed position
bool keepWagging = false;   // bool to track if dog should keep wagging when photocell is only uncovered for a moment
bool unstick = true;    // bool to help exit an infinite loop

int timeUncovered = 0;     // tracks moment that the photocell is uncovered
int timeSincePet = 0;      // used to compare difference between moment photocell
  // is uncovered and time program has been running overall to find how long photocell has been uncovered
int waitForAttnCounter = 0;


// custom functions -----------------------------------------------------------

void relax() {
  /*
  Custom function to put dog in resting position and stay there before it can 
  continue checking if there is motion nearby and reacting to it.
  */            

  // if not already in relaxed position, then move:
  if (!relaxed) {
    Serial.println("relaxing");
    neckServo.write(100);     // start moving neck servo backwards at a little less than full speed
    delay(forwardPosTime);    // + (forwardPosTime/2)); // added a correction time value because it would drift forward
    neckServo.write(90);      // stop neck servo movement
    Serial.println("neck servo stopped");

           
    sniff = false;     // let dog check for motion again
    Serial.println("not sniffing");
    relaxed = true;
    delay(2500);      // wait in relaxed position for 2.5 seconds
  } else {
    Serial.println ("already relaxed");
  }

  sniff = true; 
  Serial.println("sniffing");
}

void wag() {
  /*
  Custom function to make sure dog's tail wags a minimum of 4 times even if 
  photocell is covered and uncovered again immediately.
  */
  Serial.println("tail servo wagging");
  for (int i = 0; i <= 3; i++) {
    // this for loop says the starting condition, i = 0, happens once.
      // As long as i is < = 3, the loop runs. At the end of the loop, i++
    tailServo.write(180);   // starts tail servo forwards at full speed
    delay(wagTime);         // wait for tail to reach up position
    tailServo.write(0);     // starts tail servo backwards at full speed
    delay(wagTime * 2);     // wait for tail to reach down position
    tailServo.write(180);   // starts tail servo forwards at full speed
    delay(wagTime);         // waits to get back to default position
  }
  tailServo.write(90);      // set tail servo speed to 0
  Serial.println("tail servo stopped");
}

void wagPosition() {
  /*
  Custom function to make sure dog's tail wags a minimum of 4 times even if 
  photocell is covered and uncovered again immediately.
  
  Adjusted for the position assigned servo
  */
  Serial.println("tail servo wagging");
  for (int i = 0; i <= 3; i++) {
    // this for loop says the starting condition, i = 0, happens once.
      // As long as i is < = 3, the loop runs. At the end of the loop, i++
    tailServo.write(110);   // starts tail servo forwards at full speed
    delay(100);             // wait 100 ms for tail to reach up position
    tailServo.write(90);    // go back to central position
    delay(100);             // wait for tail to reach down position
    tailServo.write(70);    // starts tail servo forwards at full speed
    delay(100);             // waits to get back to default position
  }
  tailServo.write(90);      // put tail back in relaxed position
  Serial.println("tail servo stopped");
}


void checkPhotocell() {
  /*
  Custom function to check the photocell. If the photocell is uncovered when
  it is checked, this function compares how long it has been since the
  photocell was first uncovered to the current moment. This will ensure that
  Pablo keeps wagging his tail if the photocell has only been uncovered for
  a moment while he is still being pet. 
  */
  photoVal = analogRead(photoInPin);  // read photocell
  if (photoVal < photoThresh) {
    if (photoCovered) {
      // record moment it was uncovered here
      timeUncovered = millis(); // millis gives number of milliseconds since program started
        // so timeUncovered keeps track of moment it was uncovered
    }
    photoCovered = false;
    timeSincePet = millis() - timeUncovered;  // gives elapsed time between 
      // time this line was run and moment dog was uncovered
    if (timeSincePet < 3000) {  // if it has been less than 3 seconds since the photocell was covered
      keepWagging = true;       // then keep wagging, person probably hasn't left yet
    } else {
      keepWagging = false;      // if it's been longer, person is likely gone
    }
  }
  else {
    photoCovered = true;        // loop will continue
  }
}


// setup and loop -------------------------------------------------------------

void setup() {
  Serial.begin(9600);   // begin serial communication for debugging

  // attach digital pins to servo objects
  neckServo.attach(9);  // attached to digital pin 9
  tailServo.attach(8);  // attach lines must be in setup() or loop() because 
    // they're dynamic code (calls a function)

  // initialize servo speeds to be 0
  neckServo.write(90);  
  tailServo.write(90);  // with position servo, sets position to 90 degrees

  // assign PIR pin mode
  pinMode(nosePin, INPUT);  // motion sensor pin as input, digital pin so will give HIGH or LOW only

  // initialize dog position
  relax();  // custom function written above
}

void loop() {  

  unstick = true; // reset unstick bool so whole loop will run by default

  // check if motion nearby ---------------------------------------------------

  if (sniff) {
    noseReading = digitalRead(nosePin); // check if nose detects motion

    if (noseReading == HIGH) {

      // if there IS motion nearby ------------------------------------------------

      Serial.println("motion detected");

      // put head in forward position if it is not already
      if (relaxed) {
        neckServo.write(80);   // spin neck servo backward slowly
        delay(forwardPosTime);  // wait for head to be put in forward position
        neckServo.write(90);    // stop neck servo spin
        Serial.println("neck servo stopped");
        Serial.println("pets please!");
        relaxed = false;
      }
      
      // check if being pet -----------------------------------------------------

      checkPhotocell();
      Serial.println("first photo check");

      // loop checking until being pet ----------------------------------------
      
      // loops as long as the dog is not being pet (photocell is uncovered)
      while (!photoCovered && unstick) {
        Serial.println("waiting to be pet");
        checkPhotocell();
        waitForAttnCounter += 1;
        if (waitForAttnCounter > 600) {
          Serial.println("took too long to pet");
          relax();
          unstick = false;
        } 
      }
      waitForAttnCounter = 0;
      // loop is left when dog is being pet


      // loop as long as dog is being pet -------------------------------------

      // loops as long as the dog is being pet (photocell is covered or has only been uncovered for a short while)
      while ((photoCovered || keepWagging) && unstick) {
        Serial.println("being pet");
        wag();
        checkPhotocell();
      }

      relax();
    }
  } 
}