/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include "vt_cs_signature_features.h"
#include "vt_debug.h"
#include <math.h>

VT_FLOAT cs_repeating_signature_feature_vector_evaluate(VT_FLOAT signature_frequency_under_test,
    VT_FLOAT signature_frequency_saved,
    VT_FLOAT duty_cycle_under_test,
    VT_FLOAT duty_cycle_saved,
    VT_FLOAT relative_current_draw_under_test,
    VT_FLOAT relative_current_draw_saved)
{
    VT_FLOAT signature_drift = 0;
    signature_drift += fabsf((signature_frequency_under_test - signature_frequency_saved) / signature_frequency_saved) * 100.0f;
    signature_drift += fabsf((duty_cycle_under_test - duty_cycle_saved) / duty_cycle_saved) * 100.0f;
    signature_drift +=
        fabsf((relative_current_draw_under_test - relative_current_draw_saved) / relative_current_draw_saved) * 100.0f;
    signature_drift /= 3.0f;

#if VT_LOG_LEVEL > 2
    int32_t decimal  = signature_drift;
    float frac_float = signature_drift - (float)decimal;
    int32_t frac     = frac_float * 10000;
    VTLogDebug("Deviation in Repeating Signature Feature Vector: %lu.%lu \r\n", decimal, frac);
#endif /* VT_LOG_LEVEL > 2 */
    return signature_drift;
}

VT_FLOAT cs_repeating_signature_offset_current_evaluate(VT_FLOAT offset_current_under_test, VT_FLOAT offset_current_saved)
{
    VT_FLOAT signature_drift = 0;
    signature_drift          = fabsf((offset_current_under_test - offset_current_saved) / offset_current_saved) * 100.0f;
#if VT_LOG_LEVEL > 2
    int32_t decimal  = signature_drift;
    float frac_float = signature_drift - (float)decimal;
    int32_t frac     = frac_float * 10000;
    VTLogDebug("Deviation in Repeating Signature Offset Current: %lu.%lu \r\n", decimal, frac);
#endif /* VT_LOG_LEVEL > 2 */
    return signature_drift;
}

VT_FLOAT cs_non_repeating_signature_average_current_evaluate(
    VT_FLOAT avg_curr_on_under_test, VT_FLOAT avg_curr_on_saved, VT_FLOAT avg_curr_off_under_test, VT_FLOAT avg_curr_off_saved)
{
    VT_FLOAT signature_drift = 0;
    signature_drift += fabsf((avg_curr_on_under_test - avg_curr_on_saved) / avg_curr_on_saved) * 100.0f;
    signature_drift += fabsf((avg_curr_off_under_test - avg_curr_off_saved) / avg_curr_off_saved) * 100.0f;
    signature_drift /= 2.0f;

#if VT_LOG_LEVEL > 2
    int32_t decimal  = signature_drift;
    float frac_float = signature_drift - (float)decimal;
    int32_t frac     = frac_float * 10000;
    VTLogDebug("Deviation in Non-Repeating Signature Average Current Draw: %lu.%lu \r\n", decimal, frac);
#endif /* VT_LOG_LEVEL > 2 */
    return signature_drift;
}
