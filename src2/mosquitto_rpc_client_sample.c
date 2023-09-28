/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

/**
 * @file
 * @brief RPC client sample for Mosquitto MQTT
 *
 */

#include <az_log_listener.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>

#include <azure/az_core.h>
#include <azure/core/az_log.h>
#include <azure/core/az_mqtt5_rpc.h>
#include <azure/core/az_mqtt5_rpc_client.h>

#include "vehicle03_unlock.h"
#include "sample_config.h"

#include "rpc_command_list.h"

// User-defined parameters
static const az_span cert_path1 = AZ_SPAN_LITERAL_FROM_STR(CLIENT_CERTIFICATE_PATH);
static const az_span key_path1 = AZ_SPAN_LITERAL_FROM_STR(CLIENT_CERTIFICATE_KEY_PATH);
static const az_span client_id = AZ_SPAN_LITERAL_FROM_STR("mobile-app");
static const az_span username = AZ_SPAN_LITERAL_FROM_STR("mobile-app");
static const az_span hostname = AZ_SPAN_LITERAL_FROM_STR(HOSTNAME);
// static const az_span cert_path1 = AZ_SPAN_LITERAL_FROM_STR("<path to cert pem file>");
// static const az_span key_path1 = AZ_SPAN_LITERAL_FROM_STR("<path to cert key file>");
// static const az_span client_id = AZ_SPAN_LITERAL_FROM_STR("mobile-app");
// static const az_span username = AZ_SPAN_LITERAL_FROM_STR("mobile-app");
// static const az_span hostname = AZ_SPAN_LITERAL_FROM_STR("<hostname>");
static const az_span command_name = AZ_SPAN_LITERAL_FROM_STR("unlock");
static const az_span model_id = AZ_SPAN_LITERAL_FROM_STR("dtmi:rpc:samples:vehicle;1");
static const az_span content_type = AZ_SPAN_LITERAL_FROM_STR("application/json");
static const az_span subscription_topic_format = AZ_SPAN_LITERAL_FROM_STR(
    "vehicles/{serviceId}/commands/{executorId}/{name}/__for_{invokerId}");
static const az_span request_topic_format
    = AZ_SPAN_LITERAL_FROM_STR("vehicles/{serviceId}/commands/{executorId}/{name}");

rpc_command_list_define(5);

// Static memory allocation.
static char response_topic_buffer[256];
static char request_topic_buffer[256];
static char subscription_topic_buffer[256];
static uint8_t correlation_id_buffer[AZ_MQTT5_RPC_CORRELATION_ID_LENGTH];

// State variables
static az_mqtt5_connection mqtt_connection;
static az_context connection_context;

static az_mqtt5_rpc_client rpc_client;
static az_mqtt5_rpc_client_codec rpc_client_codec;

volatile bool sample_finished = false;

az_result mqtt_callback(az_mqtt5_connection* client, az_event event, const void* context);
void handle_response(az_span response_payload);

void az_platform_critical_error()
{
  printf(LOG_APP_ERROR "\x1B[31mPANIC!\x1B[0m\n");

  while (1)
    ;
}

void handle_response(az_span response_payload)
{
  printf(LOG_APP "Command response received: %s\n", az_span_ptr(response_payload));
}

/**
 * @brief MQTT client callback function for all clients
 * @note If you add other clients, you can add handling for their events here
 */
az_result mqtt_callback(az_mqtt5_connection* client, az_event event, const void* context)
{
  (void)client;
  (void)context;
  az_app_log_callback(event.type, AZ_SPAN_FROM_STR("APP/callback"));
  switch (event.type)
  {
    case AZ_MQTT5_EVENT_CONNECT_RSP:
    {
      az_mqtt5_connack_data* connack_data = (az_mqtt5_connack_data*)event.data;
      printf(LOG_APP "CONNACK: reason=%d\n", connack_data->connack_reason);
      LOG_AND_EXIT_IF_FAILED(az_mqtt5_rpc_client_subscribe_begin(&rpc_client));
      break;
    }

    case AZ_MQTT5_EVENT_DISCONNECT_RSP:
    {
      printf(LOG_APP "DISCONNECTED\n");
      sample_finished = true;
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_READY_IND:
    {
      az_mqtt5_rpc_client_codec* ready_rpc_client_codec = (az_mqtt5_rpc_client_codec*)event.data;
      if (ready_rpc_client_codec == &rpc_client_codec)
      {
        printf(LOG_APP "RPC Client Ready\n");
        // invoke any queued requests that couldn't be sent earlier?
      }
      else
      {
        printf(LOG_APP_ERROR "Unknown client ready\n");
      }
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_RSP:
    {
      az_mqtt5_rpc_client_rsp_event_data* recv_data
          = (az_mqtt5_rpc_client_rsp_event_data*)event.data;

      if (rpc_command_list_contains(recv_data->correlation_id))
      {
        if (recv_data->status != AZ_MQTT5_RPC_STATUS_OK)
        {
          printf(
              LOG_APP_ERROR "Error response received. Status :%d. Message :%s\n",
              recv_data->status,
              az_span_ptr(recv_data->error_message));
        }
        else
        {
          if (!az_span_is_content_equal(content_type, recv_data->content_type))
          {
            printf(
                LOG_APP_ERROR "Invalid content type. Expected: {%s} Actual: {%s}\n",
                az_span_ptr(content_type),
                az_span_ptr(recv_data->content_type));
          }
          else
          {
            // TODO: deserialize before passing to handle_response
            handle_response(recv_data->response_payload);
          }
        }

        rpc_command_list_remove(recv_data->correlation_id);
      }
      else
      {
        printf(LOG_APP_ERROR "Request with ");
        print_correlation_id(recv_data->correlation_id);
        printf("not found\n");
      }
      break;
    }

    case AZ_MQTT5_EVENT_RPC_CLIENT_ERROR_RSP:
    {
      az_mqtt5_rpc_client_rsp_event_data* recv_data
          = (az_mqtt5_rpc_client_rsp_event_data*)event.data;
      printf(LOG_APP_ERROR "Broker/Client failure for command ");
      print_correlation_id(recv_data->correlation_id);
      printf(": %s Status: %d\n", az_span_ptr(recv_data->error_message), recv_data->status);
      rpc_command_list_remove(recv_data->correlation_id);

      break;
    }

    case AZ_HFSM_EVENT_ERROR:
      printf(LOG_APP_ERROR "Error Event\n");
      break;

    default:
      break;
  }

  return AZ_OK;
}

void expired_rpc_command_removed_callback(rpc_command_item_t* item)
{
  printf("command %.*s expired\n", (int)sizeof(item->correlation_id), item->correlation_id);
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  rpc_command_list_reset();

  /* Required before calling other mosquitto functions */
  if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS)
  {
    printf(LOG_APP "Failed to initialize MosquittoLib\n");
    return -1;
  }

  printf(LOG_APP "Using MosquittoLib version %d\n", mosquitto_lib_version(NULL, NULL, NULL));

  az_log_set_message_callback(az_sdk_log_callback);
  az_log_set_classification_filter_callback(az_sdk_log_filter_callback);

  connection_context = az_context_create_with_expiration(
      &az_context_application, az_context_get_expiration(&az_context_application));

  az_mqtt5 mqtt5;
  struct mosquitto* mosq = NULL;

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_init(&mqtt5, &mosq, NULL));

  az_mqtt5_x509_client_certificate primary_credential = (az_mqtt5_x509_client_certificate){
    .cert = cert_path1,
    .key = key_path1,
  };

  az_mqtt5_connection_options connection_options = az_mqtt5_connection_options_default();
  connection_options.client_id_buffer = client_id;
  connection_options.username_buffer = username;
  connection_options.password_buffer = AZ_SPAN_EMPTY;
  connection_options.hostname = hostname;
  connection_options.client_certificates[0] = primary_credential;

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_connection_init(
      &mqtt_connection, &connection_context, &mqtt5, mqtt_callback, &connection_options, NULL));

  az_mqtt5_property_bag property_bag;
  mosquitto_property* mosq_prop = NULL;
  LOG_AND_EXIT_IF_FAILED(az_mqtt5_property_bag_init(&property_bag, &mqtt5, &mosq_prop));

  az_mqtt5_rpc_client_codec_options client_codec_options
      = az_mqtt5_rpc_client_codec_options_default();
  client_codec_options.subscription_topic_format = subscription_topic_format;
  client_codec_options.request_topic_format = request_topic_format;

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_rpc_client_init(
      &rpc_client,
      &rpc_client_codec,
      &mqtt_connection,
      property_bag,
      client_id,
      model_id,
      command_name,
      AZ_SPAN_FROM_BUFFER(response_topic_buffer),
      AZ_SPAN_FROM_BUFFER(request_topic_buffer),
      AZ_SPAN_FROM_BUFFER(subscription_topic_buffer),
      AZ_SPAN_FROM_BUFFER(correlation_id_buffer),
      &client_codec_options));

  LOG_AND_EXIT_IF_FAILED(az_mqtt5_connection_open(&mqtt_connection));

  // infinite execution loop
  for (int i = 0; !sample_finished; i++)
  {
    printf(LOG_APP "Waiting...\r");
    fflush(stdout);

    rpc_command_list_remove_expired(expired_rpc_command_removed_callback);

    // invokes a command every 15 seconds. This cadence/how it is triggered should be customized for
    // your solution.
    if (i % 15 == 0)
    {
      uuid_t new_uuid;
      uuid_generate(new_uuid);

      rpc_command_item_t* rpc_command = rpc_command_list_add(CLIENT_COMMAND_TIMEOUT_MS);

      if (rpc_command != NULL)
      {        
        az_result vehicle03_unlock_begin_result =
          vehicle03_unlock_begin(
            &rpc_client,
            &rpc_command->command_data,
            (const uint8_t*)rpc_command->correlation_id, sizeof(uuid_t),
            1691530585198, "mobile-app", strlen("mobile-app"));

        if (az_result_failed(vehicle03_unlock_begin_result))
        {
          printf("Failed invoking vehicle03/unlock");
        }
      }
    }

    LOG_AND_EXIT_IF_FAILED(az_platform_sleep_msec(1000));
  }

  // clean-up functions shown for completeness
  LOG_AND_EXIT_IF_FAILED(az_mqtt5_connection_close(&mqtt_connection));

  if (mosq != NULL)
  {
    mosquitto_loop_stop(mosq, false);
    mosquitto_destroy(mosq);
  }

  // mosquitto allocates the property bag for us, but we're responsible for free'ing it
  mosquitto_property_free_all(&mosq_prop);

  if (mosquitto_lib_cleanup() != MOSQ_ERR_SUCCESS)
  {
    printf(LOG_APP "Failed to cleanup MosquittoLib\n");
    return -1;
  }

  printf(LOG_APP "Done.                                \n");
  return 0;
}
