// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef RPC_COMMAND_LIST_H
#define RPC_COMMAND_LIST_H

#include <uuid/uuid.h>

#include <azure/az_core.h>
#include <azure/core/az_mqtt5_rpc.h>
#include <azure/core/az_mqtt5_rpc_client.h>

#include <azure/core/_az_cfg_prefix.h>

typedef struct rpc_command_item
{
    uuid_t correlation_id;
    az_mqtt5_rpc_client_invoke_req_event_data command_data;
    int32_t timeout_ms;
} rpc_command_item_t;

typedef void (*rpc_command_removed_callback_t)(rpc_command_item_t* item);

void rpc_command_list_reset_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size);

rpc_command_item_t* rpc_command_list_add_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, int32_t timeout_ms);

bool rpc_command_list_contains_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, az_span correlation_id);

void rpc_command_list_remove_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, az_span correlation_id);

void rpc_command_list_remove_expired_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, rpc_command_removed_callback_t item_removed_callback);

#define rpc_command_list_define(LIST_SIZE)                          \
    const int32_t RPC_COMMAND_LIST_SIZE = LIST_SIZE;                \
    static rpc_command_item_t _rpc_commands[LIST_SIZE];

#define rpc_command_list_reset() \
    rpc_command_list_reset_n(_rpc_commands, RPC_COMMAND_LIST_SIZE)

#define rpc_command_list_add(timeout_ms) \
    rpc_command_list_add_n(_rpc_commands, RPC_COMMAND_LIST_SIZE, timeout_ms)

#define rpc_command_list_contains(correlation_id) \
    rpc_command_list_contains_n(_rpc_commands, RPC_COMMAND_LIST_SIZE, correlation_id)

#define rpc_command_list_remove(correlation_id) \
    rpc_command_list_remove_n(_rpc_commands, RPC_COMMAND_LIST_SIZE, correlation_id)

#define rpc_command_list_remove_expired(item_removed_callback) \
    rpc_command_list_remove_expired_n(_rpc_commands, RPC_COMMAND_LIST_SIZE, item_removed_callback)

#include <azure/core/_az_cfg_suffix.h>

#endif // RPC_COMMAND_LIST_H