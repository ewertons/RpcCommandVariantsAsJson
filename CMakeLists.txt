cmake_minimum_required(VERSION 3.16)

project(RpcCommandVariantsAsJson LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_compile_definitions(TRANSPORT_MOSQUITTO)

add_subdirectory(deps)
add_subdirectory(src)
