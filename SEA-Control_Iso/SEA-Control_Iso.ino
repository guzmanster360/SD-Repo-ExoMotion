// Libraries
#include <AS5600.h>
#include <Wire.h>

// Hardware pins
#define EN1 9
#define IN1 8
#define IN2 7

#define POT A0

float ROM = 250;  // max ROM fro SEA

// PID Gains (Adjust for best performance)
double Kp = 10.0;
double Ki = 0.5;
double Kd = 0.1;

float integral = 0, lastError = 0;
unsigned long lastTime = 0;

// parameters
double Lever_Arm = 0.31590;           //m
double end_Effector_D = 0.0635;       //m
double pot_D = 0.04;                  //m
double mot_D = 0.0315;                //m
double springConstant = 59.86002275;  // N/m (adjust based on real spring)
double drive_ratio = end_Effector_D / mot_D;

double start_pos = 0;
double rotation = 0;

// Control Parameters (Tune These)
const float k_s = springConstant * ((end_Effector_D / 2) * (end_Effector_D / 2));  // Spring stiffness constant (Nm/rad)
const float k_d = 2.0;                                                             // Desired stiffness (Nm/rad)
const float b_d = 0.5;                                                             // Desired damping (Nm·s/rad)

// Sensor Objects
AS5600 encoder;
float theta_m, theta_l, torque_estimate, torque_command, impedance_torque, total_torque;

void setup() {
  Serial.begin(115200);

  Wire.begin();  // Initialize I²C

  pinMode(POT, INPUT);

  if (encoder.begin()) {
    Serial.println("AS5600 found!");
  } else {
    Serial.println("AS5600 NOT detected. Check wiring.");
  }
  zero_motor();
}

void loop() {
  transparency();
}



void compliance() {
  // Read Motor Position from AS5600
  theta_m = get_encoder_angle_rad() - PI;  //radians
  theta_l = get_pot_angle_rad() - PI;      // Convert to radians

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
  int pos_mot_angle = get_encoder_angle_deg() - 180;
  while (pos_mot_angle != angle) { // maybe GIVE ANGLE TOLERANCE OF +/- 1
    if (pos_mot_angle < angle) {
      analogWrite(EN1, 225);
      digitalWrite(IN1, LOW);  // RIGHT
      digitalWrite(IN2, HIGH);
    } else {
      analogWrite(EN1, 225);
      digitalWrite(IN1, HIGH);  // LEFT
      digitalWrite(IN2, LOW);
    }
    pos_mot_angle = get_encoder_angle_deg() - 180;
    Serial.print("MOT: ");
    Serial.print(pos_mot_angle);
    Serial.print("  POT: ");
    Serial.println(angle);
  }
}

void drive_Motor(float torque) {
  int pwm_value = map(abs(torque) * 1000, 0, 5000, 50, 255);  // Scale torque to PWM range
  if (torque > 0) {
    analogWrite(EN1, pwm_value);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    analogWrite(EN1, pwm_value);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
}

void transparency() {
  int pot_angle = get_pot_angle_deg() - 180;
  pos_control(pot_angle / drive_ratio);
}

// Method for getting the value of the potentiometer and converting it to an angular position
float get_pot_angle_rad() {
  float val = analogRead(POT);
  return val * (2 * PI / 1023.0);  // Convert to radians
}

float get_encoder_angle_rad() {
  float raw_angle = encoder.rawAngle();  // 0 - 4095 (12-bit)
  return raw_angle * (2 * PI / 4095.0);  // Convert to radians
}

float get_pot_angle_deg() {
  float val = analogRead(POT);
  return val * (360 / 1023.0);  // Convert to degrees
}

float get_encoder_angle_deg() {
  float raw_angle = encoder.rawAngle();  // 0 - 4095 (12-bit)
  return raw_angle * (360 / 4095.0);     // Convert to degrees
}

void stop_Motor() {
  analogWrite(EN1, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void zero_motor() {
  // slow speed
  float cal_speed = 200;
  float mot_angle = int(get_encoder_angle_deg());
  float pot_angle = int(get_pot_angle_deg());

  while (mot_angle != 180) {
    Serial.println(mot_angle);
    if (mot_angle <= 360 && mot_angle > 180) {
      analogWrite(EN1, cal_speed);
      digitalWrite(IN1, HIGH);  // RIGHT
      digitalWrite(IN2, LOW);
    } else {
      analogWrite(EN1, cal_speed);
      digitalWrite(IN1, LOW);  // LEFT
      digitalWrite(IN2, HIGH);
    }
    mot_angle = int(get_encoder_angle_deg());
  }
  stop_Motor();
  delay(50);
  while (pot_angle != 180) {
    pot_angle = int(get_pot_angle_deg());
    Serial.print("ZERO ENCODER: ");
    Serial.println(pot_angle);
  }

  Serial.print(mot_angle);
  Serial.print("  ");
  Serial.print(pot_angle);
  delay(5000);
}

void check_rom() {
  if (get_encoder_angle_rad() > (180 + (ROM / 2)) || get_encoder_angle_rad() < (180 - (ROM / 2))) {
    stop_Motor();
  }
}