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

    // Diagnostic: read back CTRL1_XL (accel ODR/FS), CTRL2_G (gyro ODR/FS),
    // and INT1_CTRL (DRDY routing) as raw bytes, to confirm init() actually
    // took effect on the chip rather than assuming it from unchecked SPI
    // writes. 0x00 in ctrl1Xl/ctrl2G means that axis's ODR is OFF (never
    // sampling), and bit0/bit1 of int1Ctrl are int1_drdy_xl/int1_drdy_g.
    bool readDebugRegs(uint8_t &ctrl1Xl, uint8_t &ctrl2G, uint8_t &int1Ctrl);

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