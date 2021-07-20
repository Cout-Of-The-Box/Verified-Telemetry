/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

/** @file */ 

#ifndef NX_VT_MIDDLEWARE_HELPER_H
#define NX_VT_MIDDLEWARE_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nx_azure_iot_pnp_client.h"

/**
 * @brief Return the next reported property in the property document passed.
 *
 * @param[in] pnp_client_ptr A pointer to a #NX_AZURE_IOT_PNP_CLIENT.
 * @param[in] reader_ptr A pointer to a #NX_AZURE_IOT_JSON_READER containing properties document
 * @param[in] message_type Type of document, only valid value are NX_AZURE_IOT_PNP_DESIRED_PROPERTIES or
 * NX_AZURE_IOT_PNP_PROPERTIES
 * @param[out] component_pptr A pointer to component name for the property returned using name_value_reader_ptr
 * @param[out] component_len_ptr Length of the component name
 * @param[out] name_value_reader_ptr A pointer to a #NX_AZURE_IOT_JSON_READER containing property name and value
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if next desired property is found.
 */
UINT nx_azure_iot_pnp_client_reported_component_property_value_next(NX_AZURE_IOT_PNP_CLIENT* pnp_client_ptr,
    NX_AZURE_IOT_JSON_READER* reader_ptr,
    UINT message_type,
    const UCHAR** component_pptr,
    UINT* component_len_ptr,
    NX_AZURE_IOT_JSON_READER* name_value_reader_ptr);

UINT nx_azure_iot_pnp_client_telemetry_message_create_with_message_property(NX_AZURE_IOT_PNP_CLIENT* pnp_client_ptr,
    const UCHAR* component_name_ptr,
    UINT component_name_length,
    NX_PACKET** packet_pptr,
    UINT wait_option,
    UCHAR* message_properties_buffer,
    UINT message_properties_buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* NX_VT_MIDDLEWARE_HELPER_H */