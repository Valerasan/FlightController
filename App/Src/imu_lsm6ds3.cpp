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
    _dmaTxBuf[0] = LSM6DS3_OUTX_L_G | 0x80;
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

    lsm6ds3_int1_route_t val = {0};
    val.int1_drdy_g = 1;
    val.int1_drdy_xl = 1;
    lsm6ds3_pin_int1_route_set(&_ctx, &val);

    return true;
}

bool ImuLsm6ds3::readDebugRegs(uint8_t &ctrl1Xl, uint8_t &ctrl2G, uint8_t &int1Ctrl)
{
    return readReg(this, LSM6DS3_CTRL1_XL, &ctrl1Xl, 1) == 0 &&
           readReg(this, LSM6DS3_CTRL2_G, &ctrl2G, 1) == 0 &&
           readReg(this, LSM6DS3_INT1_CTRL, &int1Ctrl, 1) == 0;
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

bool ImuLsm6ds3::startReadRawDma()
{
    if (_dmaBusy) {
        return false;
    }
    _dmaBusy = true;

    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_RESET);
    if (HAL_SPI_TransmitReceive_DMA(_hspi, _dmaTxBuf, _dmaRxBuf, sizeof(_dmaTxBuf)) != HAL_OK) {
        HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
        _dmaBusy = false;
        return false;
    }
    return true;
}

void ImuLsm6ds3::onDmaComplete()
{
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);

    int16_t gyro[3];
    int16_t accel[3];
    gyro[0]  = static_cast<int16_t>((_dmaRxBuf[2]  << 8) | _dmaRxBuf[1]);
    gyro[1]  = static_cast<int16_t>((_dmaRxBuf[4]  << 8) | _dmaRxBuf[3]);
    gyro[2]  = static_cast<int16_t>((_dmaRxBuf[6]  << 8) | _dmaRxBuf[5]);
    accel[0] = static_cast<int16_t>((_dmaRxBuf[8]  << 8) | _dmaRxBuf[7]);
    accel[1] = static_cast<int16_t>((_dmaRxBuf[10] << 8) | _dmaRxBuf[9]);
    accel[2] = static_cast<int16_t>((_dmaRxBuf[12] << 8) | _dmaRxBuf[11]);

    if (_sampleReadyCb) {
        _sampleReadyCb(accel, gyro);
    }
    _dmaBusy = false;
}

void ImuLsm6ds3::onDmaError()
{
    HAL_GPIO_WritePin(_csPort, _csPin, GPIO_PIN_SET);
    _dmaBusy = false;
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