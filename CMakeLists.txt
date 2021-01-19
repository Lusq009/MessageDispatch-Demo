cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
project(Demo)

message("Current folder is: ${CMAKE_CURRENT_LIST_DIR}")
set(BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/Bin")

#link_directories("${BIN_DIR}")
############################################################
# Create an executable
############################################################

# Add an executable with the above sources
add_executable(Demo
    src/main.cpp 
    src/se_dataprocessor.hpp
)

set_target_properties(Demo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
