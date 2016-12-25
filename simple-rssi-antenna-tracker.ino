/*
  Simple RSSI Antenna Tracker



RELATIVE
*/

#include <Servo.h>
#include <Timer.h>

/* 
 *  For testing different curves
 *  All new curves are more precise
 *  and maintain lock better than old RELATIVE
 *  
 *  Uncomment one 
 */
//#define EXPONENTIAL       // OK
//#define RELATIVE          // old, don't use
#define SIGMOID             // Best
//#define PROPORTIONAL      // twitchy

#define SIGMOID_SLOPE       1
#define SIGMOID_OFFSET      4

#if !defined(EXPONENTIAL) && !defined(SIGMOID) && !defined(PROPORTIONAL) && !defined(RELATIVE)
  #error "Please define tracking curve: EXPONENTIAL, SIGMOID, PROPORTIONAL, RELATIVE"
#endif


/*
   Pin mapping
   You can change these pins, just make sure that
   RSSI pins are analog inputs
   and servo pin supports PWM
*/
#define LEFT_RSSI_PIN       0     // analog RSSI measurement pin
#define RIGHT_RSSI_PIN      1
#define PAN_SERVO_PIN       5     // Pan servo pin

/*
   There is going to be some difference in receivers,
   one is going to be less sensitive than the other.
   It will result in slight offset when tracker is centered on target

   Find value that evens out RSSI
 */
#define RSSI_OFFSET_RIGHT   17
#define RSSI_OFFSET_LEFT    0

/*
   MIN and MAX RSSI values corresponding to 0% and 100% from receiver
   See serial monitor
*/
#define RSSI_MAX            400
#define RSSI_MIN            120

/*
   Safety limits for servo

   Find values where servo doesn't buzz at full deflection
*/
#define SERVO_MAX           175
#define SERVO_MIN           3

/*
 * Servo 'speed' limits
 * MAX and MIN step in degrees
 */
#define SERVO_MAX_STEP      5
#define SERVO_MIN_STEP      0.09     // prevents windup and servo crawl

/*
   Center deadband value!
   Prevents tracker from oscillating when target is in the center
   When using SIGMOID or EXPONENTIAL you can set this almost to 0
*/
#define DEADBAND            15

/*
   Depending which way around you put your servo
   you may have to change direction

   either 1 or -1
*/
#define SERVO_DIRECTION     1

#define FIR_SIZE            10
#define LAST                FIR_SIZE - 1


uint16_t rssi_left_array[FIR_SIZE];
uint16_t rssi_right_array[FIR_SIZE];

float anglePan = 90;
boolean debug = false;

Timer timer;
Servo servoPan;


void setup() {
  servoPan.attach(PAN_SERVO_PIN);
  servoPan.write(90);

  // wipe array
  for (int i = 0; i < FIR_SIZE; i++) {
    rssi_right_array[i] = 0;
    rssi_left_array[i] = 0;
  }

  Serial.begin(115200);
  while (!debug) {
    delay(3000);
    debug = true;
  }


  timer.every(50, mainLoop);
  timer.every(5, measureRSSI);
}


void loop() {

  timer.update();

}


void mainLoop() {

  uint16_t avgLeft = max(avg(rssi_left_array, FIR_SIZE) + RSSI_OFFSET_LEFT, RSSI_MIN);
  uint16_t avgRight = max(avg(rssi_right_array, FIR_SIZE) + RSSI_OFFSET_RIGHT, RSSI_MIN);

  // If avg RSSI is above 90%, don't move
  if ((avgRight + avgLeft) / 2 > 360) {
    return;
  }

  /*
     the lower total RSSI is, the lower deadband gets
     allows more precise tracking then target is far away
  */
  uint8_t dynamicDeadband = (float(avgRight + avgLeft) / 2 - RSSI_MIN) / (RSSI_MAX - RSSI_MIN) * DEADBAND;

  // if target is in the middle, don't move
  if (abs(avgRight - avgLeft) < dynamicDeadband ) {
    return;
  }

  float ang = 0;

  // move towards stronger signal
  if (avgRight > avgLeft) {
  
  #if defined(EXPONENTIAL)
    float x = float(avgRight - avgLeft);
    x = x * x / 500;
    ang = x * SERVO_DIRECTION * -1;
  #endif

  #if defined(RELATIVE)
    ang = float(avgRight / avgLeft) * (SERVO_DIRECTION * -1);
  #endif

  #if defined(SIGMOID)
    float x = float(avgRight - avgLeft) / 10;
    x = SERVO_MAX_STEP / (1+ exp(-SIGMOID_SLOPE * x + SIGMOID_OFFSET));
    ang = x * SERVO_DIRECTION * -1;
  #endif
    
  #if defined(PROPORTIONAL)
    float x = float(avgRight - avgLeft) / 10;
    ang = x * SERVO_DIRECTION * -1;  
  #endif
  }
  else {

  #if defined(EXPONENTIAL)
    float x = float(avgLeft - avgRight);
    x = x * x / 500;
    ang = x * SERVO_DIRECTION;
  #endif

  #if defined(RELATIVE)
    ang = float(avgLeft / avgRight) * SERVO_DIRECTION;
  #endif

  #if defined(SIGMOID)
    float x = float(avgLeft - avgRight) / 10;
    x = SERVO_MAX_STEP / (1+ pow(EULER, -SIGMOID_SLOPE * x + SIGMOID_OFFSET));
    ang = x * SERVO_DIRECTION;
  #endif
    
  #if defined(PROPORTIONAL)
    float x = float(avgLeft - avgRight) / 10;
    ang = x * SERVO_DIRECTION;  
  #endif
  }

  // upper and lower limit for angle step
  ang = (abs(ang) > SERVO_MAX_STEP ? SERVO_MAX_STEP * ang/abs(ang) : ang);
  ang = (abs(ang) < SERVO_MIN_STEP ? 0 : ang);

  // move servo by n degrees
  movePanBy(ang);


  if (debug) {
    Serial.print("RSSI%: ");
    Serial.print(map(avgLeft, RSSI_MIN, RSSI_MAX, 0, 100));
    Serial.print(", ");
    Serial.print(map(avgRight, RSSI_MIN, RSSI_MAX, 0, 100));

    // raw rssi values, use these for RSSI_MIN and RSSI_MAX
    //Serial.print("RAW RSSI: ");
    //Serial.print(avgLeft);
    //Serial.print(", ");
    //Serial.print(avgRight);

    Serial.print(" servo: ");
    Serial.print(anglePan);

    Serial.print(" > ");
    Serial.println(ang);
  }
}

void movePanBy(float angle) {

  anglePan += angle;
  anglePan = limit(SERVO_MIN, SERVO_MAX, anglePan);
  servoPan.write(anglePan);
}

void measureRSSI() {

  advanceArray(rssi_left_array, FIR_SIZE);
  advanceArray(rssi_right_array, FIR_SIZE);

  rssi_left_array[LAST] = analogRead(LEFT_RSSI_PIN);
  rssi_right_array[LAST] = analogRead(RIGHT_RSSI_PIN);
}

uint16_t avg(uint16_t samples[], uint8_t n) {

  uint32_t summ = 0;
  for (uint8_t i = 0; i < n; i++) {
    summ += samples[i];
  }

  return uint16_t(summ / n);
}

void advanceArray(uint16_t *samples, uint8_t n) {

  for (uint8_t i = 0; i < n - 1; i++) {
    samples[i] = samples[i + 1];
  }
}


float limit(float lowerLimit, float upperLimit, float var) {

  return min(max(var, lowerLimit), upperLimit);
}

