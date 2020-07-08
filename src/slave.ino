#ifdef SLV
#include "Encoder.h"
#include <PIDController.h>
#include <Servo.h>
#include <SparkFun_TB6612.h>
#include <Wire.h>
#include <trajfactory.h>

// Address of slave.
#define SLAVE_ADDR 8

// Pin definitions.
#define SERVO_PIN 9

// Limits of servo.
#define SERVO_MIN 2400
#define SERVO_MAX 550

// Init trajectory generation and current pointer.
TrajFactory tf = TrajFactory();
Trajectory *traj_ptr = 0;

// Init current ventilator params.
int rr;
float ie;
int setpoint;
float hold;
int delta_t;
char state;

// Init motor params.
#define AIN1 5
#define AIN2 4
#define PWMA 11

#define STBY 9

#define BIN1 7
#define BIN2 8
#define PWMB 6

const int offsetA = 1;
const int offsetB = 1;

Motor motorLeft = Motor(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motorRight = Motor(BIN1, BIN2, PWMB, offsetB, STBY);

Encoder leftEnc(2, 3);
Encoder rightEnc(18, 19);

PIDController pidLeft;
PIDController pidRight;

double p = 2;
double i = 0.5;
double d = 0;

double oldLeft = -999;
double oldRight = -999;

void setup() {

  // Join i2c bus with address SLAVE_ADDR.
  Wire.begin(SLAVE_ADDR);

  // Register callback function for i2c receive.
  Wire.onReceive(recieveTraj);

  // Start serial output for trajectory monitoring.
  Serial.begin(9600);

  // Start PID controllers.
  pidLeft.begin();
  pidLeft.tune(p, i, d);
  pidLeft.limit(-255, 255);

  pidRight.begin();
  pidRight.tune(p, i, d);
  pidRight.limit(-255, 255);

  // Set state to off.
  state = 'X';
}

void loop() {
  // Run device in different modes.
  switch (state) {
  // Device is on, continue following trajectory.
  case 'O':
    moveTo(traj_ptr->nextStep(), traj_ptr->getDeltaTime());
    break;

  // Device has been instructed to shutdown.
  case 'X':
    stop();
    break;

  // Device has recieved a new trajectory, load and start.
  case 'L':

    // Stop motion.
    stop();

    // If assigned ptr, delete contents
    if (traj_ptr != 0) {
      delete traj_ptr;
    }

    // Build new trajectory
    traj_ptr = tf.build(rr, ie, setpoint, hold, delta_t);

    // Set device on.
    state = 'O';
    break;
  }
}

void moveTo(int pos, int delta_t) {

  pidLeft.setpoint(pos);
  pidRight.setpoint(pos);

  unsigned long startedWaiting = millis();
  while (millis() - startedWaiting <= delta_t) {

    double newLeft = leftEnc.read();
    if (newLeft != oldLeft) {
      oldLeft = newLeft;
    }

    double newRight = rightEnc.read();
    if (newRight != oldRight) {
      oldRight = newRight;
    }

    Serial.print(oldRight);
    Serial.print(" ");
    Serial.println(pos);

    double setLeft = pidLeft.compute(oldLeft);
    motorLeft.drive(setLeft);

    double setRight = pidRight.compute(oldRight);
    motorRight.drive(setRight);
  }
}

void stop() {

  // If we have already generated a trajectory, follow it till the end.
  if (traj_ptr != 0) {
    while (traj_ptr->getCurrentStep() != 0) {
      moveTo(traj_ptr->nextStep(), traj_ptr->getDeltaTime());
    }
    // If not, got to 0 position.
  } else {
    moveTo(0, 100);
  }
}

void recieveTraj(int num_entries) {

  // Read the state from incoming transmission.
  char recieved_state = Wire.read();

  // Either change machine state or load new settings.
  switch (recieved_state) {
  case 'X':
    state = 'X';
    break;
  case 'L':
    state = 'L';

    // Load new params
    byte high = Wire.read();
    byte low = Wire.read();
    setpoint = (high << 8) | low;

    rr = Wire.read();

    byte inhale = Wire.read();
    byte exhale = Wire.read();
    ie = float(inhale) / exhale;

    byte hold_s = Wire.read();
    byte hold_dec = Wire.read();
    hold = hold_s + hold_dec / 100.0;

    delta_t = Wire.read();

    break;
  }
}

#endif
