#include <Servo.h>
#include "U8glib.h"
#include "floatToString.h"

// oled display config
U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9);	// SW SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9
String DANGER_MSG = "Danger at ", INCOMING_MSG = "Object incoming at ", DETECT_MSG = "Object detected at ";

// pins and variables for servos
Servo rightHandServo; // with attached ultrasonic sensor
Servo leftHandServo;

int rightServoPos = 0;
int leftServoPos = 0;
int servoDelay=1000;
const int SERVO_RANGE=180, ANGLE_TO_MOVE=10, DANGER_ANGLE_TO_MOVE=30;
const int DEFAULT_DELAY=1000, DANGER_DELAY=200, INCOMING_DELAY=600, DETECT_DELAY=800; // measured in ms
const int DETECT_DIST=395, INCOMING_DIST=200, DANGER_DIST=100; // measured in cm

boolean IN_RANGE=false;
boolean IS_DANGER=false;

const int RIGHT_SERVO=6;
const int LEFT_SERVO=5;

// pins for leds
const int LEFT_BLUE_LED=4;
const int RIGHT_BLUE_LED=3;

const int RIGHT_RED_LED=2;
const int RED_BLINKS=3;

// pins and variables for ultrasonic sensor
const int TRIG = A2;
const int ECHO = A1;
float DURATION;
char S[15];

// ----------------------------
// main setup
void setup() {
  Serial.begin(9600);
  pinMode (TRIG, OUTPUT);
  pinMode (ECHO, INPUT);

  pinMode(LEFT_BLUE_LED, OUTPUT);
  pinMode(RIGHT_BLUE_LED, OUTPUT);
  pinMode(RIGHT_RED_LED, OUTPUT);

  rightHandServo.attach(RIGHT_SERVO);
  leftHandServo.attach(LEFT_SERVO);

  Serial.println("Done pins configs");

}

// ----------------------------
// main loop
void loop() {
  Serial.flush();

  float distance = ultraSense();

  if (IS_DANGER==true) alarm();

  digitalWrite(LEFT_BLUE_LED, LOW);
  digitalWrite(RIGHT_BLUE_LED, LOW);

  if (distance < DETECT_DIST) {
    IS_DANGER=false;
    IN_RANGE = true;
      
    if (distance < DANGER_DIST ) {
      IS_DANGER=true;
      clearOLED();
      servoDelay = DANGER_DELAY;

      showOnDisplay(DANGER_MSG, distance);

      alarm();

    } 

    else if (distance < INCOMING_DIST)  {
      clearOLED();
      servoDelay = INCOMING_DELAY;

      showOnDisplay(INCOMING_MSG, distance);

      digitalWrite(LEFT_BLUE_LED, HIGH);
      digitalWrite(RIGHT_BLUE_LED, HIGH);
    } 

    else {
      clearOLED();
      servoDelay = DETECT_DELAY;

      showOnDisplay(DETECT_MSG, distance);

      digitalWrite(LEFT_BLUE_LED, HIGH);
      digitalWrite(RIGHT_BLUE_LED, HIGH);
    }

  } else {
    IN_RANGE = false;
    servoDelay = DEFAULT_DELAY;
  }

  if (IS_DANGER==true) alarm();

  leftServoPos = (leftServoPos+ANGLE_TO_MOVE) % SERVO_RANGE;
  leftHandServo.write(leftServoPos);

  if (IN_RANGE==false) {
    rightServoPos = (rightServoPos+ANGLE_TO_MOVE) % SERVO_RANGE;
    rightHandServo.write(rightServoPos);
  }

  delay(servoDelay);
}

// ----------------------------
// helper functions
// ----------------------------

float ultraSense() {
  /* gets the distance measured by ultrasonic sensor */
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  DURATION = pulseIn(ECHO, HIGH);
  float distance = (DURATION / 2e4) * 343; // convert to cm
  Serial.println(distance);

  return distance;
}

void showOnDisplay(String msg, float distance) {
  /* displays the message and the distance of detected object */
  u8g.firstPage();
  do {
    draw(msg, distance);
  } while( u8g.nextPage() );
}

void clearOLED() {
  /* clears the oled display, prevents overwriting text */
  u8g.firstPage();
  do {} while( u8g.nextPage() );
}

void draw(String msg, float distance) {
  /* prints message on the first line and the distance from the detected
  object in the next line */
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(0,20);
  u8g.print(msg);

  u8g.setPrintPos(0,40);
  String dist = floatToString(distance, S, sizeof(S), 2);
  u8g.print(dist + " cm");
}

void alarm() {
  /* blinks the red led and increases the angle of rotation of the servo */
  for (int i = 0; i < RED_BLINKS; i++) {
    leftServoPos = (leftServoPos + DANGER_ANGLE_TO_MOVE) % SERVO_RANGE;

    digitalWrite(RIGHT_RED_LED, HIGH);
    delay(100);
    digitalWrite(RIGHT_RED_LED, LOW);
    
    leftHandServo.write(leftServoPos);
  }
}
