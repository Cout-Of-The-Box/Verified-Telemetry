/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include "vt_cs_database.h"
#include "vt_debug.h"

VT_UINT cs_store_repeating_signature_feature_vector(VT_CURRENTSENSE_OBJECT* cs_object,
    VT_FLOAT sampling_frequency,
    VT_FLOAT signature_frequency,
    VT_FLOAT duty_cycle,
    VT_FLOAT relative_current_draw)
{
    VT_UINT num_signatures = cs_object->fingerprintdb.template.repeating_signatures.num_signatures;
    if ((num_signatures + 1) >= VT_CS_MAX_SIGNATURES)
    {
        return VT_ERROR;
    }

    cs_object->fingerprintdb.template.repeating_signatures.signatures[num_signatures].sampling_freq      = sampling_frequency;
    cs_object->fingerprintdb.template.repeating_signatures.signatures[num_signatures].signature_freq     = signature_frequency;
    cs_object->fingerprintdb.template.repeating_signatures.signatures[num_signatures].duty_cycle         = duty_cycle;
    cs_object->fingerprintdb.template.repeating_signatures.signatures[num_signatures].relative_curr_draw = relative_current_draw;
    cs_object->fingerprintdb.template.repeating_signatures.num_signatures++;

    cs_object->db_updated                  = VT_DB_UPDATED;
    cs_object->fingerprintdb.template_type = VT_CS_REPEATING_SIGNATURE;
    VTLogDebug("Number of Repeating Signatures stored in DB %d\r\n",
        cs_object->fingerprintdb.template.repeating_signatures.num_signatures);
    return VT_SUCCESS;
}

VT_UINT cs_update_repeating_signature_offset_current_draw(
    VT_CURRENTSENSE_OBJECT* cs_object, VT_FLOAT lowest_sample_freq, VT_FLOAT offset_curr)
{
    cs_object->fingerprintdb.template.repeating_signatures.lowest_sample_freq = lowest_sample_freq;
    cs_object->fingerprintdb.template.repeating_signatures.offset_current     = offset_curr;

    cs_object->db_updated                  = VT_DB_UPDATED;
    cs_object->fingerprintdb.template_type = VT_CS_REPEATING_SIGNATURE;
    VTLogDebug("Modified Offset Current for Repeating Signatures value stored in DB \r\n");
    return VT_SUCCESS;
}

VT_UINT cs_update_non_repeating_signature_average_current_draw(
    VT_CURRENTSENSE_OBJECT* cs_object, VT_FLOAT avg_curr_on, VT_FLOAT avg_curr_off)
{
    cs_object->fingerprintdb.template.non_repeating_signature.avg_curr_on  = avg_curr_on;
    cs_object->fingerprintdb.template.non_repeating_signature.avg_curr_off = avg_curr_off;

    cs_object->db_updated                  = VT_DB_UPDATED;
    cs_object->fingerprintdb.template_type = VT_CS_NON_REPEATING_SIGNATURE;
    VTLogDebug("Modified Average Current for Non-Repeating Signatures value stored in DB \r\n");
    return VT_SUCCESS;
}