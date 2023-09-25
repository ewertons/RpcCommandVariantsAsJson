/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef AZURE_RPC_CLIENT_H
#define AZURE_RPC_CLIENT_H

#include <stdlib.h>
#include <inttypes.h>

#include <azure/az_core.h>
#include <azure/core/az_mqtt5_rpc_client.h>

typedef struct azure_rpc_connection_options_struct
{
    /**
     * @brief    Expiration of the connection context in Unix Time.
     */
    int64_t expiration;
} azure_rpc_connection_context_options_t;

typedef struct azure_rpc_client_config_struct
{
    /**
     * @brief    Options for the connection context.
     */
    azure_rpc_connection_context_options_t connection_context;
    az_span model_id;
    az_span command_name;
    az_mqtt5_connection_options connection_options;
    az_span response_topic_buffer;
    az_span request_topic_buffer;
    az_span subscription_topic_buffer;
} azure_rpc_client_config_t;

typedef struct azure_rpc_client_struct
{
    azure_rpc_client_config_t config;
    az_context connection_context;
    az_mqtt5 mqtt5;
    struct mosquitto* mosquitto_client;
    az_mqtt5_connection mqtt_connection;

    az_mqtt5_property_bag property_bag;
    mosquitto_property* mosq_prop;

    az_mqtt5_rpc_client_policy rpc_client_policy;
    az_mqtt5_rpc_client rpc_client;
} azure_rpc_client_t;

AZ_NODISCARD azure_rpc_client_config_t azure_rpc_client_config_get_default();

AZ_NODISCARD int azure_rpc_platform_initialize();

void azure_rpc_platform_deinitialize();

AZ_NODISCARD int azure_rpc_client_initialize(azure_rpc_client_t* client, azure_rpc_client_config_t config);

AZ_NODISCARD int azure_rpc_client_start(azure_rpc_client_t* client);

AZ_NODISCARD int azure_rpc_client_invoke(azure_rpc_client_t* client, az_span server_client_id, az_span correlation_id, az_span payload, az_span content_type);

void azure_rpc_client_deinitialize(azure_rpc_client_t* client);

#endif // AZURE_RPC_CLIENT_H