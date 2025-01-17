/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include "vt_cs_api.h"
#include "vt_cs_calibrate.h"
#include "vt_cs_database.h"
#include "vt_cs_raw_signature_read.h"
#include "vt_cs_sensor_status.h"
#include "vt_debug.h"


VT_VOID vt_currentsense_object_signature_read(VT_CURRENTSENSE_OBJECT* cs_object)
{
    VT_FLOAT sampling_frequencies[VT_CS_MAX_SIGNATURES];
    VT_UINT num_sampling_frqeuencies = 0;
    if (cs_object->mode == VT_MODE_RUNTIME_EVALUATE)
    {
        cs_fetch_template_repeating_signature_sampling_frequencies(
            cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
    }
    else
    {
        cs_calibrate_repeating_signatures_compute_sampling_frequencies(
            cs_object, sampling_frequencies, VT_CS_MAX_SIGNATURES, &num_sampling_frqeuencies);
    }
    cs_raw_signature_read(cs_object, sampling_frequencies, num_sampling_frqeuencies, VT_CS_SAMPLE_LENGTH);
}

VT_VOID vt_currentsense_object_signature_process(VT_CURRENTSENSE_OBJECT* cs_object)
{
    VTLogDebug("Signature processing started \r\n");
    cs_object->raw_signatures_reader->non_repeating_raw_signature_stop_collection = true;
    while (cs_object->raw_signatures_reader->repeating_raw_signature_ongoing_collection)
    {
        // do nothing, wait till repeating raw signatures are collected
    }
    switch (cs_object->mode)
    {
        case VT_MODE_RUNTIME_EVALUATE:
            VTLogDebug("Computing Sensor Status \r\n");
            cs_sensor_status(cs_object);
            break;

        case VT_MODE_CALIBRATE:
            VTLogDebug("Calibrating Sensor Fingerprint \r\n");
            cs_calibrate_sensor(cs_object);
            cs_object->mode = VT_MODE_RUNTIME_EVALUATE;
            break;

        case VT_MODE_RECALIBRATE:
            VTLogDebug("Recalibrating Sensor Fingerprint \r\n");
            cs_recalibrate_sensor(cs_object);
            cs_object->mode = VT_MODE_RUNTIME_EVALUATE;
            break;
    }
}