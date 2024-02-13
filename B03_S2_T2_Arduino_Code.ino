#include <MeMCore.h>
#define S1 A2
#define S2 A3
#define IR A1
//Delays for LDR and LED
#define LDRWait 10
#define RGBWait 500
int analogPin = A0;

//Empty array used to store R,G,B values for colour currently being detected
float colour_arr[] = {0, 0, 0};
//The R,B,G value for black and white card is pre-determined in isolation
float white_arr[] = {870, 870, 870};
float black_arr[] = {500, 500, 500};
float grey_diff[] = {370, 370, 370}; // The difference in the r,b,g value between white and black card
float final_colour[] = {0, 0, 0};

//Initialising Line Sensor
MeLineFollower LineFinder(PORT_1);

//Initialising Ultrasonic Sensor
MeUltrasonicSensor ultrasonic(PORT_2);

//Initialising buzzer
MeBuzzer buzzer;

int note, duration;

//Initialising Right and Left Motors
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);

//Motor Speed during forward movement
uint8_t motorSpeed1 = 151; //Right motor
uint8_t motorSpeed2 = 155; //Left Motor

/**
 * Finds the average reading out of n readings in the analogRead.
 * @param[in] n The number of readings in the analogRead to be considered in
   the calculation of average values.
 * @return The average value out of n readings in the analogRead
*/
float avg_reading(int n) {
  float reading = 0;
  float total = 0;
  for (int i = 0; i < n; i += 1) {
    reading = analogRead(analogPin);
    total += reading;  //The sum of n readings
    delay(LDRWait);
  }
  return total / n;
}
/**
 * Populating the final_color array which would be used for colour detection
   in each of the challenges.
 * Final_colour[0] refers to the ‘r’ value, final_colour[1] refers to the ‘b’
   value, and final_colour[2] refers to the ‘g’ value.
*/
void colour_detection() {
  for (int i = 0; i < 3; i += 1) {
    if (i == 0) {
      Serial.print("R = ");
      digitalWrite(S1, LOW);
      digitalWrite(S2, LOW);
    } else if (i == 1) {
      Serial.print("B = ");
      digitalWrite(S1, LOW);
      digitalWrite(S2, HIGH);
    } else {
      Serial.print("G = ");
      digitalWrite(S1, HIGH);
      digitalWrite(S2, LOW);
    }
    delay(RGBWait);
    colour_arr[i] = avg_reading(5);
    final_colour[i] = ((colour_arr[i] - black_arr[i]) / (grey_diff[i])) * 255;   // final=(colour-black)/(white-black)
    delay(RGBWait);
    Serial.println(final_colour[i]);
  }
}
/**
 * Adjust the output of pins S1 and S2 pins to turn the led off and turn the
   IR emitter on
*/
void offled_onir () {
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
}

/**
 * Adjusts the output of pins S1 and S2 to enable the LED to display red
*/
void on_red() {
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
}
/**
 * Adjusts the output of pins S1 and S2 to enable the LED to display green
*/
void on_green() {
  digitalWrite(S1, HIGH);
  digitalWrite(S2, LOW);
}
/**
 * Adjusts the output of pins S1 and S2 to enable the LED to display blue
*/
void on_blue() {
  digitalWrite(S1, LOW);
  digitalWrite(S2, HIGH);
}

/**
 * To pre-determine a set of conditions for the code
 * S1 and S2 are set to be OUTPUT pins
 * randomSeed provides a random sequence of numbers for the buzzer noise
 * Serial.begin allows the arduino to exchange messages at a certain bits per second
*/
void setup() {
  pinMode (S1, OUTPUT);
  pinMode (S2, OUTPUT);
  randomSeed(7);
  Serial.begin(9600);
}

void loop() {
  int sensorState = LineFinder.readSensors();
  switch (sensorState)
  {
    case S1_IN_S2_IN: {
        // Indicates that both sensors on Line Sensor has detected a black strip
        motor1.stop();
        motor2.stop();
        Serial.println("S1_IN_S2_IN");
        Serial.println(sensorState);
        delay(10);
        colour_detection();
        if ((final_colour[2] > final_colour[1]) &&
            (final_colour[2] > final_colour[0]) &&
            ((final_colour[2] - final_colour[0]) < 25)) // White
        {
          Serial.print("White"); // White detected - Signifies end of maze
          for (char b = 0; b < 10; b++) // To play celebratory tune
          {
            note = random(100, 1500); // Frequency range of numbers
            duration = random(50, 300); // Duration for each note
            buzzer.tone(note, duration);
          }
          delay(1000);
        }
        else if ((final_colour[0] > final_colour[1] &&
                  final_colour[0] > final_colour[2]) &&
                 abs(final_colour[1] - final_colour[2]) <= 10) // Red
        {
          Serial.print("Red"); // Red detected - Signifies Left Turn
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(motorSpeed2); // Left motor turns backward
          delay(400); // Delay set for a 90-degree turn
        }
        else if ((final_colour[0] > final_colour[1] &&
                  final_colour[0] > final_colour[2]) &&
                 (final_colour[2] > final_colour[1])) // Orange
        {
          Serial.print("Orange"); // Orange detected - Signifies U-Turn
          motor1.run(-motorSpeed1); // Right motor turns backward
          motor2.run(-motorSpeed2); // Left motor turns forward
          delay(700); // Delay set for a 180-degree turn
        }
        else if ((final_colour[2] > final_colour[0] &&
                  final_colour[2] > final_colour[1]) &&
                 (final_colour[1] > final_colour[0])) // Green
        {
          Serial.print("Green"); // Green detected - Signifies Right Turn
          motor1.run(-motorSpeed1); // Right motor turns backward
          motor2.run(-motorSpeed2); // Left motor turns forward
          delay(350); // Delay set for a 90-degree turn
        }
        else if ((final_colour[1] > final_colour[0] &&
                  final_colour[1] > final_colour[2]) &&
                 (abs(final_colour[0] - final_colour[2])) <= 30) // Purple
        {
          Serial.print("Purple"); // Purple detected - Signifies 2 successive Left Turns
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(motorSpeed2); // Left motor turns backward
          delay(420); // Delay set for a 90-degree turn
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(-motorSpeed2); // Left motor turns forward
          delay(750); // Delay set for mBot to go straight
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(motorSpeed2); // Left motor turns backward
          delay(420); // Delay set for a 90-degree turn
        }
        else if ((final_colour[1] > final_colour[0] &&
                  final_colour[1] > final_colour[2]) &&
                 (final_colour[2] > final_colour[0])) // Blue
        {
          Serial.print("Blue"); // Blue detected - Signifies 2 successive Right Turns
          motor1.run(-motorSpeed1); // Right motor turns backward
          motor2.run(-motorSpeed2); // Left motor turns forward
          delay(440); // Delay set for a 90-degree turn
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(-motorSpeed2); // Left motor turn forward
          delay(750); // Delay set for mBot to go straight
          motor1.run(-motorSpeed1); // Right motor turns backward
          motor2.run(-motorSpeed2); // Left motor turns forward
          delay(440); // Delay set for a 90-degree turn
        }
        break;
      }
    case S1_IN_S2_OUT:
    //Indicates that only one of the sensors on Line Sensor has detected a black strip
    case S1_OUT_S2_IN:
    //Indicates that only one of the sensors on Line Sensor has detected a black strip
    case S1_OUT_S2_OUT: {
        //Indicates that neither of the sensors on Line Sensor has detected a black strip
        offled_onir();
        Serial.println("S1_OUT_S2_OUT"); // Printing out values for debugging
        Serial.println(sensorState); // Printing out values for debugging
        Serial.println(analogRead(IR)); // Printing out values for debugging
        while (ultrasonic.distanceCm() < 5)
          //While it is too close to the left wall, turn right a bit
        {
          motor1.run(-motorSpeed1); // Right motor turns backward
          motor2.run(-motorSpeed2 - 5); // Left motor turns forward with slight increase in speed
          delay(50);
        }
        while (ultrasonic.distanceCm() > 8 && analogRead(IR) < 900)
          //While it is too close to the right wall, turn left a bit
        {
          motor1.run(motorSpeed1); // Right motor turns forward
          motor2.run(motorSpeed2 + 2);// Left motor turns backward with slight decrease in speed
          delay(30);
        }
        // Continue going straight
        motor1.run(motorSpeed1); // Right motor turns forward
        motor2.run(-motorSpeed2); // Left motor turns forward
        Serial.println(ultrasonic.distanceCm());
        delay(10);
      }
  }
  delay(10); // Delay after every loop
}
