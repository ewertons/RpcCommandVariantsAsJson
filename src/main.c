/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <stdio.h>

#include "rpc_client_config.h"

#include <azure/az_core.h>
#include <azure/core/az_log.h>
#include <azure/core/az_mqtt5_rpc.h>
#include <azure/core/az_mqtt5_rpc_client.h>

#include "azure_rpc_client.h"

#ifdef _WIN32
// Required for Sleep(DWORD)
#include <Windows.h>
#else
// Required for sleep(unsigned int)
#include <unistd.h>
#endif

// Static memory allocation.
static uint8_t response_topic_buffer[256];
static uint8_t request_topic_buffer[256];
static uint8_t subscription_topic_buffer[256];
static uint8_t correlation_id_buffer[AZ_MQTT5_RPC_CORRELATION_ID_LENGTH];

static azure_rpc_client_t azure_rpc_client;

static bool az_sdk_log_filter_callback(az_log_classification classification)
{
  (void)classification;
  // Enable all logging.
  return true;
}

static void az_sdk_log_callback(az_log_classification classification, az_span message)
{
    printf("%.*s\n", az_span_size(message), az_span_ptr(message));
}

static int azure_rpc_invoke_vehicle03_unlock(azure_rpc_client_t* client, az_span correlation_id, az_span request_payload)
{
    az_span server_client_id = AZ_SPAN_LITERAL_FROM_STR(SERVER_CLIENT_ID);  
    az_span request_payload_content_type = AZ_SPAN_LITERAL_FROM_STR("application/json");

    return azure_rpc_client_invoke(client, server_client_id, correlation_id, request_payload, request_payload_content_type);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    az_log_set_message_callback(az_sdk_log_callback);
    az_log_set_classification_filter_callback(az_sdk_log_filter_callback);

    azure_rpc_client_config_t azure_rpc_client_config = azure_rpc_client_config_get_default();
    azure_rpc_client_config.connection_options.client_id_buffer = AZ_SPAN_FROM_STR(CLIENT_ID);
    azure_rpc_client_config.connection_options.hostname = AZ_SPAN_FROM_STR(HOSTNAME);
    azure_rpc_client_config.connection_options.username_buffer = AZ_SPAN_FROM_STR(USERNAME);
    azure_rpc_client_config.connection_options.client_certificates[0] =
        (az_mqtt5_x509_client_certificate){ .cert = AZ_SPAN_FROM_STR(CLIENT_CERTIFICATE_PATH), .key = AZ_SPAN_FROM_STR(CLIENT_CERTIFICATE_KEY_PATH) };
    azure_rpc_client_config.model_id = AZ_SPAN_FROM_STR(MODEL_ID);
    azure_rpc_client_config.command_name = AZ_SPAN_FROM_STR(COMMAND_NAME);
    azure_rpc_client_config.response_topic_buffer = AZ_SPAN_FROM_BUFFER(response_topic_buffer);
    azure_rpc_client_config.request_topic_buffer = AZ_SPAN_FROM_BUFFER(request_topic_buffer);
    azure_rpc_client_config.subscription_topic_buffer = AZ_SPAN_FROM_BUFFER(subscription_topic_buffer);
    azure_rpc_client_config.correlation_id_buffer = AZ_SPAN_FROM_BUFFER(correlation_id_buffer);
    azure_rpc_client_config.client_codec_options.request_topic_format = AZ_SPAN_FROM_STR(REQUEST_TOPIC_FORMAT);
    azure_rpc_client_config.client_codec_options.subscription_topic_format = AZ_SPAN_FROM_STR(SUBSCRIPTION_TOPIC_FORMAT);

    if (azure_rpc_platform_initialize() != 0)
    {
        printf("Failed initializing Azure RPC platform\n");
        return __LINE__;
    }

    if (azure_rpc_client_initialize(&azure_rpc_client, azure_rpc_client_config) != 0)
    {
        printf("Failed initializing Azure RPC Client\n");
        return __LINE__;
    }

    if (azure_rpc_client_start(&azure_rpc_client) != 0)
    {
        printf("Failed starting Azure RPC Client\n");
        return __LINE__;
    }

    
    az_span correlation_id = AZ_SPAN_LITERAL_FROM_STR("Acde070d162843c1");
    az_span request_payload = AZ_SPAN_LITERAL_FROM_STR("{\"RequestTimestamp\":1691530585198,\"RequestedFrom\":\"mobile-app\"}");

    while (1)
    {
        if (azure_rpc_invoke_vehicle03_unlock(&azure_rpc_client, correlation_id, request_payload) != 0)
        {
        }
        sleep(3);
    }

    azure_rpc_client_deinitialize(&azure_rpc_client);
    azure_rpc_platform_deinitialize();
    return 0;
}
