#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define EN1 9
#define IN1 8
#define IN2 7
#define POT A3
#define ENCA A1
#define ENCB A2
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-abcdefabcdef"

BLECharacteristic *pCharacteristic;

// PID Gains (Adjust for best performance)
double Kp = 2.0;
double Ki = 0.5;
double Kd = 0.1;

float integral = 0, lastError = 0;
unsigned long lastTime = 0;

// parameters
double motor_CPR = 48.0;
double Lever_Arm = 0.2159;            //m
double end_Effector_D = 0.0635;       //m
double pot_D = 0.04;                  //m
double mot_D = 0.0315;                  //m
double springConstant = 59.86002275;  // N/m (adjust based on real spring)

double start_pos = 0;
double rotation = 0;

// void readEncoder() {
//   if (digitalRead(ENCA) == digitalRead(ENCB)) {
//     encoderCount++;
//   } else {
//     encoderCount--;
//   }
// }

// Send command to motor
void driveMotor(double pwmValue, double springDeflection) {
  int pwm = constrain(abs(pwmValue), 0, 255);  // Limit PWM between 0-255

  if (springDeflection > 0) {
    analogWrite(EN1, pwm);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    analogWrite(EN1, pwm);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(POT, INPUT);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  BLEDevice::init("ESP32_BLE");

    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

    pCharacteristic->setValue("Hello iPhone!");
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

  Serial.println("BLE Server Started");
  
}
void loop() {

  // Read spring deflection
  double rawSpringReading = analogRead(POT);
  double adjust = map(rawSpringReading, 0, 1023, -511, 511);
  // Convert to meters (accounting for pulley scaling)
  double springDeflection = (adjust / 511) * (pot_D);

  // Compute force using Hooke's Law
  double force = springConstant * springDeflection;  // Newtons

  // Compute torque at the end-effector
  double torque = force * Lever_Arm;  // Newton-meters

  // Convert torque to motor PWM signal
  double pwmValue = torqueToPWM(torque);
  pCharacteristic->setValue(torque); // Send updated data
  pCharacteristic->notify();  // Notify connected device

  // Drive motor
  driveMotor(pwmValue, springDeflection);

  // Debug output
  Serial.print("Spring Deflection (m): ");
  Serial.print(springDeflection);
  Serial.print(" | Force (N): ");
  Serial.print(force);
  Serial.print(" | Torque (Nm): ");
  Serial.print(torque);
  Serial.print(" | PWM: ");
  Serial.println(pwmValue);

  delay(10);
}

// Convert Torque to PWM Signal
double torqueToPWM(double torque) {
  double maxTorque = 0.1;  // Adjust as needed
  int pwm = map(abs(torque) * 1000, 0, maxTorque * 1000, 0, 255);
  return constrain(pwm, 0, 255);
}


void setup() {
    Serial.begin(115200);
}

void loop() {
    delay(2000);
    pCharacteristic->setValue("Data: " + String(millis())); // Send updated data
    pCharacteristic->notify();  // Notify connected device
}