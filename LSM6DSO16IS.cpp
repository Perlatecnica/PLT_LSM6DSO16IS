/*
MIT License

Copyright (c) [2024] 
Organization: Perlatecnica APS ETS
Author: Mauro D'Angelo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "LSM6DSO16IS.h"
#include <cstdint>


// Costruttore privato per I2C
LSM6DSO16IS::LSM6DSO16IS(PinName sda, PinName scl) {
    i2c = new I2C(sda, scl);
    initialize();
}

// Costruttore privato per SPI
LSM6DSO16IS::LSM6DSO16IS(PinName mosi, PinName miso, PinName sck, PinName cs) {
    // Inizializzazione del sensore tramite SPI
    initialize();
}

// Distruttore privato
LSM6DSO16IS::~LSM6DSO16IS() {
    // Pulizia delle risorse, se necessario
    delete i2c;
    delete spi;
    delete cs_pin;
}

void LSM6DSO16IS::initialize() {
    // Configurazione del sensore
    if (i2c) {
        i2c->frequency(400000); // Set I2C frequency to 400kHz
    } else if (spi) {
        // Configurazione specifica per SPI
        cs_pin->write(0); // Attiva il chip select se necessario
        // Scrivi la configurazione iniziale ai registri del sensore
        cs_pin->write(1); // Disattiva il chip select se necessario
    }
}

LSM6DSO16ISStatusTypeDef LSM6DSO16IS::begin(void)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  if (spi) {
    // Configure CS pin
    //pinMode(cs_pin, OUTPUT);
    //digitalWrite(cs_pin, HIGH);
  }
  /* Enable register address automatically incremented during a multiple byte
  access with a serial interface. */
  if (auto_increment_set(PROPERTY_ENABLE) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Enable BDU */
  if (block_data_update_set(PROPERTY_ENABLE) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Select default output data rate. */
  X_Last_ODR = 104;

  /* Output data rate selection - power down. */
  if (xl_data_rate_set(LSM6DSO16IS_XL_ODR_OFF) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Full scale selection. */
  if (xl_full_scale_set(LSM6DSO16IS_2g) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Select default output data rate. */
  G_Last_ODR = 104;

  /* Output data rate selection - power down. */
  if (gy_data_rate_set(LSM6DSO16IS_GY_ODR_OFF) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Full scale selection. */
  if (gy_full_scale_set(LSM6DSO16IS_2000dps) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  if (ret == LSM6DSO16IS_STATUS_OK) {
    isInitialized = 1;
  }

  return ret;
}


bool LSM6DSO16IS::isConnected() {
    uint8_t who_am_i;
    ReadID(&who_am_i);
    return (who_am_i == LSM6DSO16IS_ID); // Who Am I register value of LSM6DSO16IS
}

void LSM6DSO16IS::set_SDO_SAO_TO_GND(){
    LSM6DSO16IS::lsm6ds01tis_8bit_address = 0x0B << 1;
}

void LSM6DSO16IS::set_SDO_SAO_TO_VCC(){
    LSM6DSO16IS::lsm6ds01tis_8bit_address = 0x0A << 1;
}

bool LSM6DSO16IS::readRegister(uint8_t reg, uint8_t *value, uint16_t len) {
    if (i2c->write(lsm6ds01tis_8bit_address, (const char*)&reg, 1) != 0)
        return 1;
    if (i2c->read(lsm6ds01tis_8bit_address, (char*) value, len) != 0)
        return 1;

    bool ret = 0;
    return ret;
}

bool LSM6DSO16IS::writeRegister(uint8_t reg, const uint8_t *value, uint16_t len) {
    uint8_t data[len + 1];
    data[0] = reg; // inserisci il byte di registro nella prima posizione

    // copia i dati da value a data a partire dalla seconda posizione
    for (uint16_t i = 0; i < len; ++i) {
        data[i + 1] = value[i];
    }

    // scrivi il registro con i dati al bus I2C
    if (i2c->write(lsm6ds01tis_8bit_address, (const char*)data, len + 1) != 0)
        return 1;
    return 0;
}

LSM6DSO16ISStatusTypeDef LSM6DSO16IS::ReadID(uint8_t *val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  if(!readRegister( LSM6DSO16IS_WHO_AM_I, (uint8_t *)val, 1)){
        ret = LSM6DSO16IS_STATUS_ERROR;
  }
  return ret;
}

LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Enable_X(void)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_data_rate_t new_odr;

  /* Check if the component is already enabled */
  if (X_isEnabled == 1U) {
    ret = LSM6DSO16IS_STATUS_OK;
  } else {
    new_odr = (X_Last_ODR <=   12.5f) ? LSM6DSO16IS_XL_ODR_AT_12Hz5_HP
              : (X_Last_ODR <=   26.0f) ? LSM6DSO16IS_XL_ODR_AT_26H_HP
              : (X_Last_ODR <=   52.0f) ? LSM6DSO16IS_XL_ODR_AT_52Hz_HP
              : (X_Last_ODR <=  104.0f) ? LSM6DSO16IS_XL_ODR_AT_104Hz_HP
              : (X_Last_ODR <=  208.0f) ? LSM6DSO16IS_XL_ODR_AT_208Hz_HP
              : (X_Last_ODR <=  416.0f) ? LSM6DSO16IS_XL_ODR_AT_416Hz_HP
              : (X_Last_ODR <=  833.0f) ? LSM6DSO16IS_XL_ODR_AT_833Hz_HP
              : (X_Last_ODR <= 1667.0f) ? LSM6DSO16IS_XL_ODR_AT_1667Hz_HP
              : (X_Last_ODR <= 3333.0f) ? LSM6DSO16IS_XL_ODR_AT_3333Hz_HP
              :                    LSM6DSO16IS_XL_ODR_AT_6667Hz_HP;

    /* Output data rate selection. */
    if (xl_data_rate_set(new_odr) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }

    X_isEnabled = 1;
  }
  return ret;
}

/**
  * @brief  Disable the LSM6DSO16IS accelerometer sensor
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Disable_X(void)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;

  /* Check if the component is already disabled */
  if (X_isEnabled == 0U) {
    ret = LSM6DSO16IS_STATUS_OK;
  } else {
    /* Get current output data rate. */
    if (Get_X_ODR(&X_Last_ODR) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }
    /* Output data rate selection - power down. */
    if (xl_data_rate_set(LSM6DSO16IS_XL_ODR_OFF) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }

    X_isEnabled = 0;
  }

  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS accelerometer sensor output data rate
  * @param  Odr the output data rate value to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_ODR(float_t Odr)
{
  LSM6DSO16ISStatusTypeDef ret;

  /* Check if the component is enabled */
  if (X_isEnabled == 1U) {
    ret = Set_X_ODR_When_Enabled(Odr);
  } else {
    ret = Set_X_ODR_When_Disabled(Odr);
  }

  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS accelerometer sensor full scale
  * @param  FullScale pointer where the full scale is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_FS(int32_t *FullScale)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_full_scale_t fs_low_level;

  /* Read actual full scale selection from sensor. */
  if (xl_full_scale_get(&fs_low_level) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  switch (fs_low_level) {
    case LSM6DSO16IS_2g:
      *FullScale =  2;
      break;

    case LSM6DSO16IS_4g:
      *FullScale =  4;
      break;

    case LSM6DSO16IS_8g:
      *FullScale =  8;
      break;

    case LSM6DSO16IS_16g:
      *FullScale = 16;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}




/**
  * @brief  Get the LSM6DSO16IS accelerometer sensor output data rate
  * @param  Odr pointer where the output data rate is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_ODR(float_t *Odr)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_data_rate_t odr_low_level;

  /* Get current output data rate. */
  if (xl_data_rate_get(&odr_low_level) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  switch (odr_low_level) {
    case LSM6DSO16IS_XL_ODR_OFF:
      *Odr = 0.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_12Hz5_HP:
      *Odr = 12.5f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_26H_HP:
      *Odr = 26.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_52Hz_HP:
      *Odr = 52.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_104Hz_HP:
      *Odr = 104.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_208Hz_HP:
      *Odr = 208.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_416Hz_HP:
      *Odr = 416.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_833Hz_HP:
      *Odr = 833.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_1667Hz_HP:
      *Odr = 1667.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_3333Hz_HP:
      *Odr = 3333.0f;
      break;

    case LSM6DSO16IS_XL_ODR_AT_6667Hz_HP:
      *Odr = 6667.0f;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS accelerometer sensor sensitivity
  * @param  Sensitivity pointer
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_Sensitivity(float_t *Sensitivity)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_full_scale_t full_scale;

  /* Read actual full scale selection from sensor. */
  if (xl_full_scale_get(&full_scale) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Store the Sensitivity based on actual full scale. */
  switch (full_scale) {
    case LSM6DSO16IS_2g:
      *Sensitivity = LSM6DSO16IS_ACC_SENSITIVITY_FS_2G;
      break;

    case LSM6DSO16IS_4g:
      *Sensitivity = LSM6DSO16IS_ACC_SENSITIVITY_FS_4G;
      break;

    case LSM6DSO16IS_8g:
      *Sensitivity = LSM6DSO16IS_ACC_SENSITIVITY_FS_8G;
      break;

    case LSM6DSO16IS_16g:
      *Sensitivity = LSM6DSO16IS_ACC_SENSITIVITY_FS_16G;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS accelerometer sensor full scale
  * @param  FullScale the functional full scale to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_FS(int32_t FullScale)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_full_scale_t new_fs;

  new_fs = (FullScale <= 2) ? LSM6DSO16IS_2g
           : (FullScale <= 4) ? LSM6DSO16IS_4g
           : (FullScale <= 8) ? LSM6DSO16IS_8g
           :                    LSM6DSO16IS_16g;

  if (xl_full_scale_set(new_fs) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS accelerometer sensor raw axes
  * @param  Value pointer where the raw values of the axes are written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_AxesRaw(int32_t *Value)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  int16_t data_raw[3];

  /* Read raw data values. */
  if (acceleration_raw_get(data_raw) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Format the data. */
  Value[0] = (int32_t) data_raw[0];
  Value[1] = (int32_t) data_raw[1];
  Value[2] = (int32_t) data_raw[2];

  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS accelerometer sensor axes
  * @param  Acceleration pointer where the values of the axes are written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_Axes(float *Acceleration)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  int16_t data_raw[3];
  float_t sensitivity = 0.0f;

  /* Read raw data values. */
  if (acceleration_raw_get(data_raw) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Get LSM6DSO16IS actual sensitivity. */
  if (Get_X_Sensitivity(&sensitivity) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Calculate the data. */
  Acceleration[0] = ((float_t)((float_t)data_raw[0] * sensitivity));
  Acceleration[1] = ((float_t)((float_t)data_raw[1] * sensitivity));
  Acceleration[2] = ((float_t)((float_t)data_raw[2] * sensitivity));

  return ret;
}


/**
  * @brief  Enable the LSM6DSO16IS gyroscope sensor
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Enable_G(void)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_data_rate_t new_odr;

  /* Check if the component is already enabled */
  if (G_isEnabled == 1U) {
    ret = LSM6DSO16IS_STATUS_OK;
  } else {
    new_odr = (G_Last_ODR <=   12.5f) ? LSM6DSO16IS_GY_ODR_AT_12Hz5_HP
              : (G_Last_ODR <=   26.0f) ? LSM6DSO16IS_GY_ODR_AT_26H_HP
              : (G_Last_ODR <=   52.0f) ? LSM6DSO16IS_GY_ODR_AT_52Hz_HP
              : (G_Last_ODR <=  104.0f) ? LSM6DSO16IS_GY_ODR_AT_104Hz_HP
              : (G_Last_ODR <=  208.0f) ? LSM6DSO16IS_GY_ODR_AT_208Hz_HP
              : (G_Last_ODR <=  416.0f) ? LSM6DSO16IS_GY_ODR_AT_416Hz_HP
              : (G_Last_ODR <=  833.0f) ? LSM6DSO16IS_GY_ODR_AT_833Hz_HP
              : (G_Last_ODR <= 1667.0f) ? LSM6DSO16IS_GY_ODR_AT_1667Hz_HP
              : (G_Last_ODR <= 3333.0f) ? LSM6DSO16IS_GY_ODR_AT_3333Hz_HP
              :                    LSM6DSO16IS_GY_ODR_AT_6667Hz_HP;
    /* Output data rate selection. */
    if (gy_data_rate_set(new_odr) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }

    G_isEnabled = 1;
  }

  return ret;
}


/**
  * @brief  Disable the LSM6DSO16IS gyroscope sensor
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Disable_G(void)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;

  /* Check if the component is already disabled */
  if (G_isEnabled == 0U) {
    ret = LSM6DSO16IS_STATUS_OK;
  } else {
    /* Get current output data rate. */
    if (Get_G_ODR(&G_Last_ODR) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }
    /* Output data rate selection - power down. */
    if (gy_data_rate_set(LSM6DSO16IS_GY_ODR_OFF) != LSM6DSO16IS_STATUS_OK) {
      ret = LSM6DSO16IS_STATUS_ERROR;
    }

    G_isEnabled = 0;
  }

  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS gyroscope sensor sensitivity
  * @param  Sensitivity pointer
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_Sensitivity(float_t *Sensitivity)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_full_scale_t full_scale;

  /* Read actual full scale selection from sensor. */
  if (gy_full_scale_get(&full_scale) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Store the sensitivity based on actual full scale. */
  switch (full_scale) {
    case LSM6DSO16IS_125dps:
      *Sensitivity = LSM6DSO16IS_GYRO_SENSITIVITY_FS_125DPS;
      break;

    case LSM6DSO16IS_250dps:
      *Sensitivity = LSM6DSO16IS_GYRO_SENSITIVITY_FS_250DPS;
      break;

    case LSM6DSO16IS_500dps:
      *Sensitivity = LSM6DSO16IS_GYRO_SENSITIVITY_FS_500DPS;
      break;

    case LSM6DSO16IS_1000dps:
      *Sensitivity = LSM6DSO16IS_GYRO_SENSITIVITY_FS_1000DPS;
      break;

    case LSM6DSO16IS_2000dps:
      *Sensitivity = LSM6DSO16IS_GYRO_SENSITIVITY_FS_2000DPS;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS gyroscope sensor output data rate
  * @param  Odr pointer where the output data rate is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_ODR(float_t *Odr)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_data_rate_t odr_low_level;

  /* Get current output data rate. */
  if (gy_data_rate_get(&odr_low_level) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  switch (odr_low_level) {
    case LSM6DSO16IS_GY_ODR_OFF:
      *Odr = 0.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_12Hz5_HP:
      *Odr = 12.5f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_26H_HP:
      *Odr = 26.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_52Hz_HP:
      *Odr = 52.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_104Hz_HP:
      *Odr = 104.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_208Hz_HP:
      *Odr = 208.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_416Hz_HP:
      *Odr = 416.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_833Hz_HP:
      *Odr = 833.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_1667Hz_HP:
      *Odr =  1667.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_3333Hz_HP:
      *Odr =  3333.0f;
      break;

    case LSM6DSO16IS_GY_ODR_AT_6667Hz_HP:
      *Odr =  6667.0f;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS gyroscope sensor output data rate
  * @param  Odr the output data rate value to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_ODR(float_t Odr)
{
  LSM6DSO16ISStatusTypeDef ret;

  /* Check if the component is enabled */
  if (G_isEnabled == 1U) {
    ret = Set_G_ODR_When_Enabled(Odr);
  } else {
    ret = Set_G_ODR_When_Disabled(Odr);
  }

  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS gyroscope sensor full scale
  * @param  FullScale pointer where the full scale is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_FS(int32_t  *FullScale)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_full_scale_t fs_low_level;

  /* Read actual full scale selection from sensor. */
  if (gy_full_scale_get(&fs_low_level) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  switch (fs_low_level) {
    case LSM6DSO16IS_125dps:
      *FullScale =  125;
      break;

    case LSM6DSO16IS_250dps:
      *FullScale =  250;
      break;

    case LSM6DSO16IS_500dps:
      *FullScale =  500;
      break;

    case LSM6DSO16IS_1000dps:
      *FullScale = 1000;
      break;

    case LSM6DSO16IS_2000dps:
      *FullScale = 2000;
      break;

    default:
      ret = LSM6DSO16IS_STATUS_ERROR;
      break;
  }

  return ret;
}


/**
  * @brief  Set the LSM6DSO16IS gyroscope sensor full scale
  * @param  FullScale the functional full scale to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_FS(int32_t FullScale)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_full_scale_t new_fs;

  new_fs = (FullScale <= 125)  ? LSM6DSO16IS_125dps
           : (FullScale <= 250)  ? LSM6DSO16IS_250dps
           : (FullScale <= 500)  ? LSM6DSO16IS_500dps
           : (FullScale <= 1000) ? LSM6DSO16IS_1000dps
           :                       LSM6DSO16IS_2000dps;

  if (gy_full_scale_set(new_fs) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS gyroscope sensor raw axes
  * @param  Value pointer where the raw values of the axes are written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_AxesRaw(int32_t *Value)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  int16_t data_raw[3];

  /* Read raw data values. */
  if (angular_rate_raw_get(data_raw) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Format the data. */
  Value[0] = (int32_t) data_raw[0];
  Value[1] = (int32_t) data_raw[1];
  Value[2] = (int32_t) data_raw[2];

  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS gyroscope sensor axes
  * @param  AngularRate pointer where the values of the axes are written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_Axes(float *AngularRate)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  int16_t data_raw[3];
  float_t sensitivity;

  /* Read raw data values. */
  if (angular_rate_raw_get(data_raw) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Get LSM6DSO16IS actual sensitivity. */
  if (Get_G_Sensitivity(&sensitivity) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  /* Calculate the data. */
  AngularRate[0] = (float)((float_t)((float_t)data_raw[0] * sensitivity));
  AngularRate[1] = (float)((float_t)((float_t)data_raw[1] * sensitivity));
  AngularRate[2] = (float)((float_t)((float_t)data_raw[2] * sensitivity));

  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS register value
  * @param  Reg address to be read
  * @param  Data pointer where the value is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Read_Reg(uint8_t Reg, uint8_t *Data)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  if (readRegister(Reg, Data, 1) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  return ret;
}


/**
 * @brief  Get the status of all ISPU events
 * @param  Status the status of all ISPU events
 * @retval 0 in case of success, an error code otherwise
 */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_ISPU_Status(LSM6DSO16IS_ISPU_Status_t *Status)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;

  union {
    uint32_t val;
    LSM6DSO16IS_ISPU_Status_t status;
  } ispu_status;

  if (ia_ispu_get(&ispu_status.val) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

  *Status = ispu_status.status;
  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS ISPU Output
  * @param  Reg address where to start reading
  * @param  len number of registers to read
  * @param  Data pointer where the value is written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Read_ISPU_Output(uint8_t Reg, uint8_t *Data, uint8_t len)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  //Check that register to read is an ISPU Output register
  if (Reg < LSM6DSO16IS_ISPU_DOUT_00_L || Reg > LSM6DSO16IS_ISPU_DOUT_31_H) {
    return LSM6DSO16IS_STATUS_ERROR;
  }
  //Enable the access to the ISPU interaction registers
  if (mem_bank_set(LSM6DSO16IS_ISPU_MEM_BANK) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
  if (readRegister(Reg, Data, len) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
  //Disable the access to the ISPU interaction registers
  if (mem_bank_set(LSM6DSO16IS_MAIN_MEM_BANK) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS register value
  * @param  Reg address to be written
  * @param  Data value to be written
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Write_Reg(uint8_t Reg, uint8_t Data)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
/*
  if (lsm6dso16is_write_reg(&reg_ctx, Reg, &Data, 1) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Set self test
  * @param  Val the value of st_xl in reg CTRL5_C
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_SelfTest(uint8_t Val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_self_test_t reg;

  reg = (Val == 1U)  ? LSM6DSO16IS_XL_ST_POSITIVE
        : (Val == 2U)  ? LSM6DSO16IS_XL_ST_NEGATIVE
        :                LSM6DSO16IS_XL_ST_DISABLE;
/*
  if (lsm6dso16is_xl_self_test_set(&reg_ctx, reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}


/**
  * @brief  Get the LSM6DSO16IS ACC data ready bit value
  * @param  Status the status of data ready bit
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_DRDY_Status(uint8_t *Status)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
/*
  if (lsm6dso16is_xl_flag_data_ready_get(&reg_ctx, Status) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS ACC initialization status
  * @param  Status 1 if initialized, 0 otherwise
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_X_Init_Status(uint8_t *Status)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;

  *Status = isInitialized;

  return ret;
}

/**
  * @brief  Set DRDY on INT1
  * @param  Val the value of int1_drdy_xl in reg INT1_CTRL
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_INT1_DRDY(uint8_t Val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_pin_int1_route_t reg;
/*
  if (lsm6dso16is_pin_int1_route_get(&reg_ctx, &reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  if (Val <= 1U) {
    reg.drdy_xl = Val;
  } else {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }

/*
  if (lsm6dso16is_pin_int1_route_set(&reg_ctx, reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Set self test
  * @param  Val the value of st_xl in reg CTRL5_C
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_SelfTest(uint8_t Val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_self_test_t reg;

  reg = (Val == 1U)  ? LSM6DSO16IS_GY_ST_POSITIVE
        : (Val == 2U)  ? LSM6DSO16IS_GY_ST_NEGATIVE
        :                LSM6DSO16IS_GY_ST_DISABLE;

/*
  if (lsm6dso16is_gy_self_test_set(&reg_ctx, reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS GYRO data ready bit value
  * @param  Status the status of data ready bit
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_DRDY_Status(uint8_t *Status)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
/*
  if (lsm6dso16is_gy_flag_data_ready_get(&reg_ctx, Status) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Get the LSM6DSO16IS GYRO initialization status
  * @param  Status 1 if initialized, 0 otherwise
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Get_G_Init_Status(uint8_t *Status)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;

  *Status = isInitialized;

  return ret;
}

/**
  * @brief  Set DRDY on INT1
  * @param  Val the value of int1_drdy_g in reg INT1_CTRL
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_INT1_DRDY(uint8_t Val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_pin_int1_route_t reg;
/*
  if (lsm6dso16is_pin_int1_route_get(&reg_ctx, &reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  if (Val <= 1U) {
    reg.drdy_gy = Val;
  } else {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
/*
  if (lsm6dso16is_pin_int1_route_set(&reg_ctx, reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}


/**
  * @brief  Set DRDY mode
  * @param  Val the value of drdy_pulsed in reg LSM6DSO16IS_DRDY_PULSE_CFG_G
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_DRDY_Mode(uint8_t Val)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_data_ready_mode_t reg;

  reg = (Val == 0U)  ? LSM6DSO16IS_DRDY_LATCHED
        :                LSM6DSO16IS_DRDY_PULSED;
/*
  if (lsm6dso16is_data_ready_mode_set(&reg_ctx, reg) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS accelerometer sensor output data rate when enabled
  * @param  Odr the functional output data rate to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_ODR_When_Enabled(float_t Odr)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_xl_data_rate_t new_odr;

  new_odr = (Odr <=   12.5f) ? LSM6DSO16IS_XL_ODR_AT_12Hz5_HP
            : (Odr <=   26.0f) ? LSM6DSO16IS_XL_ODR_AT_26H_HP
            : (Odr <=   52.0f) ? LSM6DSO16IS_XL_ODR_AT_52Hz_HP
            : (Odr <=  104.0f) ? LSM6DSO16IS_XL_ODR_AT_104Hz_HP
            : (Odr <=  208.0f) ? LSM6DSO16IS_XL_ODR_AT_208Hz_HP
            : (Odr <=  416.0f) ? LSM6DSO16IS_XL_ODR_AT_416Hz_HP
            : (Odr <=  833.0f) ? LSM6DSO16IS_XL_ODR_AT_833Hz_HP
            : (Odr <= 1667.0f) ? LSM6DSO16IS_XL_ODR_AT_1667Hz_HP
            : (Odr <= 3333.0f) ? LSM6DSO16IS_XL_ODR_AT_3333Hz_HP
            :                    LSM6DSO16IS_XL_ODR_AT_6667Hz_HP;

  /* Output data rate selection. */
  /*
  if (lsm6dso16is_xl_data_rate_set(&reg_ctx, new_odr) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS accelerometer sensor output data rate when disabled
  * @param  Odr the functional output data rate to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_X_ODR_When_Disabled(float_t Odr)
{
  X_Last_ODR = (Odr <=   12.5f) ? LSM6DSO16IS_XL_ODR_AT_12Hz5_HP
               : (Odr <=   26.0f) ? LSM6DSO16IS_XL_ODR_AT_26H_HP
               : (Odr <=   52.0f) ? LSM6DSO16IS_XL_ODR_AT_52Hz_HP
               : (Odr <=  104.0f) ? LSM6DSO16IS_XL_ODR_AT_104Hz_HP
               : (Odr <=  208.0f) ? LSM6DSO16IS_XL_ODR_AT_208Hz_HP
               : (Odr <=  416.0f) ? LSM6DSO16IS_XL_ODR_AT_416Hz_HP
               : (Odr <=  833.0f) ? LSM6DSO16IS_XL_ODR_AT_833Hz_HP
               : (Odr <= 1667.0f) ? LSM6DSO16IS_XL_ODR_AT_1667Hz_HP
               : (Odr <= 3333.0f) ? LSM6DSO16IS_XL_ODR_AT_3333Hz_HP
               :                    LSM6DSO16IS_XL_ODR_AT_6667Hz_HP;

  return LSM6DSO16IS_STATUS_OK;
}

/**
  * @brief  Set the LSM6DSO16IS gyroscope sensor output data rate when enabled
  * @param  Odr the functional output data rate to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_ODR_When_Enabled(float_t Odr)
{
  LSM6DSO16ISStatusTypeDef ret = LSM6DSO16IS_STATUS_OK;
  lsm6dso16is_gy_data_rate_t new_odr;

  new_odr = (Odr <=   12.5f) ? LSM6DSO16IS_GY_ODR_AT_12Hz5_HP
            : (Odr <=   26.0f) ? LSM6DSO16IS_GY_ODR_AT_26H_HP
            : (Odr <=   52.0f) ? LSM6DSO16IS_GY_ODR_AT_52Hz_HP
            : (Odr <=  104.0f) ? LSM6DSO16IS_GY_ODR_AT_104Hz_HP
            : (Odr <=  208.0f) ? LSM6DSO16IS_GY_ODR_AT_208Hz_HP
            : (Odr <=  416.0f) ? LSM6DSO16IS_GY_ODR_AT_416Hz_HP
            : (Odr <=  833.0f) ? LSM6DSO16IS_GY_ODR_AT_833Hz_HP
            : (Odr <= 1667.0f) ? LSM6DSO16IS_GY_ODR_AT_1667Hz_HP
            : (Odr <= 3333.0f) ? LSM6DSO16IS_GY_ODR_AT_3333Hz_HP
            :                    LSM6DSO16IS_GY_ODR_AT_6667Hz_HP;

  /* Output data rate selection. */
  /*
  if (lsm6dso16is_gy_data_rate_set(&reg_ctx, new_odr) != LSM6DSO16IS_STATUS_OK) {
    ret = LSM6DSO16IS_STATUS_ERROR;
  }
*/
  return ret;
}

/**
  * @brief  Set the LSM6DSO16IS gyroscope sensor output data rate when disabled
  * @param  Odr the functional output data rate to be set
  * @retval 0 in case of success, an error code otherwise
  */
LSM6DSO16ISStatusTypeDef LSM6DSO16IS::Set_G_ODR_When_Disabled(float_t Odr)
{
  G_Last_ODR = (Odr <=   12.5f) ? LSM6DSO16IS_GY_ODR_AT_12Hz5_HP
               : (Odr <=   26.0f) ? LSM6DSO16IS_GY_ODR_AT_26H_HP
               : (Odr <=   52.0f) ? LSM6DSO16IS_GY_ODR_AT_52Hz_HP
               : (Odr <=  104.0f) ? LSM6DSO16IS_GY_ODR_AT_104Hz_HP
               : (Odr <=  208.0f) ? LSM6DSO16IS_GY_ODR_AT_208Hz_HP
               : (Odr <=  416.0f) ? LSM6DSO16IS_GY_ODR_AT_416Hz_HP
               : (Odr <=  833.0f) ? LSM6DSO16IS_GY_ODR_AT_833Hz_HP
               : (Odr <= 1667.0f) ? LSM6DSO16IS_GY_ODR_AT_1667Hz_HP
               : (Odr <= 3333.0f) ? LSM6DSO16IS_GY_ODR_AT_3333Hz_HP
               :                    LSM6DSO16IS_GY_ODR_AT_6667Hz_HP;

  return LSM6DSO16IS_STATUS_OK;
}

/* Utility */

float_t LSM6DSO16IS::from_fs2g_to_mg(int16_t lsb)
{
  return ((float_t)lsb * 0.061f);
}

float_t LSM6DSO16IS::from_fs4g_to_mg(int16_t lsb)
{
  return ((float_t)lsb * 0.122f);
}

float_t LSM6DSO16IS::from_fs8g_to_mg(int16_t lsb)
{
  return ((float_t)lsb * 0.244f);
}

float_t LSM6DSO16IS::from_fs16g_to_mg(int16_t lsb)
{
  return ((float_t)lsb * 0.488f);
}

float_t LSM6DSO16IS::from_fs125dps_to_mdps(int16_t lsb)
{
  return ((float_t)lsb * 4.375f);
}

float_t LSM6DSO16IS::from_fs250dps_to_mdps(int16_t lsb)
{
  return ((float_t)lsb * 8.75f);
}

float_t LSM6DSO16IS::from_fs500dps_to_mdps(int16_t lsb)
{
  return ((float_t)lsb * 17.50f);
}

float_t LSM6DSO16IS::from_fs1000dps_to_mdps(int16_t lsb)
{
  return ((float_t)lsb * 35.0f);
}

float_t LSM6DSO16IS::from_fs2000dps_to_mdps(int16_t lsb)
{
  return ((float_t)lsb * 70.0f);
}

float_t LSM6DSO16IS::from_lsb_to_celsius(int16_t lsb)
{
  return (((float_t)lsb / 256.0f) + 25.0f);
}


void LSM6DSO16IS::readSensorData() {
    // Leggi i dati dal sensore
    if (i2c) {
        // Lettura specifica per I2C
    } else if (spi) {
        // Lettura specifica per SPI
    }
}

/********************************************************************/
int32_t LSM6DSO16IS::xl_data_rate_set(lsm6dso16is_xl_data_rate_t val)
{
  lsm6dso16is_ctrl1_xl_t ctrl1_xl;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);

  if (ret == 0) {
    if (((uint8_t)val & 0x10U) == 0x10U) {
      ret += xl_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_DISABLED);
    } else {
      ret += xl_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_ENABLED);
    }

    ctrl1_xl.odr_xl = ((uint8_t)val & 0xfU);
    ret += writeRegister(LSM6DSO16IS_CTRL1_XL, reinterpret_cast<const uint8_t*>(&ctrl1_xl), sizeof(ctrl1_xl));
  }

  return ret;
}

int32_t LSM6DSO16IS::xl_hm_mode_set(lsm6dso16is_hm_mode_t val)
{
  lsm6dso16is_ctrl6_c_t ctrl6_c;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL6_C, (uint8_t *)&ctrl6_c, 1);

  if (ret == 0) {
    ctrl6_c.xl_hm_mode = ((uint8_t)val & 0x1U);
    ret = writeRegister(LSM6DSO16IS_CTRL6_C, reinterpret_cast<const uint8_t*>(&ctrl6_c), sizeof(ctrl6_c));
  }

  return ret;
}

int32_t LSM6DSO16IS::gy_data_rate_set(lsm6dso16is_gy_data_rate_t val)
{
  lsm6dso16is_ctrl2_g_t ctrl2_g;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);

  if (ret == 0) {
    if (((uint8_t)val & 0x10U) == 0x10U) {
      ret += gy_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_DISABLED);
    } else {
      ret += gy_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_ENABLED);
    }

    ctrl2_g.odr_g = ((uint8_t)val & 0xfU);
    ret += writeRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::gy_hm_mode_set(lsm6dso16is_hm_mode_t val)
{
  lsm6dso16is_ctrl7_g_t ctrl7_g;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL7_G, (uint8_t *)&ctrl7_g, 1);

  if (ret == 0) {
    ctrl7_g.g_hm_mode = ((uint8_t)val & 0x1U);
    ret = writeRegister(LSM6DSO16IS_CTRL7_G, (uint8_t *)&ctrl7_g, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::acceleration_raw_get(int16_t *val)
{
  uint8_t buff[6];
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_OUTX_L_A, buff, 6);
  val[0] = (int16_t)buff[1];
  val[0] = (val[0] * 256) + (int16_t)buff[0];
  val[1] = (int16_t)buff[3];
  val[1] = (val[1] * 256) + (int16_t)buff[2];
  val[2] = (int16_t)buff[5];
  val[2] = (val[2] * 256) + (int16_t)buff[4];

  return ret;
}

int32_t LSM6DSO16IS::auto_increment_set(uint8_t val)
{
  lsm6dso16is_ctrl3_c_t ctrl3_c;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL3_C, (uint8_t *)&ctrl3_c, 1);

  if (ret == 0) {
    ctrl3_c.if_inc = val;
    ret = writeRegister(LSM6DSO16IS_CTRL3_C, (uint8_t *)&ctrl3_c, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::block_data_update_set(uint8_t val)
{
  lsm6dso16is_ctrl3_c_t ctrl3_c;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL3_C, (uint8_t *)&ctrl3_c, 1);

  if (ret == 0) {
    ctrl3_c.bdu = val;
    ret = writeRegister(LSM6DSO16IS_CTRL3_C, (uint8_t *)&ctrl3_c, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::data_rate_set(lsm6dso16is_xl_data_rate_t val)
{
  lsm6dso16is_ctrl1_xl_t ctrl1_xl;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);

  if (ret == 0) {
    if (((uint8_t)val & 0x10U) == 0x10U) {
      ret += xl_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_DISABLED);
    } else {
      ret += xl_hm_mode_set(LSM6DSO16IS_HIGH_PERFOMANCE_MODE_ENABLED);
    }

    ctrl1_xl.odr_xl = ((uint8_t)val & 0xfU);
    ret += writeRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::xl_full_scale_set(lsm6dso16is_xl_full_scale_t val)
{
  lsm6dso16is_ctrl1_xl_t ctrl1_xl;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);

  if (ret == 0) {
    ctrl1_xl.fs_xl = ((uint8_t)val & 0x3U);
    ret = writeRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::gy_full_scale_set(lsm6dso16is_gy_full_scale_t val)
{
  lsm6dso16is_ctrl2_g_t ctrl2_g;
  int32_t ret;
\
  ret = readRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);

  if (ret == 0) {
    ctrl2_g.fs_g = ((uint8_t)val & 0x3U);
    ctrl2_g.fs_125 = ((uint8_t)val >> 4);
    ret = writeRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);
  }

  return ret;
}

int32_t LSM6DSO16IS::xl_full_scale_get(lsm6dso16is_xl_full_scale_t *val)
{
  lsm6dso16is_ctrl1_xl_t ctrl1_xl;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);

  switch ((ctrl1_xl.fs_xl)) {
    case LSM6DSO16IS_2g:
      *val = LSM6DSO16IS_2g;
      break;

    case LSM6DSO16IS_4g:
      *val = LSM6DSO16IS_4g;
      break;

    case LSM6DSO16IS_8g:
      *val = LSM6DSO16IS_8g;
      break;

    case LSM6DSO16IS_16g:
      *val = LSM6DSO16IS_16g;
      break;

    default:
      *val = LSM6DSO16IS_2g;
      break;
  }
  return ret;
}

int32_t LSM6DSO16IS::xl_data_rate_get(lsm6dso16is_xl_data_rate_t *val)
{
  lsm6dso16is_ctrl1_xl_t ctrl1_xl;
  lsm6dso16is_ctrl6_c_t ctrl6_c;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL1_XL, (uint8_t *)&ctrl1_xl, 1);
  if (ret == 0) {
    ret = readRegister(LSM6DSO16IS_CTRL6_C, (uint8_t *)&ctrl6_c, 1);
  }

  switch ((ctrl6_c.xl_hm_mode << 4) | (ctrl1_xl.odr_xl)) {
    case LSM6DSO16IS_XL_ODR_OFF:
      *val = LSM6DSO16IS_XL_ODR_OFF;
      break;

    case LSM6DSO16IS_XL_ODR_AT_12Hz5_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_12Hz5_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_26H_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_26H_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_52Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_52Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_104Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_104Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_208Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_208Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_416Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_416Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_833Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_833Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_1667Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_1667Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_3333Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_3333Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_6667Hz_HP:
      *val = LSM6DSO16IS_XL_ODR_AT_6667Hz_HP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_12Hz5_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_12Hz5_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_26H_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_26H_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_52Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_52Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_104Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_104Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_208Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_208Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_416Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_416Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_833Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_833Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_1667Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_1667Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_3333Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_3333Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_6667Hz_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_6667Hz_LP;
      break;

    case LSM6DSO16IS_XL_ODR_AT_1Hz6_LP:
      *val = LSM6DSO16IS_XL_ODR_AT_1Hz6_LP;
      break;

    default:
      *val = LSM6DSO16IS_XL_ODR_OFF;
      break;
  }
  return ret;
}

int32_t LSM6DSO16IS::gy_full_scale_get(lsm6dso16is_gy_full_scale_t *val)
{
  lsm6dso16is_ctrl2_g_t ctrl2_g;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);

  switch ((ctrl2_g.fs_125 << 4) | (ctrl2_g.fs_g)) {
    case LSM6DSO16IS_125dps:
      *val = LSM6DSO16IS_125dps;
      break;

    case LSM6DSO16IS_250dps:
      *val = LSM6DSO16IS_250dps;
      break;

    case LSM6DSO16IS_500dps:
      *val = LSM6DSO16IS_500dps;
      break;

    case LSM6DSO16IS_1000dps:
      *val = LSM6DSO16IS_1000dps;
      break;

    case LSM6DSO16IS_2000dps:
      *val = LSM6DSO16IS_2000dps;
      break;

    default:
      *val = LSM6DSO16IS_125dps;
      break;
  }
  return ret;
}

int32_t LSM6DSO16IS::gy_data_rate_get(lsm6dso16is_gy_data_rate_t *val)
{
  lsm6dso16is_ctrl2_g_t ctrl2_g;
  lsm6dso16is_ctrl7_g_t ctrl7_g;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_CTRL2_G, (uint8_t *)&ctrl2_g, 1);
  if (ret == 0) {
    ret = readRegister(LSM6DSO16IS_CTRL7_G, (uint8_t *)&ctrl7_g, 1);
  }

  switch ((ctrl7_g.g_hm_mode << 4) | (ctrl2_g.odr_g)) {
    case LSM6DSO16IS_GY_ODR_OFF:
      *val = LSM6DSO16IS_GY_ODR_OFF;
      break;

    case LSM6DSO16IS_GY_ODR_AT_12Hz5_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_12Hz5_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_26H_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_26H_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_52Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_52Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_104Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_104Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_208Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_208Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_416Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_416Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_833Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_833Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_1667Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_1667Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_3333Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_3333Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_6667Hz_HP:
      *val = LSM6DSO16IS_GY_ODR_AT_6667Hz_HP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_12Hz5_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_12Hz5_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_26H_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_26H_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_52Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_52Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_104Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_104Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_208Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_208Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_416Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_416Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_833Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_833Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_1667Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_1667Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_3333Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_3333Hz_LP;
      break;

    case LSM6DSO16IS_GY_ODR_AT_6667Hz_LP:
      *val = LSM6DSO16IS_GY_ODR_AT_6667Hz_LP;
      break;

    default:
      *val = LSM6DSO16IS_GY_ODR_OFF;
      break;
  }

  return ret;
}

int32_t LSM6DSO16IS::angular_rate_raw_get(int16_t *val)
{
  uint8_t buff[6];
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_OUTX_L_G, buff, 6);
  val[0] = (int16_t)buff[1];
  val[0] = (val[0] * 256) + (int16_t)buff[0];
  val[1] = (int16_t)buff[3];
  val[1] = (val[1] * 256) + (int16_t)buff[2];
  val[2] = (int16_t)buff[5];
  val[2] = (val[2] * 256) + (int16_t)buff[4];

  return ret;
}

int32_t LSM6DSO16IS::ia_ispu_get(uint32_t *val)
{
  uint8_t buff[4];
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_ISPU_INT_STATUS0_MAINPAGE, &buff[0], 4);

  *val = (uint32_t)buff[3];
  *val = (*val * 256U) + (uint32_t)buff[2];
  *val = (*val * 256U) + (uint32_t)buff[1];
  *val = (*val * 256U) + (uint32_t)buff[0];

  return ret;
}

int32_t LSM6DSO16IS::mem_bank_set(lsm6dso16is_mem_bank_t val)
{
  lsm6dso16is_func_cfg_access_t func_cfg_access;
  int32_t ret;

  ret = readRegister(LSM6DSO16IS_FUNC_CFG_ACCESS, (uint8_t *)&func_cfg_access, 1);

  if (ret == 0) {
    func_cfg_access.shub_reg_access = (val == LSM6DSO16IS_SENSOR_HUB_MEM_BANK) ? 0x1U : 0x0U;
    func_cfg_access.ispu_reg_access = (val == LSM6DSO16IS_ISPU_MEM_BANK) ? 0x1U : 0x0U;
    ret = writeRegister(LSM6DSO16IS_FUNC_CFG_ACCESS, (uint8_t *)&func_cfg_access, 1);
  }

  return ret;
}