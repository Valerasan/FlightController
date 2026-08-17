#include "flight_state.h"

#include "stm32f4xx_hal.h"  

FlightState _flightState;

void FlightState::setAttitude(float rollDeg, float pitchDeg)
{
    __disable_irq();
    _attitude.rollDeg = rollDeg;
    _attitude.pitchDeg = pitchDeg;
    __enable_irq();
}

void FlightState::setImuSample(const int16_t accel[3], const int16_t gyro[3])
{
    __disable_irq();
    _imuSample.accel[0] = accel[0];
    _imuSample.accel[1] = accel[1];
    _imuSample.accel[2] = accel[2];
    _imuSample.gyro[0] = gyro[0];
    _imuSample.gyro[1] = gyro[1];
    _imuSample.gyro[2] = gyro[2];
    __enable_irq();
}

void FlightState::incrementImuIntCount()
{
    _imuIntCount++;
}

void FlightState::setChannelsUs(const uint16_t channelsUs[kChannelCount])
{
    __disable_irq();
    for (uint8_t i = 0; i < kChannelCount; i++) {
        _channelsUs[i] = channelsUs[i];
    }
    __enable_irq();
}

AttitudeSample FlightState::attitude() const
{
    __disable_irq();
    const AttitudeSample copy = _attitude;
    __enable_irq();
    return copy;
}

ImuSample FlightState::imuSample() const
{
    __disable_irq();
    const ImuSample copy = _imuSample;
    __enable_irq();
    return copy;
}

uint32_t FlightState::imuIntCount() const
{
    return _imuIntCount;
}

void FlightState::channelsUs(uint16_t outChannelsUs[kChannelCount]) const
{
    __disable_irq();
    for (uint8_t i = 0; i < kChannelCount; i++) {
        outChannelsUs[i] = _channelsUs[i];
    }
    __enable_irq();
}