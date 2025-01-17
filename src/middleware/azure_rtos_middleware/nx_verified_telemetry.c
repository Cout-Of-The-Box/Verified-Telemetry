/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "nx_verified_telemetry.h"
#include "nx_vt_middleware_helper.h"
#include "vt_debug.h"

#define PROPERTY_NAME_MAX_LENGTH 50

static const CHAR enable_verified_telemetry_property[] = "enableVerifiedTelemetry";
static const CHAR device_status_property[]             = "deviceStatus";

static const CHAR temp_response_description_success[] = "success";
static const CHAR temp_response_description_failed[]  = "failed";

static VOID send_reported_property(NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr,
    const UCHAR* component_name_ptr,
    UINT component_name_length,
    bool status,
    INT status_code,
    UINT version,
    const CHAR* description,
    UCHAR* property_name,
    UINT property_name_length)
{
    UINT response_status;
    NX_AZURE_IOT_JSON_WRITER json_writer;

    if (nx_azure_iot_pnp_client_reported_properties_create(iotpnp_client_ptr, &json_writer, NX_WAIT_FOREVER))
    {
        VTLogError("Failed to build reported response\r\n");
        return;
    }

    if (nx_azure_iot_pnp_client_reported_property_component_begin(
            iotpnp_client_ptr, &json_writer, component_name_ptr, component_name_length) ||
        nx_azure_iot_pnp_client_reported_property_status_begin(iotpnp_client_ptr,
            &json_writer,
            (UCHAR*)property_name,
            property_name_length,
            status_code,
            version,
            (const UCHAR*)description,
            strlen(description)) ||
        nx_azure_iot_json_writer_append_bool(&json_writer, status) ||
        nx_azure_iot_pnp_client_reported_property_status_end(iotpnp_client_ptr, &json_writer) ||
        nx_azure_iot_pnp_client_reported_property_component_end(iotpnp_client_ptr, &json_writer))
    {

        VTLogError("Failed to build reported response\r\n");
    }
    else
    {
        if (nx_azure_iot_pnp_client_reported_properties_send(
                iotpnp_client_ptr, &json_writer, NX_NULL, &response_status, NX_NULL, (5 * NX_IP_PERIODIC_RATE)))
        {
            VTLogError("Failed to send reported response\r\n");
        }
    }
    nx_azure_iot_json_writer_deinit(&json_writer);
}

UINT nx_vt_device_status_property(
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, bool device_status, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr)
{
    UINT status;
    UINT response_status = 0;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    if ((status = nx_azure_iot_pnp_client_reported_properties_create(iotpnp_client_ptr, &json_writer, NX_WAIT_FOREVER)))
    {
        VTLogError("Failed create reported properties: error code = 0x%08x\r\n", status);
        return (status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_property_component_begin(iotpnp_client_ptr,
             &json_writer,
             verified_telemetry_DB->component_name_ptr,
             verified_telemetry_DB->component_name_length)) ||
        (status = nx_azure_iot_json_writer_append_property_with_bool_value(
             &json_writer, (const UCHAR*)device_status_property, sizeof(device_status_property) - 1, device_status)) ||
        (status = nx_azure_iot_pnp_client_reported_property_component_end(iotpnp_client_ptr, &json_writer)))
    {
        VTLogError("Failed to build reported property!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (status);
    }

    if ((status = nx_azure_iot_pnp_client_reported_properties_send(
             iotpnp_client_ptr, &json_writer, NX_NULL, &response_status, NX_NULL, (5 * NX_IP_PERIODIC_RATE))))
    {
        VTLogError("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_json_writer_deinit(&json_writer);
        return (status);
    }

    nx_azure_iot_json_writer_deinit(&json_writer);

    if ((response_status < 200) || (response_status >= 300))
    {
        VTLogError("device twin report properties failed with code : %d\r\n", response_status);
        return (NX_NOT_SUCCESSFUL);
    }

    return (status);
}

UINT nx_vt_process_command(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr,
    UCHAR* component_name_ptr,
    UINT component_name_length,
    UCHAR* pnp_command_name_ptr,
    UINT pnp_command_name_length,
    NX_AZURE_IOT_JSON_READER* json_reader_ptr,
    NX_AZURE_IOT_JSON_WRITER* json_response_ptr,
    UINT* status_code)
{

    UINT status             = 0;
    UINT iter               = 0;
    UINT components_num     = verified_telemetry_DB->components_num;
    void* component_pointer = verified_telemetry_DB->first_component;

    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
        {
            if ((status = nx_vt_fallcurve_process_command(&(((NX_VT_OBJECT*)component_pointer)->component.fc),
                     iotpnp_client_ptr,
                     component_name_ptr,
                     component_name_length,
                     pnp_command_name_ptr,
                     pnp_command_name_length,
                     json_reader_ptr,
                     json_response_ptr,
                     status_code)) == NX_AZURE_IOT_SUCCESS)
            {
                VTLogInfo("Successfully executed command %.*s on %.*s component\r\n\n",
                    pnp_command_name_length,
                    pnp_command_name_ptr,
                    component_name_length,
                    component_name_ptr);
                return NX_AZURE_IOT_SUCCESS;
            }
        }
        else if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            if ((status = nx_vt_currentsense_process_command(&(((NX_VT_OBJECT*)component_pointer)->component.cs),
                     iotpnp_client_ptr,
                     component_name_ptr,
                     component_name_length,
                     pnp_command_name_ptr,
                     pnp_command_name_length,
                     json_reader_ptr,
                     json_response_ptr,
                     status_code)) == NX_AZURE_IOT_SUCCESS)
            {
                VTLogInfo("Successfully executed command %.*s on %.*s component\r\n\n",
                    pnp_command_name_length,
                    pnp_command_name_ptr,
                    component_name_length,
                    component_name_ptr);
                return NX_AZURE_IOT_SUCCESS;
            }
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }

    return NX_NOT_SUCCESSFUL;
}

UINT nx_vt_process_property_update(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr,
    const UCHAR* component_name_ptr,
    UINT component_name_length,
    NX_AZURE_IOT_JSON_READER* name_value_reader_ptr,
    UINT version)
{
    UINT parsed_value = 0;
    INT status_code;
    const CHAR* description;
    CHAR property_name[PROPERTY_NAME_MAX_LENGTH];
    UINT property_name_length = 0;

    if (verified_telemetry_DB->component_name_length != component_name_length ||
        strncmp((CHAR*)verified_telemetry_DB->component_name_ptr, (CHAR*)component_name_ptr, component_name_length) != 0)
    {
        return (NX_NOT_SUCCESSFUL);
    }

    // Get Property Name
    nx_azure_iot_json_reader_token_string_get(
        name_value_reader_ptr, (UCHAR*)property_name, sizeof(property_name), &property_name_length);

    // Property 1: Enable Verified Telemetry
    if (nx_azure_iot_json_reader_token_is_text_equal(name_value_reader_ptr,
            (UCHAR*)enable_verified_telemetry_property,
            sizeof(enable_verified_telemetry_property) - 1) == NX_TRUE)
    {
        if (nx_azure_iot_json_reader_next_token(name_value_reader_ptr) ||
            nx_azure_iot_json_reader_token_bool_get(name_value_reader_ptr, &parsed_value))
        {
            status_code = 401;
            description = temp_response_description_failed;
        }
        else
        {
            status_code = 200;
            description = temp_response_description_success;

            verified_telemetry_DB->enable_verified_telemetry = (bool)parsed_value;
            VTLogInfo("Received Enable Verified Telemetry Twin update with value %s\r\n", (bool)parsed_value ? "true" : "false");
        }
        send_reported_property(iotpnp_client_ptr,
            component_name_ptr,
            component_name_length,
            (bool)parsed_value,
            status_code,
            version,
            description,
            (UCHAR*)property_name,
            property_name_length);
        return NX_AZURE_IOT_SUCCESS;
    }

    return NX_NOT_SUCCESSFUL;
}

UINT nx_vt_process_reported_property_sync(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr,
    const UCHAR* component_name_ptr,
    UINT component_name_length,
    NX_AZURE_IOT_JSON_READER* name_value_reader_ptr,
    UINT version)
{
    UINT iter               = 0;
    UINT components_num     = verified_telemetry_DB->components_num;
    void* component_pointer = verified_telemetry_DB->first_component;
    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
        {
            if (nx_vt_fallcurve_process_reported_property_sync(&(((NX_VT_OBJECT*)component_pointer)->component.fc),
                    iotpnp_client_ptr,
                    component_name_ptr,
                    component_name_length,
                    name_value_reader_ptr,
                    version) == NX_AZURE_IOT_SUCCESS)
            {
                return NX_AZURE_IOT_SUCCESS;
            }
        }
        else if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            if (nx_vt_currentsense_process_reported_property_sync(&(((NX_VT_OBJECT*)component_pointer)->component.cs),
                    iotpnp_client_ptr,
                    component_name_ptr,
                    component_name_length,
                    name_value_reader_ptr,
                    version) == NX_AZURE_IOT_SUCCESS)
            {
                return NX_AZURE_IOT_SUCCESS;
            }
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }
    return NX_NOT_SUCCESSFUL;
}

UINT nx_vt_send_desired_property_after_boot(
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr, UINT message_type)
{
    if (message_type == NX_AZURE_IOT_PNP_PROPERTIES)
    {
        send_reported_property(iotpnp_client_ptr,
            verified_telemetry_DB->component_name_ptr,
            verified_telemetry_DB->component_name_length,
            verified_telemetry_DB->enable_verified_telemetry,
            200,
            1,
            temp_response_description_success,
            (UCHAR*)enable_verified_telemetry_property,
            sizeof(enable_verified_telemetry_property) - 1);
    }
    return NX_AZURE_IOT_SUCCESS;
}

UINT nx_vt_properties(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, NX_AZURE_IOT_PNP_CLIENT* iotpnp_client_ptr)
{
    UINT status = 0;

    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;
    bool device_status             = true;
    for (UINT i = 0; i < components_num; i++)
    {

        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
        {
            if ((status = nx_vt_fallcurve_telemetry_status_property(
                     &(((NX_VT_OBJECT*)component_pointer)->component.fc), iotpnp_client_ptr, &device_status)))
            {
                VTLogError("Failed nx_vt_fallcurve_telemetry_status_property for component %.*s: error code = "
                           "0x%08x\r\n\n",
                    (INT)((NX_VT_OBJECT*)component_pointer)->component.fc.component_name_length,
                    (CHAR*)((NX_VT_OBJECT*)component_pointer)->component.fc.component_name_ptr,
                    status);
            }

            if (((NX_VT_OBJECT*)component_pointer)->component.fc.property_sent == 0)
            {
                if ((status = nx_vt_fallcurve_fingerprint_type_property(
                         &(((NX_VT_OBJECT*)component_pointer)->component.fc), iotpnp_client_ptr)))
                {
                    VTLogError("Failed nx_vt_fallcurve_fingerprint_type_property: error code = 0x%08x\r\n", status);
                }

                ((NX_VT_OBJECT*)component_pointer)->component.fc.property_sent = 1;
            }
        }
        else if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            if ((status = nx_vt_currentsense_reported_properties(&(((NX_VT_OBJECT*)component_pointer)->component.cs),
                     iotpnp_client_ptr,
                     &device_status,
                     enable_verified_telemetry)))
            {
                VTLogError("Failed nx_vt_currentsense_telemetry_status_property for component %.*s: error code = "
                           "0x%08x\r\n\n",
                    (INT)((NX_VT_OBJECT*)component_pointer)->component.cs.component_name_length,
                    (CHAR*)((NX_VT_OBJECT*)component_pointer)->component.cs.component_name_ptr,
                    status);
            }
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }

    if (device_status != verified_telemetry_DB->device_status || verified_telemetry_DB->device_status_property_sent == 0)
    {

        verified_telemetry_DB->device_status = device_status;
        if ((status = nx_vt_device_status_property(
                 (NX_VERIFIED_TELEMETRY_DB*)verified_telemetry_DB, device_status, iotpnp_client_ptr)))
        {
            VTLogError("Failed pnp_vt_device_status_property: error code = 0x%08x\r\n", status);
        }
        else if (verified_telemetry_DB->device_status_property_sent == 0)
        {
            verified_telemetry_DB->device_status_property_sent = 1;
        }
    }

    return status;
}

UINT nx_vt_init(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    UCHAR* component_name_ptr,
    bool enable_verified_telemetry,
    VT_DEVICE_DRIVER* device_driver,
    CHAR* scratch_buffer,
    UINT scratch_buffer_length)
{
    strncpy((CHAR*)verified_telemetry_DB->component_name_ptr,
        (CHAR*)component_name_ptr,
        sizeof(verified_telemetry_DB->component_name_ptr));
    verified_telemetry_DB->component_name_length       = strlen((const char*)component_name_ptr);
    verified_telemetry_DB->enable_verified_telemetry   = enable_verified_telemetry;
    verified_telemetry_DB->device_status_property_sent = false;
    verified_telemetry_DB->components_num              = 0;
    verified_telemetry_DB->first_component             = NULL;
    verified_telemetry_DB->last_component              = NULL;

    verified_telemetry_DB->device_driver         = device_driver;
    verified_telemetry_DB->scratch_buffer        = scratch_buffer;
    verified_telemetry_DB->scratch_buffer_length = scratch_buffer_length;

    return NX_AZURE_IOT_SUCCESS;
}

UINT nx_vt_signature_init(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    NX_VT_OBJECT* handle,
    UCHAR* component_name_ptr,
    UINT signature_type,
    UCHAR* associated_telemetry,
    bool telemetry_status_auto_update,
    VT_SENSOR_HANDLE* sensor_handle)
{
    if (signature_type != VT_SIGNATURE_TYPE_FALLCURVE)
    {
        return NX_AZURE_IOT_FAILURE;
    }
    verified_telemetry_DB->components_num++;
    if (verified_telemetry_DB->first_component == NULL)
    {
        verified_telemetry_DB->first_component = (void*)handle;
    }
    else
    {
        ((NX_VT_OBJECT*)(verified_telemetry_DB->last_component))->next_component = (void*)handle;
    }
    verified_telemetry_DB->last_component = (void*)handle;

    handle->next_component = NULL;
    handle->signature_type = signature_type;
    if (signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
    {
        nx_vt_fallcurve_init(&(handle->component.fc),
            component_name_ptr,
            verified_telemetry_DB->device_driver,
            sensor_handle,
            associated_telemetry,
            telemetry_status_auto_update);
    }
    else if (signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
    {
        nx_vt_currentsense_init(&(handle->component.cs),
            component_name_ptr,
            verified_telemetry_DB->device_driver,
            sensor_handle,
            associated_telemetry,
            verified_telemetry_DB->scratch_buffer,
            verified_telemetry_DB->scratch_buffer_length);
    }
    return NX_AZURE_IOT_SUCCESS;
}

UINT nx_vt_verified_telemetry_message_create_send(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB,
    NX_AZURE_IOT_PNP_CLIENT* pnp_client_ptr,
    const UCHAR* component_name_ptr,
    UINT component_name_length,
    UINT wait_option,
    const UCHAR* telemetry_data,
    UINT data_size)
{
    UINT status;
    NX_PACKET* packet_ptr;
    NX_AZURE_IOT_JSON_READER json_reader;
    NX_AZURE_IOT_JSON_READER json_reader_copy;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    UCHAR property_name[PROPERTY_NAME_MAX_LENGTH];
    memset(property_name, 0, sizeof(property_name));
    UINT bytes_copied = 0;
    CHAR vt_property_name[PROPERTY_NAME_MAX_LENGTH];
    memset(vt_property_name, 0, sizeof(vt_property_name));
    memset(verified_telemetry_DB->scratch_buffer, 0, verified_telemetry_DB->scratch_buffer_length);
    UINT token_found                = 0;
    UCHAR* token_pointer            = NULL;
    UINT tokens                     = 0;
    UINT str_buffer_space_available = 0;

    nx_azure_iot_json_reader_with_buffer_init(&json_reader, telemetry_data, data_size);

    for (UINT i = 0; i < components_num; i++)
    {
        json_reader_copy = json_reader;
        token_found      = 0;
        while (token_found == 0 && nx_azure_iot_json_reader_next_token(&json_reader_copy) == NX_AZURE_IOT_SUCCESS)
        {
            nx_azure_iot_json_reader_token_string_get(&json_reader_copy, property_name, sizeof(property_name), &bytes_copied);
            if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
            {
                if ((token_pointer = (UCHAR*)strstr(
                         (CHAR*)(((NX_VT_OBJECT*)component_pointer)->component.fc.associated_telemetry), (CHAR*)property_name)))
                {
                    snprintf(vt_property_name, sizeof(vt_property_name), "vT");
                    strcat(vt_property_name, (CHAR*)token_pointer);
                    if (tokens > 0)
                    {
                        str_buffer_space_available =
                            verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                        strncat(verified_telemetry_DB->scratch_buffer, "&", str_buffer_space_available);
                    }
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer, vt_property_name, str_buffer_space_available);
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer, "=", str_buffer_space_available);
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer,
                        (((NX_VT_OBJECT*)component_pointer)->component.fc.telemetry_status > 0) ? "true" : "false",
                        str_buffer_space_available);
                    token_found = 1;
                    tokens++;
                }
            }
            else if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
            {
                if ((token_pointer = (UCHAR*)strstr(
                         (CHAR*)(((NX_VT_OBJECT*)component_pointer)->component.cs.associated_telemetry), (CHAR*)property_name)))
                {
                    snprintf(vt_property_name, sizeof(vt_property_name), "vT");
                    strcat(vt_property_name, (CHAR*)token_pointer);
                    if (tokens > 0)
                    {
                        str_buffer_space_available =
                            verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                        strncat(verified_telemetry_DB->scratch_buffer, "&", str_buffer_space_available);
                    }
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer, vt_property_name, str_buffer_space_available);
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer, "=", str_buffer_space_available);
                    str_buffer_space_available =
                        verified_telemetry_DB->scratch_buffer_length - (strlen(verified_telemetry_DB->scratch_buffer) + 1);
                    strncat(verified_telemetry_DB->scratch_buffer,
                        (nx_vt_currentsense_fetch_telemetry_status(
                             &(((NX_VT_OBJECT*)component_pointer)->component.cs), enable_verified_telemetry) == true)
                            ? "true"
                            : "false",
                        str_buffer_space_available);
                    token_found = 1;
                    tokens++;
                }
            }
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }
    VTLogInfo("Attaching Telemetry Message Properties: %.*s \r\n",
        strlen(verified_telemetry_DB->scratch_buffer),
        verified_telemetry_DB->scratch_buffer);
    /* Create a telemetry message packet. */
    if ((status = nx_azure_iot_pnp_client_telemetry_message_create_with_message_property(pnp_client_ptr,
             component_name_ptr,
             component_name_length,
             &packet_ptr,
             wait_option,
             (UCHAR*)verified_telemetry_DB->scratch_buffer,
             strlen(verified_telemetry_DB->scratch_buffer))))
    {
        printf("Telemetry message with message properties create failed!: error code = 0x%08x\r\n", status);
        return (status);
    }
    if ((status = nx_azure_iot_pnp_client_telemetry_send(pnp_client_ptr, packet_ptr, telemetry_data, data_size, wait_option)))
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_pnp_client_telemetry_message_delete(packet_ptr);
        return (status);
    }
    return (status);
}

UINT nx_vt_compute_evaluate_fingerprint_all_sensors(NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB)
{
    UINT status                    = 0;
    UINT iter                      = 0;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;
    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
        {
            status = status || nx_vt_fallcurve_compute_sensor_status_global(
                                   &(((NX_VT_OBJECT*)component_pointer)->component.fc), enable_verified_telemetry);
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }
    return status;
}

UINT nx_vt_azure_iot_pnp_client_component_add(
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, NX_AZURE_IOT_PNP_CLIENT* pnp_client_ptr)
{
    UINT status             = 0;
    UINT iter               = 0;
    UINT components_num     = verified_telemetry_DB->components_num;
    void* component_pointer = verified_telemetry_DB->first_component;

    status =
        status || nx_azure_iot_pnp_client_component_add(
                      pnp_client_ptr, verified_telemetry_DB->component_name_ptr, verified_telemetry_DB->component_name_length);

    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_FALLCURVE)
        {
            status = status || nx_azure_iot_pnp_client_component_add(pnp_client_ptr,
                                   ((NX_VT_OBJECT*)component_pointer)->component.fc.component_name_ptr,
                                   ((NX_VT_OBJECT*)component_pointer)->component.fc.component_name_length);
        }
        else if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            status = status || nx_azure_iot_pnp_client_component_add(pnp_client_ptr,
                                   ((NX_VT_OBJECT*)component_pointer)->component.cs.component_name_ptr,
                                   ((NX_VT_OBJECT*)component_pointer)->component.cs.component_name_length);
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }

    return status;
}

UINT nx_vt_signature_read(
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length)
{
    UINT status                    = 0;
    UINT iter                      = 0;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;

    if (!enable_verified_telemetry)
    {
        return (NX_AZURE_IOT_FAILURE);
    }

    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            status = status || nx_vt_currentsense_signature_read(&(((NX_VT_OBJECT*)component_pointer)->component.cs),
                                   associated_telemetry,
                                   associated_telemetry_length,
                                   enable_verified_telemetry);
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }
    return status;
}

UINT nx_vt_signature_process(
    NX_VERIFIED_TELEMETRY_DB* verified_telemetry_DB, UCHAR* associated_telemetry, UINT associated_telemetry_length)
{
    UINT status                    = 0;
    UINT iter                      = 0;
    UINT components_num            = verified_telemetry_DB->components_num;
    void* component_pointer        = verified_telemetry_DB->first_component;
    bool enable_verified_telemetry = verified_telemetry_DB->enable_verified_telemetry;

    if (!enable_verified_telemetry)
    {
        return (NX_AZURE_IOT_FAILURE);
    }

    for (iter = 0; iter < components_num; iter++)
    {
        if (((NX_VT_OBJECT*)component_pointer)->signature_type == VT_SIGNATURE_TYPE_CURRENTSENSE)
        {
            status = status || nx_vt_currentsense_signature_process(&(((NX_VT_OBJECT*)component_pointer)->component.cs),
                                   associated_telemetry,
                                   associated_telemetry_length,
                                   enable_verified_telemetry);
        }
        component_pointer = (((NX_VT_OBJECT*)component_pointer)->next_component);
    }
    return status;
}