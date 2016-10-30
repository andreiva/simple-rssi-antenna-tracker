/*
* Simple RSSI Antenna Tracker
*
*
*
*
*/

#include <Servo.h>
#include "Timer.h"


/*
 * Pin mapping
 * You can change these pins, just make sure that 
 * RSSI pins are analog inputs
 * and servo pin supports PWM
 */
#define LEFT_RSSI_PIN       0     // analog RSSI measurement pin
#define RIGHT_RSSI_PIN      1
#define PAN_SERVO_PIN       5     // Pan servo pin

/*
 * There is going to be some difference in receivers,
 * one is going to be less sensitive than the other.
 * It will result in slight offset when tracker is centered on target
 * 
 * Find value that evens out RSSI 
 */
#define RSSI_OFFSET_RIGHT   0
#define RSSI_OFFSET_LEFT    0

/*
 * Safety limits for servo
 * 
 * Find values where servo doesn't buzz at full deflection
 */
#define SERVO_MAX           170
#define SERVO_MIN           6

/*
 * Center deadband value!
 * Prevents tracker from oscillating when target is in the center
 */
#define DEADBAND            15

/*
 * Depending which way around you put your servo
 * you may have to change direction
 * 
 * either 1 or -1
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

  uint16_t avgLeft = avg(rssi_left_array, FIR_SIZE) + RSSI_OFFSET_LEFT;
  uint16_t avgRight = avg(rssi_right_array, FIR_SIZE) + RSSI_OFFSET_RIGHT;
  float ang = 0;

  // If avg RSSI is above 90%, don't move
  if ((avgRight + avgLeft) / 2 > 360) {
    return;
  }

  // if target is in the middle, don't move
  if (abs(avgRight - avgLeft) < DEADBAND ) {
    return;
  }


  // move towards stronger signal
  if (avgRight > avgLeft) {
    ang = float(avgRight / avgLeft) * (SERVO_DIRECTION * -1);
  }
  else {
    ang = float(avgLeft / avgRight) * (SERVO_DIRECTION);
  }

  // move servo by n degrees
  movePanBy(ang);


  if (debug) {
    Serial.print("RSSI: ");
    Serial.print(map(avgLeft, 120, 400, 0, 100));
    Serial.print(", ");
    Serial.print(map(avgRight, 120, 400, 0, 100));

    Serial.print(" servo: ");
    Serial.print(anglePan);

    Serial.print(" > ");
    Serial.println(ang);
  }
}

void movePanBy(float angle) {

  anglePan += angle;

  if (anglePan >= SERVO_MAX)
    anglePan = SERVO_MAX;
  else if (anglePan <= SERVO_MIN)
    anglePan = SERVO_MIN;

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




