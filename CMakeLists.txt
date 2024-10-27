cmake_minimum_required(VERSION 3.10)

# Set the project name
project(MyProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
add_executable(MyProject main.cpp server.cpp  "binary_search.cpp")

# Link the Winsock library (ws2_32)
if(WIN32)
    target_link_libraries(MyProject PRIVATE ws2_32)
endif()

# Set working directory for Visual Studio
set_property(TARGET MyProject PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
