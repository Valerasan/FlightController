#include "pid.h"

float clamp(float value, float limit)
{
    if (value > limit) return limit;
    if (value < -limit) return -limit;
    return value;
}

PIDcontroller::PIDcontroller(float kP, float kI, float kD, float outputLimit, float integralLimit)     
    : _kP(kP), _kI(kI), _kD(kD), _outputLimit(outputLimit), _integralLimit(integralLimit)
{}

float PIDcontroller::update(float target, float current, float dt){

    const float e = target - current;
    float P = e * _kP;

    _integral = clamp(_integral + e * dt, _integralLimit);
    float I = _integral * _kI;

    float derivative = 0;
    if(_hasPrevError && dt > 0.0f) {
        derivative = (e - _e_prev) / dt;
    }
    _hasPrevError = true;
    float D = _kD * derivative;

    _e_prev = e;


    return P + I + D;
}


void PIDcontroller::reset() {
    _integral = 0;
    _e_prev = 0;
    _hasPrevError = false;
}