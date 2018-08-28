cmake_minimum_required(VERSION 3.11)
project(tokendapppub)

set(EOSIO_DEPENDENCY "1.1")
set(EOSIO_WASMSDK_DEPENDENCY "1.1")

set(CMAKE_CXX_STANDARD 14)

if(EOSIO_WASMSDK_ROOT STREQUAL "" OR NOT EOSIO_WASMSDK_ROOT)
    set(EOSIO_WASMSDK_ROOT "/usr/local/eosio.wasmsdk")
endif()

list(APPEND CMAKE_MODULE_PATH ${EOSIO_WASMSDK_ROOT}/lib/cmake)
include(EosioWasmToolchain)

add_executable(tokendapppub.wasm tokendapppub.hpp tokendapppub.cpp)

if(EOSIO_ROOT STREQUAL "" OR NOT EOSIO_ROOT)
   set(EOSIO_ROOT "/usr/local/eosio")
endif()
set(EOSIOCPP ${EOSIO_ROOT}/bin/eosiocpp)
set(CONTRACT_ABI_FILE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.abi)
set(CONTRACT_HPP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.hpp)
add_custom_target(tokendapppub.abi ALL)
add_custom_command(
    TARGET tokendapppub.abi
    COMMAND ${EOSIOCPP} -g ${CONTRACT_ABI_FILE} ${CONTRACT_HPP_FILE}
)

# include(UnitTestsExternalProject.txt)