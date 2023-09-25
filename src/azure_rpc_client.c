/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include "azure_rpc_client.h"

#include <azure/core/az_log.h>

#include "mosquitto.h"

// TODO: change this to use az_log listener
#define LOG_AND_EXIT_IF_FAILED(exp)                                        \
  do                                                                       \
  {                                                                        \
    az_result const _az_result = (exp);                                    \
    if (az_result_failed(_az_result))                                      \
    {                                                                      \
      printf(                                                              \
          "%s failed with error \x1B[31m0x%x\x1B[0m\n", \
          #exp,                                                            \
          _az_result);                                                     \
      return (int)_az_result;                                              \
    }                                                                      \
  } while (0)

az_result mqtt_callback(az_mqtt5_connection* client, az_event event);

AZ_NODISCARD azure_rpc_client_config_t azure_rpc_client_config_get_default()
{
  return (azure_rpc_client_config_t) {
    .connection_context.expiration = az_context_get_expiration(&az_context_application),
    .connection_options = az_mqtt5_connection_options_default()
  };
}

AZ_NODISCARD int azure_rpc_platform_initialize()
{
  if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

void azure_rpc_platform_deinitialize()
{
  if (mosquitto_lib_cleanup() != MOSQ_ERR_SUCCESS)
  {
    // TODO: consider returning a result in this function.
    // return -1;
  }
}

AZ_NODISCARD int azure_rpc_client_initialize(azure_rpc_client_t* client, const azure_rpc_client_config_t config)
{
  // TODO: preconditions.
  (void)memset(client, 0, sizeof(azure_rpc_client_t));

  client->config = config;

  client->connection_context = az_context_create_with_expiration(
    &az_context_application, config.connection_context.expiration);

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_init(&client->mqtt5, &client->mosquitto_client, NULL));

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_connection_init(
    &client->mqtt_connection, &client->connection_context, &client->mqtt5, mqtt_callback, &config.connection_options));

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_property_bag_init(&client->property_bag, &client->mqtt5, &client->mosq_prop));

  LOG_AND_EXIT_IF_FAILED(az_rpc_client_policy_init(
      &client->rpc_client_policy,
      &client->rpc_client,
      &client->mqtt_connection,
      client->property_bag,
      config.connection_options.client_id_buffer,
      config.model_id,
      config.command_name,
      config.response_topic_buffer,
      config.request_topic_buffer,
      config.subscription_topic_buffer,
      NULL));

  return 0;
}

AZ_NODISCARD int azure_rpc_client_start(azure_rpc_client_t* client)
{
    LOG_AND_EXIT_IF_FAILED(az_mqtt5_connection_open(&client->mqtt_connection));

    // TODO: remove this comment. Originally this call was inside the mqtt callback. Search for 454567 for reference.
    LOG_AND_EXIT_IF_FAILED(az_mqtt5_rpc_client_subscribe_begin(&client->rpc_client_policy));

    return 0;
}

AZ_NODISCARD int azure_rpc_client_invoke(azure_rpc_client_t* client, az_span server_client_id, az_span correlation_id, az_span payload, az_span content_type)
{
    az_mqtt5_rpc_client_invoke_req_event_data command_data
        = { .correlation_id = correlation_id,
            .content_type = content_type,
            .rpc_server_client_id = server_client_id,
            .request_payload = payload };

    az_result rc = az_mqtt5_rpc_client_invoke_begin(&client->rpc_client_policy, &command_data);

    if (az_result_failed(rc))
    {
      printf(
          "Failed to invoke command '%*.s' with rc: 0x%03x\n",
          az_span_size(correlation_id),
          az_span_ptr(correlation_id),
          rc);
      return __LINE__;
    }

    return 0;
}

void azure_rpc_client_deinitialize(azure_rpc_client_t* client)
{
    (void)(az_mqtt5_connection_close(&client->mqtt_connection));
    mosquitto_loop_stop(client->mosquitto_client, false);
    mosquitto_destroy(client->mosquitto_client);
    mosquitto_property_free_all(&client->mosq_prop);
}

// Required by platform/az_mqtt5_mosquitto
void az_platform_critical_error()
{
  // TODO: use az_log
//   printf("\x1B[31mPANIC!\x1B[0m\n");

  while (1);
}

/**
 * @brief MQTT client callback function for all clients
 * @note If you add other clients, you can add handling for their events here
 */
az_result mqtt_callback(az_mqtt5_connection* client, az_event event)
{
  (void)client;
//   az_app_log_callback(event.type, AZ_SPAN_FROM_STR("APP/callback"));

  switch (event.type)
  {
    case AZ_MQTT5_EVENT_CONNECT_RSP:
    {
    //   az_mqtt5_connack_data* connack_data = (az_mqtt5_connack_data*)event.data;
    //   printf(LOG_APP "CONNACK: %d\n", connack_data->connack_reason);
    // TODO: remove this comment: reference 454567
      // LOG_AND_EXIT_IF_FAILED(az_mqtt5_rpc_client_subscribe_begin(&rpc_client_policy));
      break;
    }

    case AZ_MQTT5_EVENT_DISCONNECT_RSP:
    {
    //   printf(LOG_APP "DISCONNECTED\n");
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_READY_IND:
    {
    //   az_mqtt5_rpc_client* ready_rpc_client = (az_mqtt5_rpc_client*)event.data;
    //   if (ready_rpc_client == &rpc_client)
    //   {
    //     printf(LOG_APP "RPC Client Ready\n");
    //     // invoke any queued requests that couldn't be sent earlier?
    //   }
    //   else
    //   {
    //     printf(LOG_APP_ERROR "Unknown client ready\n");
    //   }
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_RSP:
    {
    //   az_mqtt5_rpc_client_rsp_event_data* recv_data
    //       = (az_mqtt5_rpc_client_rsp_event_data*)event.data;
    //   if (is_command_pending(pending_commands, recv_data->correlation_id))
    //   {
    //     if (recv_data->status != AZ_MQTT5_RPC_STATUS_OK)
    //     {
    //       printf(
    //           LOG_APP_ERROR "Error response received. Status :%d. Message :%s\n",
    //           recv_data->status,
    //           az_span_ptr(recv_data->error_message));
    //     }
    //     else
    //     {
    //       if (!az_span_is_content_equal(content_type, recv_data->content_type))
    //       {
    //         printf(
    //             LOG_APP_ERROR "Invalid content type. Expected: {%s} Actual: {%s}\n",
    //             az_span_ptr(content_type),
    //             az_span_ptr(recv_data->content_type));
    //       }
    //       else
    //       {
    //         // TODO: deserialize
    //         printf(
    //             LOG_APP "Command response received: %s\n",
    //             az_span_ptr(recv_data->response_payload));
    //       }
    //     }
    //     remove_command(&pending_commands, recv_data->correlation_id);
    //   }
    //   else
    //   {
    //     printf(LOG_APP_ERROR "Request with ");
    //     print_correlation_id(recv_data->correlation_id);
    //     printf("not found\n");
    //   }
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_ERROR_RSP:
    {
    //   az_mqtt5_rpc_client_rsp_event_data* recv_data
    //       = (az_mqtt5_rpc_client_rsp_event_data*)event.data;
    //   printf(LOG_APP_ERROR "Broker/Client failure for command ");
    //   print_correlation_id(recv_data->correlation_id);
    //   printf(": %s Status: %d\n", az_span_ptr(recv_data->error_message), recv_data->status);
    //   remove_command(&pending_commands, recv_data->correlation_id);
      break;
    }

    case AZ_HFSM_EVENT_ERROR:
      // printf(LOG_APP_ERROR "Error Event\n");
      // az_platform_critical_error();
      break;

    default:
      break;
  }

  return AZ_OK;
}
