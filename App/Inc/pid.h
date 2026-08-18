#ifndef PID_H
#define PID_H


class PIDcontroller {
public:

    PIDcontroller(float kP, float kI, float kD, float outputLimit, float integralLimit);


    float update(float target, float current, float dt);

    void reset();

private:


    float _kP = 0;
    float _kI = 0;
    float _kD = 0;

    float _outputLimit;
    float _integralLimit;

    float _integral = 0;
    float _e_prev = 0;
    bool _hasPrevError = false;
};


#endif