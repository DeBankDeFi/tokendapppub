cmake_minimum_required( VERSION 3.5 )

include(ExternalProject)
find_package(Git REQUIRED)
include(GNUInstallDirs)

ExternalProject_Add(
    eosio.contracts
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${TEST_BUILD_TYPE} -DEOSIO_ROOT=${EOSIO_ROOT} -DEOSIO_DEPENDENCY=${EOSIO_DEPENDENCY}

    SOURCE_DIR ${CMAKE_SOURCE_DIR}/eosio.contracts
    BINARY_DIR ${CMAKE_BINARY_DIR}/eosio.contracts
    BUILD_ALWAYS 0

    EXCLUDE_FROM_ALL TRUE

    GIT_REPOSITORY git@github.com:eosio/eosio.contracts.git
    GIT_TAG        v1.2.0

    BUILD_COMMAND make eosio.sudo.wasm eosio.system.wasm eosio.token.wasm eosio.msig.wasm contracts_unit_tests
    TEST_COMMAND   ""
    INSTALL_COMMAND ln -sf ${CMAKE_BINARY_DIR}/eosio.contracts/tests/contracts.hpp ${CMAKE_BINARY_DIR}/
)

list(APPEND CMAKE_MODULE_PATH ${EOSIO_ROOT}/lib/cmake)
include(EosioTester)

### check the version of EOSIO
string(FIND "${EOSIO_VERSION}" "${EOSIO_DEPENDENCY}" output)
if (NOT "${output}" EQUAL 0)
    message(FATAL_ERROR "Incorrect EOSIO version, please use version ${EOSIO_DEPENDENCY}.x")
endif()

enable_testing()

configure_file(${CMAKE_SOURCE_DIR}/tokendapppub_test.hpp.in ${CMAKE_BINARY_DIR}/tokendapppub_test.hpp)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB UNIT_TESTS "*.cpp" "*.hpp" )

add_eosio_test(unit_test ${UNIT_TESTS})

add_dependencies(unit_test eosio.contracts)