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
int forwardPosTime = 400; // duration in milliseconds to move neck servo to put head forward
Servo tailServo;          // second servo object
int wagTime = 100;        // duration in milliseconds to move tail servo to wag

// PIR (passive infrared) motion sensor variables
int nosePin = 12;       // digital input pin for the PIR sensor, acts as the dog's nose
int noseReading = LOW;  // start with no motion

// tracking variables
bool photoCovered = false; // a boolean variable to track when photocell is covered
bool wasPet = false;       // will track if the photocell was covered after being uncovered
bool sniff = true;         // bool to track if dog should check motion or not
bool relaxed = true;       // bool to track if dog is in relaxed position

int timeUncovered = 0;     // tracks moment that the photocell is uncovered
int timeSincePet = 0;      // used to compare difference between moment photocell
  // is uncovered and time program has been running overall to find how long photocell has been uncovered


// custom functions -----------------------------------------------------------

void relax() {
  /*
  Custom function to put dog in resting position and stay there before it can 
  continue checking if there is motion nearby and reacting to it.
  */
  // if not already in relaxed position, then move:
  if (!relaxed) {
    Serial.println("relaxing");
    neckServo.write(20);     // start moving neck servo backwards at a little less than full speed
    delay(forwardPosTime);
    neckServo.write(90);    // stop neck servo movement
    Serial.println("neck servo stopped");

    // tailServo.write(tailRestPos); // put tail in resting position
    wasPet = false;               
    sniff = false;         // let dog check for motion again
    Serial.println("not sniffing");
    relaxed = true;
  } else {
    Serial.println ("already relaxed");
  }

  delay(2500);      // wait in relaxed position for 2.5 seconds
  sniff = true; // ### should this be here??
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

void checkPhotocell() {
  photoVal = analogRead(photoInPin);  // read photocell
  // Serial.println(photoVal);  // print ADC reading to serial monitor
  if (photoVal < photoThresh) {
    if (photoCovered) {
      wasPet = true;  // dog was pet if the photocell has been uncovered after being covered

      // record moment it was uncovered here??
      timeUncovered = millis(); // millis gives number of milliseconds since program started
        // so timeUncovered keeps track of moment it was uncovered
    }
    photoCovered = false;

    timeSincePet = millis() - timeUncovered;  // gives elapsed time between 
      // time this line was run and moment dog was uncovered
  }
  else {
    photoCovered = true;
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
  tailServo.write(90);  

  // assign PIR pin mode
  pinMode(nosePin, INPUT);  // motion sensor pin as input, digital pin so will give HIGH or LOW only

  // initialize dog position
  relax();  // custom function written above
}

void loop() {  

  // check if motion nearby ---------------------------------------------------

  if (sniff) {
    noseReading = digitalRead(nosePin); // check if nose detects motion

    if (noseReading == HIGH) {

      // if there IS motion nearby ------------------------------------------------

      Serial.println("motion detected");

      // put head in forward position if it is not already
      if (relaxed) {
        neckServo.write(160);   // spin neck servo forward at slightly less than full speed
        delay(forwardPosTime);  // wait for head to be put in forward position
        neckServo.write(90);    // stop neck servo spin
        Serial.println("neck servo stopped");
        Serial.println("pets please!");
        relaxed = false;
      }
      
      // check if being pet -----------------------------------------------------

      checkPhotocell();

      // loop checking until being pet ----------------------------------------
      
      // loops as long as the dog is not being pet (photocell is uncovered)
      while (!photoCovered) {
        // timeSincePet = millis() - timeUncovered;  // gives elapsed time between 
          // time this line was run and moment dog was uncovered
        // Serial.print("time since pet: ");
        // Serial.println(timeSincePet);

        if (timeSincePet > 4000) {  // if it has been more than 4 seconds since being pet
          relax();
        }

        checkPhotocell();

        // // Serial.println("uncovered");
        // photoVal = analogRead(photoInPin);  // read photocell
        // // Serial.println(photoVal);  // print ADC reading to serial monitor

        // // compares ADC reading to threshold set at top to see if photocell is covered
        // if (photoVal > photoThresh) {
        //   // Serial.println("covered");
        //   wasPet = true;  // says dog was pet if photocell is covered after being uncovered
        //   photoCovered = true;
        // }
      }
      // loop is left when dog is being pet


      // loop as long as dog is being pet -------------------------------------

      // loops as long as the dog is being pet (photocell is covered)
      while (photoCovered /*could add another OR conditional here for if it's being pet*/) {
        wag();
        checkPhotocell();

        // photoVal = analogRead(photoInPin);  // read photocell
        // // Serial.println(photoVal);  // print ADC reading to serial monitor

        // // if the photocell is uncovered
        // if (photoVal < photoThresh) {

        //   // by using photoCovered as conditional, only does this the first time it's uncovered
        //   if (photoCovered && wasPet) {
        //     // timeUncovered = millis(); // millis gives number of milliseconds since program started
        //     // so timeUncovered keeps track of moment it was uncovered
        //   }
        //   photoCovered = false;
        //   Serial.println("uncovered");
        
          // sniff = false;  // prevent dog from checking for motion
          // Serial.println("not sniffing");


          // create counter or create cases
        }
      }



      // // loops as long as the dog is being pet (photocell is covered)
      // while (photoCovered /*could add another conditional here for if it's being pet*/) {
      //   wag();
      //   photoVal = analogRead(photoInPin);  // read photocell
      //   // Serial.println(photoVal);  // print ADC reading to serial monitor

      //   // if the photocell is uncovered
      //   if (photoVal < photoThresh) {

      //     // by using photoCovered as conditional, only does this the first time it's uncovered
      //     if (photoCovered && wasPet) {
      //       // timeUncovered = millis(); // millis gives number of milliseconds since program started
      //       // so timeUncovered keeps track of moment it was uncovered
      //     }
      //     photoCovered = false;
      //     Serial.println("uncovered");
        
      //     // sniff = false;  // prevent dog from checking for motion
      //     // Serial.println("not sniffing");


      //     // create counter or create cases
      //   }
      // }
    }
  } 
}