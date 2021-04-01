/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "vt_test_definitions.h"

#include "vt_api.h"
#include "vt_database.h"

#include <setjmp.h>
#include <stdarg.h>

#include <cmocka.h>

// Initialize

VT_DATABASE test_database_ptr;

static int vt_database_setup(void** state)
{
    test_database_ptr._vt_total_fingerprints          = 1;
    test_database_ptr._vt_fingerprintdb[0][0]         = 1;
    test_database_ptr._vt_total_falltime              = 1;
    test_database_ptr._vt_falltimedb[0][0]            = 1;
    test_database_ptr._vt_total_pearson_coefficient   = 1;
    test_database_ptr._vt_pearson_coefficientdb[0][0] = 1;

    return 0;

}

static void test_vt_database_initialize_fingerprintdb(void** state)
{
    (void)state;

    assert_int_equal(test_database_ptr._vt_total_fingerprints, 1);
    assert_int_equal(test_database_ptr._vt_fingerprintdb[0][0], 1);

    _vt_database_initialize_fingerprintdb(&test_database_ptr);

    assert_int_equal(test_database_ptr._vt_total_fingerprints, 0);
    assert_int_equal(test_database_ptr._vt_fingerprintdb[0][0], 0);

}

static void test_vt_database_initialize_falltimedb(void** state)
{
    (void)state;

    assert_int_equal(test_database_ptr._vt_total_falltime, 1);
    assert_int_equal(test_database_ptr._vt_falltimedb[0][0], 1);

    _vt_database_initialize_falltimedb(&test_database_ptr);

    assert_int_equal(test_database_ptr._vt_total_falltime, 0);
    assert_int_equal(test_database_ptr._vt_falltimedb[0][0], 0);
}

static void test_vt_database_initialize_pearsoncoefficientdb(void** state)
{
    (void)state;

    assert_int_equal(test_database_ptr._vt_total_pearson_coefficient, 1);
    assert_float_equal(test_database_ptr._vt_pearson_coefficientdb[0][0], 1, 0.001);

    _vt_database_initialize_pearsoncoefficientdb(&test_database_ptr);

    assert_int_equal(test_database_ptr._vt_total_pearson_coefficient, 0);
    assert_float_equal(test_database_ptr._vt_pearson_coefficientdb[0][0], 0, 0.001);
}

// Store

static void test_vt_database_store_fingerprint(void** state)
{
    (void)state;

    // Static Function
}

static void test_vt_database_store_falltime(void** state)
{
    (void)state;
}

static void test_vt_database_store_pearsoncoefficient(void** state)
{
    (void)state;
}

static void test_vt_database_store_flash(void** state)
{
    (void)state;

    // Static Function
}

// Clear

static void test_vt_database_clear(void** state)
{
    (void)state;
}

// Fetch

static void test_vt_database_fingerprint_fetch(void** state)
{
    (void)state;
}

static void test_vt_database_falltime_fetch(void** state)
{
    (void)state;
}

static void test_vt_database_pearsoncoefficient_fetch(void** state)
{
    (void)state;
}

// Evaluate

static void test_vt_database_evaluate_nrmse(void** state)
{
    (void)state;
}

static void test_vt_database_evaluate_pearson_falltime(void** state)
{
    (void)state;
}

static void test_vt_database_evaluate_check_pearson_falltime_availability(void** state)
{
    (void)state;
}

static void test_vt_database_evaluate_falltime_nearestindex_search(void** state)
{
    (void)state;

    // Static Function
}

static void test_vt_database_evaluate_pearsoncoefficient_falltimeindex_search(void** state)
{
    (void)state;

    // Static Function
}

int test_vt_database()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_vt_database_initialize_fingerprintdb),
        cmocka_unit_test(test_vt_database_initialize_falltimedb),
        cmocka_unit_test(test_vt_database_initialize_pearsoncoefficientdb),
        cmocka_unit_test(test_vt_database_store_fingerprint),
        cmocka_unit_test(test_vt_database_store_falltime),
        cmocka_unit_test(test_vt_database_store_pearsoncoefficient),
        cmocka_unit_test(test_vt_database_store_flash),
        cmocka_unit_test(test_vt_database_clear),
        cmocka_unit_test(test_vt_database_fingerprint_fetch),
        cmocka_unit_test(test_vt_database_falltime_fetch),
        cmocka_unit_test(test_vt_database_pearsoncoefficient_fetch),
        cmocka_unit_test(test_vt_database_evaluate_nrmse),
        cmocka_unit_test(test_vt_database_evaluate_pearson_falltime),
        cmocka_unit_test(test_vt_database_evaluate_check_pearson_falltime_availability),
        cmocka_unit_test(test_vt_database_evaluate_falltime_nearestindex_search),
        cmocka_unit_test(test_vt_database_evaluate_pearsoncoefficient_falltimeindex_search),
    };

    return cmocka_run_group_tests_name("vt_core_database", tests, vt_database_setup, NULL);
}
