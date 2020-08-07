# Defines StateDB target.
project( StateDB )
cmake_minimum_required( VERSION 3.5 )
include( GNUInstallDirs )

#SET( Boost_USE_STATIC_LIBS OFF)

set (CMAKE_CXX_STANDARD 17)

set(BOOST_ALL_DYN_LINK OFF)

# add defaults for openssl
if(APPLE AND UNIX AND "${OPENSSL_ROOT_DIR}" STREQUAL "")
  set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()

find_package(OpenSSL REQUIRED) 
include_directories(${OPENSSL_ROOT_DIR}/include)

FIND_PACKAGE(Boost 1.57 REQUIRED COMPONENTS filesystem unit_test_framework)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-conversion" )

add_executable(statedb statedb.cpp eos_db.cpp name.cpp )

add_subdirectory( chainbase )
add_subdirectory( fc )
add_subdirectory( unit_test )

target_link_libraries( statedb fc Boost::filesystem chainbase)
