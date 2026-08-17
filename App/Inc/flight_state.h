#ifndef FLIGHT_STATE_H
#define FLIGHT_STATE_H

#include <cstdint>

struct AttitudeSample {
    float rollDeg;
    float pitchDeg;
};

struct ImuSample {
    int16_t accel[3];
    int16_t gyro[3];
};

class FlightState {
public:
    static constexpr uint8_t kChannelCount = 8;

    FlightState() = default;

    void setAttitude(float rollDeg, float pitchDeg);
    void setImuSample(const int16_t accel[3], const int16_t gyro[3]);

    void incrementImuIntCount();

    void setChannelsUs(const uint16_t channelsUs[kChannelCount]);

    AttitudeSample attitude() const;
    ImuSample imuSample() const;
    uint32_t imuIntCount() const;
    void channelsUs(uint16_t outChannelsUs[kChannelCount]) const;

private:
    AttitudeSample _attitude = {0.0f, 0.0f};
    ImuSample _imuSample = {{0, 0, 0}, {0, 0, 0}};
    volatile uint32_t _imuIntCount = 0;
    uint16_t _channelsUs[kChannelCount] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
};

extern FlightState _flightState;

#endif // FLIGHT_STATE_H