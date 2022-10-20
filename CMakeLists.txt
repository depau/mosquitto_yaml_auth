cmake_minimum_required(VERSION 3.23)
project(mosquitto_yaml_auth)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support objectyaml)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

add_library(mosquitto_yaml_auth SHARED src/bindings.cpp src/UserPassEntry.cpp)

target_link_libraries(mosquitto_yaml_auth ${MOSQUITTO_LIBRARIES})
target_include_directories(mosquitto_yaml_auth PUBLIC ${MOSQUITTO_INCLUDE_DIRS} ${llvm_libs})

