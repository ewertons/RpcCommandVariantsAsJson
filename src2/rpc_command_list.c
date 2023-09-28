// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "rpc_command_list.h"

void rpc_command_list_reset_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size)
{
    for (int i = 0; i < rpc_commands_size; i++)
    {
        rpc_commands[i].timeout_ms = -1;
    }
}

rpc_command_item_t* rpc_command_list_add_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, int32_t timeout_ms)
{
    for (int i = 0; i < rpc_commands_size; i++)
    {
        if (rpc_commands[i].timeout_ms == -1)
        {
            uuid_generate(rpc_commands[i].correlation_id);
            rpc_commands[i].timeout_ms = timeout_ms;
            return &rpc_commands[i];
        }
    }

    return NULL;
}

bool rpc_command_list_contains_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, az_span correlation_id)
{
    for (int i = 0; i < rpc_commands_size; i++)
    {
        if (rpc_commands[i].timeout_ms != -1 &&
            az_span_is_content_equal(
                correlation_id,
                az_span_create(rpc_commands[i].correlation_id, sizeof(uuid_t))
            ))
        {
            return true;
        }
    }

    return false;
}

void rpc_command_list_remove_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, az_span correlation_id)
{
    for (int i = 0; i < rpc_commands_size; i++)
    {
        if (rpc_commands[i].timeout_ms != -1 &&
            az_span_is_content_equal(
                correlation_id,
                az_span_create(rpc_commands[i].correlation_id, sizeof(uuid_t))
            ))
        {
            rpc_commands[i].timeout_ms = -1;
            return;
        }
    }
}

void rpc_command_list_remove_expired_n(rpc_command_item_t* rpc_commands, int32_t rpc_commands_size, rpc_command_removed_callback_t item_removed_callback)
{
    for (int i = 0; i < rpc_commands_size; i++)
    {
        if (rpc_commands[i].timeout_ms == 1000)
        {
            item_removed_callback(&rpc_commands[i]);

            rpc_commands[i].timeout_ms = -1;
        }
    }
}
