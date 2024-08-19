cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project ("SPSCLockFreeQueue")

add_executable (SPSCLockFreeQueue "main.cpp" "SPSCLockFreeQueue.h")
