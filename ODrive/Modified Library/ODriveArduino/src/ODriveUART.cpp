// Author: ODrive Robotics Inc.
// License: MIT
// Documentation: https://docs.odriverobotics.com/v/latest/guides/arduino-uart-guide.html

#include "Arduino.h"
#include "ODriveUART.h"

static const int kMotorNumber = 0;

// Print with stream operator
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }

ODriveUART::ODriveUART(Stream& serial)
    : serial_(serial) {}

void ODriveUART::clearErrors() {
    serial_ << F("sc\n");
}

void ODriveUART::setControlModeTorque(){
    serial_ << F("w axis") << kMotorNumber << ".controller.config.control_mode " << 1 << F("\n");
}

void ODriveUART::setControlModePosition(){
    serial_ << F("w axis") << kMotorNumber << ".controller.config.control_mode " << 3 << F("\n");
}

void ODriveUART::setPosition(float position) {
    setPosition(position, 0.0f, 0.0f);
}

void ODriveUART::setPosition(float position, float velocity_feedforward) {
    setPosition(position, velocity_feedforward, 0.0f);
}

void ODriveUART::setPosition(float position, float velocity_feedforward, float torque_feedforward) {
    serial_ << F("p ") << kMotorNumber  << F(" ") << position << F(" ") << velocity_feedforward << F(" ") << torque_feedforward << F("\n");
}

void ODriveUART::setPositionLimit(float position, float velocity_limit){
    setPositionLimit(position, velocity_limit, 0.0f);
}

void ODriveUART::setPositionLimit(float position, float velocity_limit, float torque_limit){
    serial_ << F("q ") << kMotorNumber << F(" ") << position << F(" ") << velocity_limit << F(" ") << torque_limit << F("\n");
}

void ODriveUART::setVelocity(float velocity) {
    setVelocity(velocity, 0.0f);
}

void ODriveUART::setVelocity(float velocity, float torque_feedforward) {
    serial_ << F("v ") << kMotorNumber  << F(" ") << velocity << F(" ") << torque_feedforward << F("\n");
}

void ODriveUART::setTorque(float torque) {
    serial_ << F("c ") << kMotorNumber << F(" ") << torque << F("\n");
}

void ODriveUART::trapezoidalMove(float position) {
    serial_ << F("t ") << kMotorNumber << F(" ") << position << F("\n");
}

ODriveFeedback ODriveUART::getFeedback() {
    // Flush RX
    while (serial_.available()) {
        serial_.read();
    }

    serial_ << F("f ") << kMotorNumber << F("\n");

    String response = readLine();

    int spacePos = response.indexOf(' ');
    if (spacePos >= 0) {
        return {
            response.substring(0, spacePos).toFloat(),
            response.substring(spacePos+1).toFloat()
        };
    } else {
        return {0.0f, 0.0f};
    }
}

String ODriveUART::getParameterAsString(const String& path) {
    serial_ << F("r ") << path << F("\n");
    return readLine();
}

void ODriveUART::setParameter(const String& path, const String& value) {
    serial_ << F("w ") << path << F(" ") << value << F("\n");
}

void ODriveUART::setState(ODriveAxisState requested_state) {
    setParameter(F("axis0.requested_state"), String((long)requested_state));
}

ODriveAxisState ODriveUART::getState() {
    return (ODriveAxisState)getParameterAsInt(F("axis0.current_state"));
}

String ODriveUART::readLine(unsigned long timeout_ms) {
    String str = "";
    unsigned long timeout_start = millis();
    for (;;) {
        while (!serial_.available()) {
            if (millis() - timeout_start >= timeout_ms) {
                return str;
            }
        }
        char c = serial_.read();
        if (c == '\n')
            break;
        str += c;
    }
    return str;
}
