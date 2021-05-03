/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _VT_API_H
#define _VT_API_H

#include <stdio.h>

#include "vt_port.h"

#define VT_SUCCESS                  0X00
#define VT_ERROR                    0X01
#define VT_PTR_ERROR                0X02
#define VT_MUTEX_ERROR              0X03
#define VT_UNIDENTIFIED_SENSOR_NAME 0X05
#define VT_THREAD_SLEEP_ERROR       0X06
#define VT_REPEATABILITY_ERROR      0X11
#define VT_RISING_FUNCTION_ERROR    0X12
#define VT_STEP_FUNCTION_ERROR      0X13
#define VT_NOISY_FUNCTION_ERROR     0X14

#define VT_SHAPE_THRESHOLD            0.85f
#define VT_PRECISION_THRESHOLD        20
#define VT_REPEATABILITY_THRESHOLD    0.3f
#define VT_STARTING_FREQUENCY         5000
#define VT_MAXIMUM_FREQUENCY          20000
#define VT_MINIMUM_FREQUENCY          1
#define FALL_TIME_THRESHOLD           10.0f
#define PEARSON_COEFFICIENT_THRESHOLD 10.0f

#define VT_SENOSR_NAME_LENGTH           50
#define VT_FINGERPRINT_LENGTH           100
#define VT_DB_SIZE                      10
#define VT_FINGERPRINT_DB_LENGTH        VT_FINGERPRINT_LENGTH + 2
#define VT_FALLTIME_DB_LENGTH           2
#define VT_PEARSONCOEFFICIENT_DB_LENGTH 2

#define abs_custom(x) ((x)>0?(x):-(x))

typedef struct VT_SENSOR_STRUCT
{
    char vt_sensor_name[VT_SENOSR_NAME_LENGTH];
    GPIO_PORT_TYPEDEF* vt_gpio_port;
    GPIO_PIN_TYPEDEF vt_gpio_pin;

    ADC_CONTROLLER_TYPEDEF* vt_adc_controller;
    ADC_CHANNEL_TYPEDEF vt_adc_channel;

    TIMER_HANDLE_TYPEDEF* vt_timer;

    uint16_t vt_sampling_frequency;
} VT_SENSOR;

typedef struct VT_DATABASE_STRUCT
{
    uint8_t _vt_total_fingerprints;
    uint32_t _vt_fingerprintdb[VT_DB_SIZE][VT_FINGERPRINT_DB_LENGTH];

    uint8_t _vt_total_falltime;
    uint32_t _vt_falltimedb[VT_DB_SIZE][VT_FALLTIME_DB_LENGTH];

    uint8_t _vt_total_pearson_coefficient;
    float _vt_pearson_coefficientdb[VT_DB_SIZE][VT_PEARSONCOEFFICIENT_DB_LENGTH];
} VT_DATABASE;

// Initialize
uint32_t vt_sensor_initialize(VT_SENSOR* sensor_ptr,
    char* sensor_name,
    GPIO_PORT_TYPEDEF* gpio_port,
    GPIO_PIN_TYPEDEF gpio_pin,
    ADC_CONTROLLER_TYPEDEF* adc_controller,
    ADC_CHANNEL_TYPEDEF adc_channel,
    TIMER_HANDLE_TYPEDEF* timer_handler);

// Calibrate
void vt_sensor_calibrate(VT_SENSOR* sensor_ptr, uint8_t* confidence_metric);

// Read
uint32_t vt_sensor_read_value(VT_SENSOR* sensor_ptr, uint32_t* sensor_value);

uint32_t vt_sensor_read_fingerprint(
    VT_SENSOR* sensor_ptr, uint32_t* fingerprint_array, char* fingerprint_string, uint32_t fingerprint_string_length);

uint32_t vt_sensor_read_status(
    VT_SENSOR* sensor_ptr, VT_DATABASE* database_ptr, uint32_t* fingerprint, int8_t* sensor_id);

// Database
uint32_t vt_database_initialize(VT_DATABASE* database_ptr);

uint32_t vt_database_store(
    VT_DATABASE* database_ptr, uint32_t* fingerprint_array, uint16_t sampling_frequency, int8_t sensor_id);

// Fetch
uint32_t vt_database_falltime_fetch(VT_DATABASE* database_ptr, int8_t* index, uint32_t* fall_time, int8_t* sensor_id);

uint32_t vt_database_pearsoncoefficient_fetch(
    VT_DATABASE* database_ptr, int8_t* index, float* pearson_coefficient, int8_t* sensor_id);

#endif
