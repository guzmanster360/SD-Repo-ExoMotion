// Libraries
#include <ODriveUART.h>
#include <SoftwareSerial.h>
#include <AS5600.h>
#include <Wire.h>

// Hardware pins
#define EN1 9
#define IN1 8
#define IN2 7

SoftwareSerial odrive_serial(10, 11);
unsigned long baudrate = 19200;  // Must match what you configure on the ODrive (see docs for details)

float ROM = 250;  // max ROM fro SEA

// PID Gains (Adjust for best performance)
double Kp = 10.0;
double Ki = 0.5;
double Kd = 0.1;

float integral = 0, lastError = 0;
unsigned long lastTime = 0;

// parameters
double Lever_Arm = 0.31590;            //m
double end_Effector_D = 0.10955;       //m
double mot_D = 0.04676;                //m
double springConstant = 8388.5753965;  // N/m (adjust based on real spring)
double drive_ratio = end_Effector_D / mot_D;

double start_pos = 0;
double rotation = 0;

// Control Parameters (Tune These)
const float k_s = springConstant * ((end_Effector_D / 2) * (end_Effector_D / 2));  // Spring stiffness constant (Nm/rad)
const float k_d = 2.0;                                                             // Desired stiffness (Nm/rad)
const float b_d = 0.5;                                                             // Desired damping (Nm·s/rad)

// Sensor Objects
AS5600 encoder;
ODriveUART odrive(odrive_serial);
float theta_m, theta_l, torque_estimate, torque_command, impedance_torque, total_torque;

void setup() {
  odrive_serial.begin(baudrate);
  Serial.begin(115200);

  Wire.begin();  // Initialize I²C

  if (encoder.begin()) {
    Serial.println("AS5600 found!");
  } else {
    Serial.println("AS5600 NOT detected. Check wiring.");
  }

  delay(10);

  Serial.println("Waiting for ODrive...");
  while (odrive.getState() == AXIS_STATE_UNDEFINED) {
    delay(100);
  }

  Serial.println("found ODrive");

  Serial.print("DC voltage: ");
  Serial.println(odrive.getParameterAsFloat("vbus_voltage"));

  Serial.println("Enabling closed loop control...");
  odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);

  // while (odrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL) {
  //   odrive.clearErrors();
  //   odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
  //   delay(10);
  // }

  Serial.println("ODrive running!");

  zero_motor();
}

void loop() {
  // Serial.println(get_EF_angle_rad());
  transparency();
  delay(5);
}

void compliance() {
  // Read Motor Position from AS5600
  theta_l = get_EF_angle_rad() - PI;   // Convert to radians
  theta_m = get_MOT_angle_rad() - PI;  //radians

  // Compute Torque from Spring Deflection
  torque_estimate = k_s * (theta_m - theta_l);

  // Apply Torque Feedforward to Cancel Spring Effect
  torque_command = -torque_estimate;

  // Add Virtual Impedance Control
  impedance_torque = k_d * (theta_l) + b_d * (theta_m - theta_l);
  total_torque = torque_command + impedance_torque;

  // Send Torque Command to Motor
  drive_Motor(total_torque);

  // Print Debug Data
  Serial.print("Theta_m: ");
  Serial.print(theta_m, 3);
  Serial.print(" | Theta_l: ");
  Serial.print(theta_l, 3);
  Serial.print(" | Torque: ");
  Serial.print(total_torque, 3);
  Serial.println(" Nm");

  delay(10);
}

void pos_control(int angle) {
}

void drive_Motor(float torque) {
  odrive.setPosition(
    0,    // position
    0.01  // velocity feedforward (optional)
  );
}

void transparency() {
  float EF_angle = get_EF_angle_rad();
  // Serial.println((EF_angle * drive_ratio) / (2 * PI));
  odrive.setPositionLimit(
    (EF_angle * drive_ratio) / (2 * PI),
    5,
    1 * EF_angle  // position
  );
}

// Method for getting the value of the potentiometer and converting it to an angular position
float get_EF_angle_rad() {
  float raw_angle = encoder.getCumulativePosition();  // 0 - 4095 (12-bit)
  // return raw_angle;
  Serial.println(raw_angle);
  return raw_angle * (2 * PI / 4095.0);  // Convert to radians
}

float get_MOT_angle_rad() {
  float raw_angle = encoder.rawAngle();  // 0 - 4095 (12-bit)
  return raw_angle * (2 * PI / 4095.0);  // Convert to radians
}

float get_EF_angle_deg() {
  float raw_angle = encoder.rawAngle();  // 0 - 4095 (12-bit)
  return raw_angle * (360 / 4095.0);     // Convert to degrees
}

float get_MOT_angle_deg() {
  ODriveFeedback feedback = odrive.getFeedback();
  return feedback.pos;
}

void stop_Motor() {
  analogWrite(EN1, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void zero_motor() {
  odrive.setPosition(
    0.5,
    0.05);
}

void check_rom() {
  if (get_MOT_angle_rad() > (180 + (ROM / 2)) || get_MOT_angle_rad() < (180 - (ROM / 2))) {
    stop_Motor();
  }
}