/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "vt_dsc.h"
#include "vt_debug.h"

static uint32_t _vt_dsc_flash_get_sector(uint32_t Address);

/*

Sector 0    0x0800 0000 - 0x0800 3FFF 16 Kbyte
Sector 1    0x0800 4000 - 0x0800 7FFF 16 Kbyte
Sector 2    0x0800 8000 - 0x0800 BFFF 16 Kbyte
Sector 3    0x0800 C000 - 0x0800 FFFF 16 Kbyte
Sector 4    0x0801 0000 - 0x0801 FFFF 64 Kbyte
Sector 5    0x0802 0000 - 0x0803 FFFF 128 Kbyte
Sector 6    0x0804 0000 - 0x0805 FFFF 128 Kbyte
Sector 7    0x0806 0000 - 0x0807 FFFF 128 Kbyte
Sector 8    0x0808 0000 - 0x0809 FFFF 128 Kbyte
Sector 9    0x080A 0000 - 0x080B FFFF 128 Kbyte
Sector 10   0x080C 0000 - 0x080D FFFF 128 Kbyte
Sector 11   0x080E 0000 - 0x080F FFFF 128 Kbyte

*/

static uint32_t _vt_dsc_flash_get_sector(uint32_t Address)
{

    if ((Address >= 0x08000000) && (Address < 0x08003FFF))
    {
        return FLASH_SECTOR_0;
    }

    else if ((Address >= 0x08004000) && (Address < 0x08007FFF))
    {
        return FLASH_SECTOR_1;
    }

    else if ((Address >= 0x08008000) && (Address < 0x0800BFFF))
    {
        return FLASH_SECTOR_2;
    }

    else if ((Address >= 0x0800C000) && (Address < 0x0800FFFF))
    {
        return FLASH_SECTOR_3;
    }

    else if ((Address >= 0x08010000) && (Address < 0x0801FFFF))
    {
        return FLASH_SECTOR_4;
    }

    else if ((Address >= 0x08020000) && (Address < 0x0803FFFF))
    {
        return FLASH_SECTOR_5;
    }

    else if ((Address >= 0x08040000) && (Address < 0x0805FFFF))
    {
        return FLASH_SECTOR_6;
    }

    else if ((Address >= 0x08060000) && (Address < 0x0807FFFF))
    {
        return FLASH_SECTOR_7;
    }

    else if ((Address >= 0x08080000) && (Address < 0x0809FFFF))
    {
        return FLASH_SECTOR_8;
    }

    else if ((Address >= 0x080A0000) && (Address < 0x080BFFFF))
    {
        return FLASH_SECTOR_9;
    }

    else if ((Address >= 0x080C0000) && (Address < 0x080DFFFF))
    {
        return FLASH_SECTOR_10;
    }

    else if ((Address >= 0x080E0000) && (Address < 0x080FFFFF))
    {
        return FLASH_SECTOR_11;
    }

    return VT_ERROR;
}

uint32_t _vt_dsc_delay_usec(TIMER_HANDLE_TYPEDEF* timer, uint32_t delay)
{
    if (timer == NULL)
    {
        if (delay > 1000)
        {
            uint32_t msec_delay = delay / 1000;
            HAL_Delay(msec_delay);
        }
        else
        {
            HAL_Delay(1);
        }
    }
    
    else
    {
        uint32_t start_time;

        HAL_TIM_Base_Start(timer);
        start_time = __HAL_TIM_GET_COUNTER(timer);
        while (__HAL_TIM_GET_COUNTER(timer) - start_time < delay)
            ;
        HAL_TIM_Base_Stop(timer);
    }

    return VT_SUCCESS;
}

uint32_t _vt_dsc_gpio_read(GPIO_PORT_TYPEDEF* gpio_port, GPIO_PIN_TYPEDEF gpio_pin, int* state)
{
    if (HAL_GPIO_ReadPin(gpio_port, gpio_pin) == GPIO_PIN_SET)
    {
        *state = 1;
    }

    else if (HAL_GPIO_ReadPin(gpio_port, gpio_pin) == GPIO_PIN_RESET)
    {
        *state = 0;
    }

    return VT_SUCCESS;
}

uint32_t _vt_dsc_gpio_turn_on(GPIO_PORT_TYPEDEF* gpio_port, GPIO_PIN_TYPEDEF gpio_pin)
{
    HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);

    return VT_SUCCESS;
}

uint32_t _vt_dsc_gpio_turn_off(GPIO_PORT_TYPEDEF* gpio_port, GPIO_PIN_TYPEDEF gpio_pin)
{
    HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);

    return VT_SUCCESS;
}

uint32_t _vt_dsc_adc_read(ADC_CONTROLLER_TYPEDEF* adc_controller, ADC_CHANNEL_TYPEDEF adc_channel, uint32_t* value)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel      = adc_channel;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;

    HAL_ADC_ConfigChannel(adc_controller, &sConfig);

    HAL_ADC_Start(adc_controller);
    if (HAL_ADC_PollForConversion(adc_controller, 10) == HAL_OK)
    {
        *value = (HAL_ADC_GetValue(adc_controller));
    }
    HAL_ADC_Stop(adc_controller);

    return VT_SUCCESS;
}

uint32_t _vt_dsc_flash_erase(uint32_t flashAddress, uint32_t Nsize)
{
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError;

    uint32_t startsector = _vt_dsc_flash_get_sector(flashAddress);
    if (startsector == VT_ERROR)
    {
        return VT_FLASH_ADDRESS_ERROR;
    }

    uint32_t endsector = _vt_dsc_flash_get_sector(flashAddress + Nsize * 4);
    if (startsector == VT_ERROR)
    {
        return VT_FLASH_ERAZE_SIZE_ERROR;
    }

    EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector       = startsector;
    EraseInitStruct.NbSectors    = startsector - endsector + 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_FLASH_Unlock();

    HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

    HAL_FLASH_Lock();

    return VT_SUCCESS;
}

uint32_t _vt_dsc_flash_write(uint32_t flashAddress, void* wrBuf, uint32_t Nsize)
{
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SECTORError;

    uint32_t startsector = _vt_dsc_flash_get_sector(flashAddress);
    if (startsector == VT_ERROR)
    {
        return VT_FLASH_ADDRESS_ERROR;
    }

    uint32_t endsector = _vt_dsc_flash_get_sector(flashAddress + Nsize * 4);
    if (startsector == VT_ERROR)
    {
        return VT_FLASH_WRITE_SIZE_ERROR;
    }

    EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector       = startsector;
    EraseInitStruct.NbSectors    = endsector - startsector + 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_FLASH_Unlock();

    HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

    for (uint32_t i = 0; i < Nsize; i++, flashAddress += 4)
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flashAddress, ((uint32_t*)wrBuf)[i]);

    HAL_FLASH_Lock();

    return VT_SUCCESS;
}

uint32_t _vt_dsc_flash_read(uint32_t flashAddress, void* rdBuf, uint32_t Nsize)
{
    for (uint32_t i = 0; i < Nsize; i++, flashAddress += 4)
        *((uint32_t*)rdBuf + i) = *(uint32_t*)flashAddress;

    return VT_SUCCESS;
}
