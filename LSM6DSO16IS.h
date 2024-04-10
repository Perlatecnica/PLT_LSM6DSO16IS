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

#ifndef LSM6DSO16IS_H
#define LSM6DSO16IS_H

#include "mbed.h"
#include <cstdint>
#include "registers.h"



class LSM6DSO16IS {
public:
    // Costruttori privati
    LSM6DSO16IS(PinName sda, PinName scl);
    LSM6DSO16IS(PinName mosi, PinName miso, PinName sck, PinName cs);

    // Distruttore privato
    ~LSM6DSO16IS();

    enum InterfaceType {
        INTERFACE_I2C,
        INTERFACE_SPI
    };

    void initialize();
    void readSensorData();
    LSM6DSO16ISStatusTypeDef begin(void);
    LSM6DSO16ISStatusTypeDef ReadID(uint8_t *Id);
    LSM6DSO16ISStatusTypeDef Enable_X(void);
    LSM6DSO16ISStatusTypeDef Disable_X(void);
    LSM6DSO16ISStatusTypeDef Get_X_ODR(float_t *Odr);
    LSM6DSO16ISStatusTypeDef Set_X_ODR(float_t Odr);
    LSM6DSO16ISStatusTypeDef Get_X_FS(int32_t *FullScale);
    LSM6DSO16ISStatusTypeDef Set_X_FS(int32_t FullScale);
    LSM6DSO16ISStatusTypeDef Get_X_Sensitivity(float_t *Sensitivity);
    LSM6DSO16ISStatusTypeDef Get_X_AxesRaw(int32_t *Value);
    LSM6DSO16ISStatusTypeDef Get_X_Axes(float *Acceleration);
    LSM6DSO16ISStatusTypeDef Enable_G(void);
    LSM6DSO16ISStatusTypeDef Disable_G(void);
    LSM6DSO16ISStatusTypeDef Get_G_Sensitivity(float_t *Sensitivity);
    LSM6DSO16ISStatusTypeDef Get_G_ODR(float_t *Odr);
    LSM6DSO16ISStatusTypeDef Set_G_ODR(float_t Odr);
    LSM6DSO16ISStatusTypeDef Get_G_FS(int32_t  *FullScale);
    LSM6DSO16ISStatusTypeDef Set_G_FS(int32_t FullScale);
    LSM6DSO16ISStatusTypeDef Get_G_AxesRaw(int32_t *Value);
    LSM6DSO16ISStatusTypeDef Get_G_Axes(float *AngularRate);
    LSM6DSO16ISStatusTypeDef Read_Reg(uint8_t Reg, uint8_t *Data);
    LSM6DSO16ISStatusTypeDef Get_ISPU_Status(LSM6DSO16IS_ISPU_Status_t *Status);
    LSM6DSO16ISStatusTypeDef Read_ISPU_Output(uint8_t Reg, uint8_t *Data, uint8_t len);
    LSM6DSO16ISStatusTypeDef Write_Reg(uint8_t Reg, uint8_t Data);
    LSM6DSO16ISStatusTypeDef Set_X_SelfTest(uint8_t Val);
    LSM6DSO16ISStatusTypeDef Get_X_DRDY_Status(uint8_t *Status);
    LSM6DSO16ISStatusTypeDef Get_X_Init_Status(uint8_t *Status);
    LSM6DSO16ISStatusTypeDef Set_X_INT1_DRDY(uint8_t Val);
    LSM6DSO16ISStatusTypeDef Set_G_SelfTest(uint8_t Val);
    LSM6DSO16ISStatusTypeDef Get_G_DRDY_Status(uint8_t *Status);
    LSM6DSO16ISStatusTypeDef Get_G_Init_Status(uint8_t *Status);
    LSM6DSO16ISStatusTypeDef Set_G_INT1_DRDY(uint8_t Val);
    LSM6DSO16ISStatusTypeDef Set_DRDY_Mode(uint8_t Val);
    LSM6DSO16ISStatusTypeDef Set_X_ODR_When_Enabled(float_t Odr);
    LSM6DSO16ISStatusTypeDef Set_X_ODR_When_Disabled(float_t Odr);
    LSM6DSO16ISStatusTypeDef Set_G_ODR_When_Enabled(float_t Odr);
    LSM6DSO16ISStatusTypeDef Set_G_ODR_When_Disabled(float_t Odr);
    bool isConnected();    
    
    void set_SDO_SAO_TO_GND();
    void set_SDO_SAO_TO_VCC();


private:
    // Variabili membro private per la gestione del sensore
    I2C* i2c;
    SPI* spi;
    DigitalOut* cs_pin;

    #ifdef IKS4A1
        uint8_t lsm6ds01tis_8bit_address = (0x6A << 1); // 8 bits device address
    #else // DEFAULT ADDRESS
        uint8_t lsm6ds01tis_8bit_address = (0x6A << 1); // 8 bits device address
    #endif

    float X_Last_ODR;
    float G_Last_ODR;
    uint8_t X_isEnabled;
    uint8_t G_isEnabled;
    uint8_t isInitialized;

    float_t from_fs2g_to_mg(int16_t lsb);
    float_t from_fs4g_to_mg(int16_t lsb);
    float_t from_fs8g_to_mg(int16_t lsb);
    float_t from_fs16g_to_mg(int16_t lsb);
    float_t from_fs125dps_to_mdps(int16_t lsb);
    float_t from_fs250dps_to_mdps(int16_t lsb);
    float_t from_fs500dps_to_mdps(int16_t lsb);
    float_t from_fs1000dps_to_mdps(int16_t lsb);
    float_t from_fs2000dps_to_mdps(int16_t lsb);
    float_t from_lsb_to_celsius(int16_t lsb);

    bool readRegister(uint8_t reg, uint8_t *value, uint16_t len);
    bool writeRegister(uint8_t reg, const uint8_t *value, uint16_t len);

    int32_t xl_data_rate_set(lsm6dso16is_xl_data_rate_t val);
    int32_t xl_hm_mode_set(lsm6dso16is_hm_mode_t val);
    int32_t gy_data_rate_set(lsm6dso16is_gy_data_rate_t val);
    int32_t gy_hm_mode_set(lsm6dso16is_hm_mode_t val);
    int32_t acceleration_raw_get(int16_t *val);
    int32_t auto_increment_set(uint8_t val);
    int32_t block_data_update_set(uint8_t val);
    int32_t data_rate_set(lsm6dso16is_xl_data_rate_t val);
    int32_t xl_full_scale_set(lsm6dso16is_xl_full_scale_t val);
    int32_t gy_full_scale_set(lsm6dso16is_gy_full_scale_t val);
    int32_t xl_full_scale_get(lsm6dso16is_xl_full_scale_t *val);
    int32_t xl_data_rate_get(lsm6dso16is_xl_data_rate_t *val);
    int32_t gy_full_scale_get(lsm6dso16is_gy_full_scale_t *val);
    int32_t gy_data_rate_get(lsm6dso16is_gy_data_rate_t *val);
    int32_t angular_rate_raw_get(int16_t *val);
    int32_t ia_ispu_get(uint32_t *val);
    int32_t mem_bank_set(lsm6dso16is_mem_bank_t val);
};

#endif // LSM6DSO16IS_H