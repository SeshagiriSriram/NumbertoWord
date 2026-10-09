cmake_minimum_required(VERSION 3.10)

# Define the project name and programming language metadata
project(NumbertoWord C)

# Set the C standard target layer profile explicitly to C99 or newer
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- CRITICAL FOR CLANG-TIDY INTEGRATION ---
# Forces CMake to automatically generate 'compile_commands.json' 
# This file maps exactly how main.c is compiled so clang-tidy can analyze it.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the target binary executable name and its mapped source files
add_executable(my_c_app main.c)
