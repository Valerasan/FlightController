#include "attitude_estimator.h"

#include <cmath>

namespace {
constexpr float kRadToDeg = 57.295779513f;
}

void AttitudeEstimator::update(const int16_t accel[3], const int16_t gyro[3], float dtSeconds)
{
    if (dtSeconds <= 0.0f) {
        return;
    }

    if (dtSeconds > 0.5f) {
        dtSeconds = 0.5f;
    }

    const float gx = gyro[0] * kGyroRadPerSecPerLsb * kGyroXSign;
    const float gy = gyro[1] * kGyroRadPerSecPerLsb * kGyroYSign;
    const float gz = gyro[2] * kGyroRadPerSecPerLsb * kGyroZSign;


    const float ax = static_cast<float>(accel[0]);
    const float ay = static_cast<float>(accel[1]);
    const float az = static_cast<float>(accel[2]);

    _elapsedMs += dtSeconds * 1000.0f;

    mahonyUpdate(gx, gy, gz, ax, ay, az, dtSeconds);
}

void AttitudeEstimator::mahonyUpdate(float gx, float gy, float gz,
                                      float ax, float ay, float az,
                                      float dtSeconds)
{
    const float twoKp = (_elapsedMs < kFastConvergeMs) ? kTwoKpFast : kTwoKpRun;
    const float twoKi = (_elapsedMs < kFastConvergeMs) ? kTwoKiFast : kTwoKiRun;

    if (!(ax == 0.0f && ay == 0.0f && az == 0.0f)) {
        float recipNorm = 1.0f / sqrtf(ax * ax + ay * ay + az * az);
        ax *= recipNorm; ay *= recipNorm; az *= recipNorm;

  
        const float halfvx = _q1 * _q3 - _q0 * _q2;
        const float halfvy = _q0 * _q1 + _q2 * _q3;
        const float halfvz = _q0 * _q0 - 0.5f + _q3 * _q3;


        const float halfex = ay * halfvz - az * halfvy;
        const float halfey = az * halfvx - ax * halfvz;
        const float halfez = ax * halfvy - ay * halfvx;

   
        if (twoKi > 0.0f) {
            _integralFBx += twoKi * halfex * dtSeconds;
            _integralFBy += twoKi * halfey * dtSeconds;
            _integralFBz += twoKi * halfez * dtSeconds;
            gx += _integralFBx; gy += _integralFBy; gz += _integralFBz;
        } else {
            _integralFBx = _integralFBy = _integralFBz = 0.0f;  
        }
        gx += twoKp * halfex;
        gy += twoKp * halfey;
        gz += twoKp * halfez;
    }


    gx *= 0.5f * dtSeconds; gy *= 0.5f * dtSeconds; gz *= 0.5f * dtSeconds;
    const float qa = _q0, qb = _q1, qc = _q2;
    _q0 += (-qb * gx - qc * gy - _q3 * gz);
    _q1 += ( qa * gx + qc * gz - _q3 * gy);
    _q2 += ( qa * gy - qb * gz + _q3 * gx);
    _q3 += ( qa * gz + qb * gy - qc * gx);


    float recipNorm = 1.0f / sqrtf(_q0*_q0 + _q1*_q1 + _q2*_q2 + _q3*_q3);
    _q0 *= recipNorm; _q1 *= recipNorm; _q2 *= recipNorm; _q3 *= recipNorm;
}

float AttitudeEstimator::rollDeg() const
{
    return atan2f(2.0f * (_q0 * _q1 + _q2 * _q3),
                  1.0f - 2.0f * (_q1 * _q1 + _q2 * _q2)) * kRadToDeg;
}

float AttitudeEstimator::pitchDeg() const
{
    float sinp = 2.0f * (_q0 * _q2 - _q3 * _q1);
    if (sinp > 1.0f) sinp = 1.0f;
    if (sinp < -1.0f) sinp = -1.0f;   
    return asinf(sinp) * kRadToDeg;
}
