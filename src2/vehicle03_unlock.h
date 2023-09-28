#ifndef VEHICLE03_UNLOCK_H
#define VEHICLE03_UNLOCK_H

#include <azure/az_core.h>
#include <azure/core/az_mqtt5_rpc.h>
#include <azure/core/az_mqtt5_rpc_client.h>

static const az_span vehicle03_unlock_command_name = AZ_SPAN_LITERAL_FROM_STR("unlock"); // TODO: Scope: command
static const az_span server_client_id = AZ_SPAN_LITERAL_FROM_STR("vehicle03"); // TODO: Scope: model
static const az_span vehicle03_unlock_payload_content_type = AZ_SPAN_FROM_STR("application/json"); // TODO: Scope: model

AZ_INLINE az_result vehicle03_unlock_begin(
    az_mqtt5_rpc_client* rpc_client,
    const uint8_t* correlation_id, int32_t correlation_id_size,
    int32_t request_timestamp,
    const uint8_t* requested_from, int32_t requested_from_size)
{
  az_span payload;

  az_mqtt5_rpc_client_invoke_req_event_data command_data
      = { .correlation_id = az_span_create((uint8_t*)correlation_id, correlation_id_size),
          .content_type = vehicle03_unlock_payload_content_type,
          .request_payload = payload,
          .command_name = vehicle03_unlock_command_name,
          .rpc_server_client_id = server_client_id };

  az_result rc = az_mqtt5_rpc_client_invoke_begin(rpc_client, &command_data);

  if (az_result_failed(rc))
  {
    printf(
        // LOG_APP_ERROR "Failed to invoke command '%s' with rc: %s\n",
        "Failed to invoke command '%s' with rc: 0x%x\n",
        az_span_ptr(vehicle03_unlock_command_name),
        rc);
        // az_result_to_string(rc));
    return rc;
  }

  return AZ_OK;
}

#endif // VEHICLE03_UNLOCK_H