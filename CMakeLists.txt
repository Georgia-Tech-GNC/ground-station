cmake_minimum_required(VERSION 3.22)
project(ground-station)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

########################################### Run Templating ###########################################
find_package(Python REQUIRED COMPONENTS Interpreter)
add_custom_target(configure_ground_station
    COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/templating/parser.py"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Configuring ground station source..."
)

################################## Build Ground Station Core Module ##################################
add_library(ground_station_core STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gs_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/core/private/data_helpers.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/private/packet_helpers.c 
)
target_include_directories(ground_station_core 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/private
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
)
add_dependencies(ground_station_core configure_ground_station)