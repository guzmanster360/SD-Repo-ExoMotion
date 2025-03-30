// VICTOR G.
# include <Servo.h>

Servo myservoL; //create servo object for the left servo in the double motor SEA
Servo myservoR; //create servo object for the right servo in the double motor SEA
Servo myservo1; // create servo object for the single motor SEA

// arduino connections
int potPinL = 0; // define analog pin zero as the data input for the left potentiometer

float servo_max = 140; //mechanical limit of servo (degrees)
int pos = 0; // create a variable for the position of the servos
int i = 0; // for loop variable

//definite parameters
float r_pots = .02; //m
float r_pulley = .034; //m
float r_mots = .02; //m
float l_tip = .039; //m
float k_s = 196; // N/m
float k_joint = 0; //stiffness of the end effector

//initialize variables
float theta_pot_L = 0; // initial angular position of the left potentiometer sensor
float theta_pot_CL = 0; // angular position of the left potentiometer sensor after some event has occurred
float difference_potL = 0; // angular displacement of the left potentiometer
float theta_desired = 0; // desired angular positon of the end effector
float theta_actual_L = 0; //actual position from left sensor data
float theta_actual = 0; // actual angle of the end effector
float dtheta = 0; //change in end effector angular position
float theta_mot_c_real = 0; // angular position of the the motors assuming the servos are mechanically capaable of 180 degree range of motion
float theta_mot_c_write = 0; // angular position of the motors adjusting for the mechanical limit of the servo

float check1 = 0; // Variables used to store the returned value of the angular position sensor in the detection method
float check2 = 0;
float check3 = 0;
float check4 = 0;

//single servo parameters
int potPin1 = 2;

float r_pot1 = .02; //m
float r_pulley1 = 0.022225; //m
float r_mot1 = 0.02; //m
float l_tip1 = .075; //m
float k_s1 = 140; //N/m
float k_joint1 = 0;

float difference_pot1 = 0; // displacement of the angular position sensor in the single motor SEA
float theta_pot_1 = 0; // angular position of the single SEA potentiometer
float theta_mot_1 = 0; // angular position of the single SEA motor
float theta_pot_C1 = 0; // angular position of the single SEA potentiometer after and event has occurred

void setup() {
  // put your setup code here, to run once:
  myservoL.attach(8); // bind the left servo of the double motor SEA to digital pin 8 of the Arduino
  myservoR.attach(9); // bind the left servo of the double motor SEA to digital pin 9 of the Arduino
  myservo1.attach(10); //bind the servo of the single motor SEA to digital pin 10 of the Arduino
  Serial.begin(9600);

  pinMode(8, OUTPUT); // set pins 8 - 10 as data output pins for servos
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  delay(100);

  myservoL.write(0); // set all servos to their 0 position
  myservoR.write(0);
  myservo1.write(0);

  //read pot value of left and right sensors to store an initial value
  theta_pot_L = read_pot_angle(potPinL);
  theta_pot_1 = read_pot_angle(potPin1);

}

void loop() {
//  stiffness();
//  compliance();
//  sweep();
//  detection();
//  single_servo_compliance();

}

// Method for getting the value of the potentiometer and converting it to an angular position
float read_pot_angle(int pot_pin) {

  float val = analogRead(pot_pin);
  float ratio = val / 1023; //the potentiometer value range is 0-1023
  float theta_pot = ratio * 360; //by multiplying the ratio by 360 we can determine the angle
  return theta_pot; // return the angle

}

// Method for creating the transparency mode for the SEA
void compliance() {
  theta_pot_CL = read_pot_angle(potPinL); // retrieve the angle value of the left sensor

  difference_potL = -(theta_pot_CL - theta_pot_L); // calculate the angular displacement of the potentiometer

  Serial.println(difference_potL); // print the angular displacement to the console

  myservoL.write(90 + (difference_potL * (r_pulley / r_pots) * (180 / servo_max))); // move the servos to the desired position
  myservoR.write(90 + (difference_potL * (r_pulley / r_pots) * (180 / servo_max)));

  delay(70);
}

// Method for simulating the stiffness of the end effector
void stiffness() {

  theta_pot_CL = read_pot_angle(potPinL); // retrieve the angle value of the left sensor 

  difference_potL = -(theta_pot_CL - theta_pot_L); // calculate the angular displacement of the potentiometer

  theta_actual = -(difference_potL) * (r_pots / r_pulley); // current angle of end effector 
  theta_desired = 0; // set the desired end effector angle

  //set desired stiffness of joint
  k_joint = 1.5; //.005

  dtheta = theta_actual - theta_desired; // calculate the angular displacement of the end effector

  theta_mot_c_real = ((difference_potL) * (r_pots / r_mots)) - ((k_joint * (dtheta)) / (-2 * k_s * r_pulley * r_mots)); //between -servo_max/2 and servo_max/2
  theta_mot_c_write = (180 / servo_max) * (theta_mot_c_real + servo_max / 2); //between 0 and 180

  myservoL.write(90 + theta_mot_c_write); // move the servos to the desired position
  myservoR.write(90 + theta_mot_c_write);

  delay(70);

}

// Method for applying the sweeping detection mode to the actuator
void detection() {

  for (pos = 0; pos < 180; pos += 9) { // for loop for the sweeping motion of the end effector
    myservoL.write(pos);
    myservoR.write(pos);
    theta_pot_CL = read_pot_angle(potPinL); // read angle and calculate displacement
    difference_potL = -(theta_pot_CL - theta_pot_L);
    Serial.println(difference_potL); 
    
    if ( (pos % 2) == 0) { // check if potentiometer returns same value twice by storing two consecutive values
      check1 = difference_potL;
    }
    else {
      check2 = difference_potL;
    }
    if (check1 == check2 && pos > 70) { // if two consecutive values are the same call the stop end effector method
      Serial.println("STOPPED1");
      stop_servo(pos);
    }
    delay(50);
  }

  // Same functionqlity as above except rotating the end effector in the opposite direction
  for (pos = 180; pos > 0; pos -= 9) {
    myservoL.write(pos);
    myservoR.write(pos);
    theta_pot_CL = read_pot_angle(potPinL);
    difference_potL = -(theta_pot_CL - theta_pot_L);
    Serial.println(difference_potL);
    if ( (pos % 2) == 0) {
      check3 = difference_potL;
    }
    else {
      check4 = difference_potL;
    }
    if (check3 == check4 && pos < 120) {
      Serial.println("STOPPED2");
      stop_servo(pos);
    }
    delay(50);
  }
}

// Method for getting the end effector to do a sweeping motion at the full mechanical range of motion of the end effectors
void sweep() {
  for (pos = 0; pos < 180; pos += 1) {
    myservoL.write(pos);
    myservoR.write(pos);
    delay(10);
  }
  for (pos = 180; pos > 0; pos -= 1) {
    myservoL.write(pos);
    myservoR.write(pos);
    delay(10);
  }
}

// Method to stop servo for 2 seconds
void stop_servo(int pos) {
  myservoL.write(pos);
  myservoR.write(pos);
  delay(2000);
}

// FOR SINGLE SERVO SEA

// Method for creating a transparency mode for the single motor SEA
void single_servo_compliance() {
  theta_pot_C1 = read_pot_angle(potPinL);

  difference_pot1 = -(theta_pot_C1 - theta_pot_1);

  Serial.println(difference_pot1);

  myservo1.write(90 + (difference_pot1 * (r_pulley1 / r_pot1) * (180 / servo_max)));

  delay(70);
}
