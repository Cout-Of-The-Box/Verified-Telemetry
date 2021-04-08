/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "vt_test_definitions.h"

#include "vt_api.h"
#include "vt_dsc.h"
#include "vt_port.h"
#include "vt_sensor.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "cmocka.h"

static void test_vt_sensor_initialize(void** state)
{
    (void)state;
}

static void test_vt_sensor_read_value(void** state)
{
    (void)state;

    VT_SENSOR x;
    uint32_t y;

    will_return(__wrap__vt_dsc_adc_read, 23);
    will_return(__wrap__vt_dsc_adc_read, 12);
    
    vt_sensor_read_value(&x, &y);
    assert_int_equal(y,23);

    vt_sensor_read_value(&x, &y);
    assert_int_equal(y,7);

}

static void test_vt_sensor_read_fingerprint(void** state)
{
    (void)state;
}

static void test_vt_sensor_read_status(void** state)
{
    (void)state;
}

static void test_vt_sensor_calibrate(void** state)
{
    (void)state;
}

int __wrap__vt_dsc_adc_read(ADC_CONTROLLER_TYPEDEF* adc_controller, ADC_CHANNEL_TYPEDEF adc_channel, uint32_t* value)
{
    *value = (uint32_t)mock();
    return VT_PLATFORM_SUCCESS;
}

int test_vt_sensor()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_vt_sensor_initialize),
        cmocka_unit_test(test_vt_sensor_read_value),
        cmocka_unit_test(test_vt_sensor_read_fingerprint),
        cmocka_unit_test(test_vt_sensor_read_status),
        cmocka_unit_test(test_vt_sensor_calibrate),
    };

    return cmocka_run_group_tests_name("vt_core_sensor", tests, NULL, NULL);
}
