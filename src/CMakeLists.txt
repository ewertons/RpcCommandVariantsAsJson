# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

project(RpcClient LANGUAGES C)

set(CMAKE_C_STANDARD 99)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

# TODO: remove this hack
# add_compile_definitions(TRANSPORT_MOSQUITTO)

set(RpcClient_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/azure_rpc_client.c
)

add_executable(RpcClient ${RpcClient_SOURCE_FILES})

target_include_directories(RpcClient
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  az_core)

target_link_libraries(RpcClient
  PRIVATE
    az_core
)
