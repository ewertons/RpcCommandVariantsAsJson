/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef RPC_CLIENT_CONFIG_H
#define RPC_CLIENT_CONFIG_H

#include <stdlib.h>
#include <azure/az_core.h>

#define CLIENT_ID "mobile-app"

#define MODEL_ID "dtmi:rpc:samples:vehicle;1"

#define USERNAME "mobile-app"

#define HOSTNAME "localhost"

#define CLIENT_CERTIFICATE_PATH "/home/ewertons/code/azure-sdk-for-c-hfsm/cmake/sdk/samples/core/mobile-app.pem"

#define CLIENT_CERTIFICATE_KEY_PATH "/home/ewertons/code/azure-sdk-for-c-hfsm/cmake/sdk/samples/core/mobile-app.key"

#define COMMAND_NAME "unlock"

#define SERVER_CLIENT_ID "vehicle03"

#define SUBSCRIPTION_TOPIC_FORMAT "vehicles/{serviceId}/commands/{executorId}/{name}/__for_{invokerId}"

#define REQUEST_TOPIC_FORMAT "vehicles/{serviceId}/commands/{executorId}/{name}"

#endif // RPC_CLIENT_CONFIG_H
 