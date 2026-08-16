#ifndef ATTITUDE_ESTIMATOR_H
#define ATTITUDE_ESTIMATOR_H

#include <cstdint>

class AttitudeEstimator {
public:
    AttitudeEstimator() = default;

    void update(const int16_t accel[3], const int16_t gyro[3], float dtSeconds);

    float rollDeg() const;
    float pitchDeg() const;

private:
    void mahonyUpdate(float gx, float gy, float gz,
                       float ax, float ay, float az, float dtSeconds);

    // LSM6DS3 @ +/-2000 dps: 70 mdps/LSB = 0.07 dps/LSB = 0.0012217305 rad/s/LSB.
    static constexpr float kGyroRadPerSecPerLsb = 0.0012217305f;

    static constexpr float kGyroXSign = 1.0f;
    static constexpr float kGyroYSign = 1.0f;
    static constexpr float kGyroZSign = 1.0f;


    static constexpr float kTwoKpFast = 5.0f;
    static constexpr float kTwoKiFast = 0.0f;
    static constexpr float kTwoKpRun  = 0.4f;
    static constexpr float kTwoKiRun  = 0.02f;
    static constexpr float kFastConvergeMs = 20000.0f;

    float _q0 = 1.0f, _q1 = 0.0f, _q2 = 0.0f, _q3 = 0.0f;   
    float _integralFBx = 0.0f, _integralFBy = 0.0f, _integralFBz = 0.0f;

    float _elapsedMs = 0.0f;
};

#endif // ATTITUDE_ESTIMATOR_H
