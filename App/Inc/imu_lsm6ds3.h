#ifndef IMU_LSM6DS3_H
#define IMU_LSM6DS3_H

#include "stm32f4xx_hal.h"
#include "lsm6ds3_reg.h"

class ImuLsm6ds3 {
public:
    ImuLsm6ds3(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort, uint16_t csPin);

    bool init();
    bool readRaw(int16_t accel[3], int16_t gyro[3]);

    uint8_t whoAmI() const { return _whoAmI; }

private:
    static int32_t writeReg(void *handle, uint8_t reg, const uint8_t *data, uint16_t len);
    static int32_t readReg(void *handle, uint8_t reg, uint8_t *data, uint16_t len);

    SPI_HandleTypeDef *_hspi;
    GPIO_TypeDef *_csPort;
    uint16_t _csPin;
    stmdev_ctx_t _ctx;
    uint8_t _whoAmI = 0;
};

#endif // IMU_LSM6DS3_H