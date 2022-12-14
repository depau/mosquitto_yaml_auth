cmake_minimum_required(VERSION 3.23)
project(mosquitto_yaml_auth)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

find_package(PkgConfig)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

execute_process(
    COMMAND llvm-config --libs --link-static core support objectyaml
    OUTPUT_VARIABLE LLVM_STATIC_LIBS
)
string(STRIP ${LLVM_STATIC_LIBS} LLVM_STATIC_LIBS)
execute_process(
    COMMAND llvm-config --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
)
string(STRIP ${LLVM_SYSTEM_LIBS} LLVM_SYSTEM_LIBS)

pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)
pkg_check_modules(NCURSES REQUIRED ncurses)

add_library(mosquitto_yaml_auth SHARED src/bindings.cpp src/UserPassEntry.cpp)

target_link_libraries(mosquitto_yaml_auth ${MOSQUITTO_LIBRARIES} ${LLVM_STATIC_LIBS} ${LLVM_SYSTEM_LIBS} ${NCURSES_LIBRARIES})
target_include_directories(mosquitto_yaml_auth PUBLIC ${MOSQUITTO_INCLUDE_DIRS})

