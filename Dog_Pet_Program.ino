/*
Dog Pet

Intended to be integrated with a toy dog who will ask for pets.
When a motion-sensor senses motion, the dog will press its head forward. 
When somebody puts their hand over the dog's head, the dog's tail will begin to wag.
When they take their hand away, the dog will reset and wait a few seconds before
looking for people again.
*/

#include <Servo.h>  // includes servo library to drive servo motors
/* FROM ADAFRUIT: 
Position "90" (1.5ms pulse) is stop, 
"180" (2ms pulse) is full speed forward, 
"0" (1ms pulse) is full speed backward.

(servo library sets speeds, instead of normal servo library position setting)
*/

// photocell variables
int photoInPin = 5;     // analog input pin number used (A5)
int photoVal = 0;       // variable to store the value read by the ADC 
  // (analog-to-digital converter), Vout from photocell voltage divider with a 22kOhm resistor
int photoThresh = 350;  // threshold for the photocell reading
  // determines the cutoff for when photocell is considered "covered"

// servo variables
Servo neckServo;      // creates a servo object to control a servo
int forwardPosTime = 400; // duration in milliseconds to move neck servo to put head forward
Servo tailServo;          // second servo object
int wagTime = 100;     // duration in milliseconds to move tail servo to wag

// PIR (passive infrared) motion sensor variables
int nosePin = 12;       // digital input pin for the PIR sensor, acts as the dog's nose
int noseReading = LOW;  // start with no motion

// tracking variables
bool photoCovered = false; // a boolean variable to track when photocell is covered
bool wasPet = false;       // will track if the photocell was covered after being uncovered
bool shouldAsk = false;    // bool to track if there's a person nearby (if motion was detected)
bool sniff = true;         // bool to track if dog should check motion or not
bool asking = false;       // bool to track if head is in forward position
bool relaxed = false;       // bool to track if dog is in relaxed position

int timeUncovered = 1500;   // tracks moment that the photocell is uncovered
int timeSincePet = 0;       // used to compare difference between moment photocell
  // is uncovered and time program has been running overall to find how long photocell has been uncovered


// custom functions -----------------------------------------------------------

void relax() {
  /*
  Custom function to put dog in resting position and stay there before it can 
  continue checking if there is motion nearby and reacting to it.
  */
  Serial.println("relaxing");
  if (!relaxed) {
    neckServo.write(20);     // start moving neck servo backwards at a little less than full speed
    delay(forwardPosTime);
    neckServo.write(90);    // stop neck servo movement
    Serial.println("neck servo stopped");
    asking = false;   // says dog's head is not in the asking position

    // tailServo.write(tailRestPos); // put tail in resting position
    shouldAsk = false;            // tell dog not to move or check if person nearby
    wasPet = false;               
    sniff = true;         // let dog check for motion again
    Serial.println("sniffing");
    relaxed = true;
  }

  delay(2500);                  // wait in relaxed position for 2.5 seconds
  
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



// setup and loop -------------------------------------------------------------

void setup() {
  Serial.begin(9600);   // begin serial communication for debugging

  // attach digital pins to servo objects
  neckServo.attach(9);  // attached to digital pin 9
  tailServo.attach(8);  // must be in setup() or loop() because it's dynamic code (calls a function)

  // initialize servo speeds to be 0
  neckServo.write(90);  
  tailServo.write(90);  

  // assign PIR pin mode
  pinMode(nosePin, INPUT);  // motion sensor pin as input, digital pin so will give HIGH or LOW only

  // initialize dog position
  relax();
}

void loop() {  

  // check if motion nearby ---------------------------------------------------

  if (sniff) {
    noseReading = digitalRead(nosePin); // check if nose detects motion
    if (noseReading == HIGH) {
      shouldAsk = true; // tells dog to ask for pets if there is motion nearby
      Serial.println("motion detected");
    }
  }

  // if there IS motion nearby ------------------------------------------------

  Serial.print("shouldAsk: ");  Serial.println(shouldAsk);
  if (shouldAsk) {
    // put head in forward position if it is not already
    if (!asking) {
      neckServo.write(160);   // spin neck servo forward at slightly less than full speed
      delay(forwardPosTime);  // wait for head to be put in forward position
      neckServo.write(90);    // stop neck servo spin
      Serial.println("neck servo stopped");
      Serial.println("pets please!");
      asking = true;
      relaxed = false;
    }
    

    // check if being pet -----------------------------------------------------

    // loops as long as the dog is not being pet (photocell is uncovered)
    while (!photoCovered) {
      timeSincePet = millis() - timeUncovered;  // gives elapsed time between 
        // time this line was run and moment dog was uncovered
      Serial.print("time since pet: ");
      Serial.println(timeSincePet);

      if (timeSincePet > 1500) {
        relax();
      }

      // Serial.println("uncovered");
      photoVal = analogRead(photoInPin);  // read photocell
      // Serial.println(photoVal);  // print ADC reading to serial monitor

      // compares ADC reading to threshold set at top to see if photocell is covered
      if (photoVal > photoThresh) {
        // Serial.println("covered");
        wasPet = true;  // says dog was pet if photocell is covered after being uncovered
        photoCovered = true;
      }
    }
    // loop is left when dog is being pet

    // loops as long as the dog is being pet (photocell is covered)
    while (photoCovered) {
      wag();
      photoVal = analogRead(photoInPin);  // read photocell
      // Serial.println(photoVal);  // print ADC reading to serial monitor

      // if the photocell is uncovered
      if (photoVal < photoThresh) {

        // by using photoCovered as conditional, only does this the first time it's uncovered
        if (photoCovered && wasPet) {
          timeUncovered = millis(); // millis gives number of milliseconds since program started
          // so timeUncovered keeps track of moment it was uncovered
        }
        photoCovered = false;
        Serial.println("uncovered");
      
        sniff = false;  // prevent dog from checking for motion
        Serial.println("not sniffing");
        Serial.print("shouldAsk: ");  Serial.println(shouldAsk);


        // create counter or create cases
        // timeSincePet = millis() - timeUncovered;  // gives elapsed time between 
        //   // time this line was run and moment dog was uncovered
        // Serial.print("time since pet: ");
        // Serial.println(timeSincePet);

        // if (timeSincePet > 1500 && wasPet) {
        //   relax();
        // }

      }
    }
  }
  


  // // servo variables
  // Servo neckServo;      // creates a servo object to control a servo
  // int forwardPos = 180; // position in degrees of neck servo when head is forward
  // int restingPos = 0;   // position in degrees of neck servo when head is resting
  // Servo tailServo;      // second servo object
  // int wagUpPos = 180;   // position in degrees of tail servo at top of tail wag
  // int wagDownPos = 0;   // position in degrees of tail servo at bottom of tail wag
  // int tailRestPos = 90; // position in degrees of tail servo at resting position

  // // check if being pet -------------------------------------------------------

  //   photoVal = analogRead(photoInPin);  // read photocell
  //   Serial.println(photoVal);  // print ADC reading to serial monitor
    
  //   // compares ADC reading to threshold set at top to see if photocell is covered
  //   if (photoVal > photoThresh){
  //     Serial.println("covered");

  //     // if the photocell was uncovered and is now covered, say the dog was pet
  //     if (!photoCovered) {
  //       wasPet = true;
  //     }
  //     photoCovered = true;

  //     // this for loop makes sure that the tail wags a minimum of 4 times
  //     for (int i = 0; i <= 3; i++) {
  //       // this for loop says the starting condition, i = 0, happens once.
  //         // As long as i is < = 3, the loop runs. At the end of the loop, i++
  //       tailServo.write(wagUpPos);    // sends the tail servo to the 'up' wagging position
  //       delay(100);                   // 100ms, wait for tail to reach position
  //       tailServo.write(wagDownPos);  // sends the tail servo to the 'down' wagging position
  //       delay(100);                   // 100ms, wait for tail to reach position
  //     }
      
  //   } else {  // if the photocell is uncovered
  //     Serial.println("uncovered");
  //     tailServo.write(tailRestPos);

  //     // when the photocell is first uncovered, keep track of the moment
  //     if (photoCovered) {
  //       timeUncovered = millis(); // millis gives number of milliseconds since program started
  //       // so timeUncovered keeps track of moment it was uncovered
  //     }
  //     photoCovered = false;

  //     timeSincePet = millis() - timeUncovered;
  //     Serial.print("time since pet: ");
  //     Serial.println(timeSincePet);

  //     // if it has been more than 1.5 seconds since the photocell has been covered, reset positions
  //     if (timeSincePet > 1500 && wasPet) {
  //       relax();
  //       delay(500); // wait 500ms (0.5 seconds)
  //       wasPet = false;
  //     }

  // } else {
  //   // if no motion, continue:


  // }

}