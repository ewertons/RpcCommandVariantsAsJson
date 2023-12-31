# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

if (TRANSPORT_MOSQUITTO)

cmake_minimum_required (VERSION 3.10)

project (RpcCommandVariantsAsJsonPosixMosquitto LANGUAGES C)

set(CMAKE_C_STANDARD 99)

# # Azure MQTT5 State Machine RPC Server Sample
# add_executable (mosquitto_rpc_server_sample
#   ${CMAKE_CURRENT_LIST_DIR}/mosquitto_rpc_server_sample.c
#   ${CMAKE_CURRENT_LIST_DIR}/mosquitto_rpc_server_sample_json_parser.c
# )

# target_include_directories(mosquitto_rpc_server_sample
#   PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}
#     ${CMAKE_SOURCE_DIR}/sdk/src/azure/core/
# )

# target_link_libraries(mosquitto_rpc_server_sample
#   PRIVATE
#   az_core
# )

# create_map_file(mosquitto_rpc_server_sample mosquitto_rpc_server_sample.map)

# Azure MQTT5 State Machine RPC Client Sample
add_executable (mosquitto_rpc_client_sample2
  ${CMAKE_CURRENT_LIST_DIR}/mosquitto_rpc_client_sample.c
  ${CMAKE_CURRENT_LIST_DIR}/rpc_command_list.c
)

target_include_directories(mosquitto_rpc_client_sample2
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/sdk/src/azure/core/
)

target_link_libraries(mosquitto_rpc_client_sample2
  PRIVATE
  uuid
  az_core
)

create_map_file(mosquitto_rpc_client_sample2 mosquitto_rpc_client_sample2.map)

# # Azure MQTT5 State Machine RPC Client & Server Sample
# add_executable (mosquitto_multiple_rpc_sample
#   ${CMAKE_CURRENT_LIST_DIR}/mosquitto_multiple_rpc_sample.c
# )

# target_include_directories(mosquitto_multiple_rpc_sample
#   PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}
#     ${CMAKE_SOURCE_DIR}/sdk/src/azure/core/
# )

# target_link_libraries(mosquitto_multiple_rpc_sample
#   PRIVATE
#   uuid
#   az_core
# )

# create_map_file(mosquitto_multiple_rpc_sample mosquitto_multiple_rpc_sample.map)

endif() # TRANSPORT_MOSQUITTO
