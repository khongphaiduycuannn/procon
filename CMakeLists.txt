# set(CMAKE_AR "C:/msys64/mingw64/bin/ar.exe" CACHE FILEPATH "Archiver")
# set(CMAKE_MAKE_PROGRAM "C:/msys64/mingw64/bin/mingw32-make.exe" CACHE FILEPATH "Make program")
# set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
# set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
cmake_minimum_required(VERSION 3.22)
project(solver CXX)
enable_testing()

option(CPU_ARM "solver will be compiled ARM arch." OFF)

if(CPU_ARM)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -O3")
    message("ARM")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -mbmi -mabm -std=c++23 -O3")
    message("x86")
endif()

add_executable(solver 
    gutiku2-8-nobeam-product/main.cpp 
    gutiku2-8-nobeam-product/solver.cpp
    include/source/kyougi_app.cpp
)

add_executable(solver_faster
    gutiku2-11-nobeam-fast-product/main.cpp 
    gutiku2-11-nobeam-fast-product/solver.cpp
    include/source/kyougi_app.cpp
)
