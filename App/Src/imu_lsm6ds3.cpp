#include "imu_lsm6ds3.h"

#include <cstring>

namespace {
constexpr uint32_t kSpiTimeoutMs = 10;
constexpr size_t kMaxSpiPayload = 16; 
}

ImuLsm6ds3::ImuLsm6ds3(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort, uint16_t csPin)
    : _hspi(hspi), _csPort(csPort), _csPin(csPin)
{
    _ctx.write_reg = writeReg;
    _ctx.read_reg = readReg;
    _ctx.mdelay = nullptr;
    _ctx.handle = this;
}

bool ImuLsm6ds3::init()
{
    if (lsm6ds3_device_id_get(&_ctx, &_whoAmI) != 0 || _whoAmI != LSM6DS3_ID) {
        return false;
    }

    lsm6ds3_block_data_update_set(&_ctx, PROPERTY_ENABLE);
    lsm6ds3_xl_full_scale_set(&_ctx, LSM6DS3_4g);
    lsm6ds3_gy_full_scale_set(&_ctx, LSM6DS3_2000dps);
    lsm6ds3_xl_data_rate_set(&_ctx, LSM6DS3_XL_ODR_208Hz);
    lsm6ds3_gy_data_rate_set(&_ctx, LSM6DS3_GY_ODR_208Hz);

    return true;
}

bool ImuLsm6ds3::readRaw(int16_t accel[3], int16_t gyro[3])
{
    if (lsm6ds3_acceleration_raw_get(&_ctx, accel) != 0) {
        return false;
    }
    if (lsm6ds3_angular_rate_raw_get(&_ctx, gyro) != 0) {
        return false;
    }
    return true;
}

int32_t ImuLsm6ds3::writeReg(void *handle, uint8_t reg, const uint8_t *data, uint16_t len)
{
    auto *self = static_cast<ImuLsm6ds3 *>(handle);

    if (len + 1u > kMaxSpiPayload) {
        return -1;
    }

    uint8_t txBuf[kMaxSpiPayload];
    txBuf[0] = reg & 0x7F;  // bit7=0 -> write
    memcpy(txBuf + 1, data, len);

    HAL_GPIO_WritePin(self->_csPort, self->_csPin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(self->_hspi, txBuf, len + 1, kSpiTimeoutMs);
    HAL_GPIO_WritePin(self->_csPort, self->_csPin, GPIO_PIN_SET);

    return status == HAL_OK ? 0 : -1;
}

int32_t ImuLsm6ds3::readReg(void *handle, uint8_t reg, uint8_t *data, uint16_t len)
{
    auto *self = static_cast<ImuLsm6ds3 *>(handle);

    uint8_t txBuf[17] = {0};
    uint8_t rxBuf[17] = {0};

    txBuf[0] = reg | 0x80; 

    HAL_GPIO_WritePin(self->_csPort, self->_csPin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(self->_hspi, txBuf, rxBuf, len + 1, kSpiTimeoutMs);
    HAL_GPIO_WritePin(self->_csPort, self->_csPin, GPIO_PIN_SET);

    if (status == HAL_OK) {
        memcpy(data, rxBuf + 1, len);
        return 0;
    }
    return -1;
}