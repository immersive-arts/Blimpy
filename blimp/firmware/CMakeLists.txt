# minimal required cmake version
cmake_minimum_required(VERSION 3.7)

# you can set your own project name
project(blimpy)

# this should not be changed
set(CMAKE_C_STANDARD 99)

# add your source files
set(SOURCE_FILES
        src/bat.c
        src/bat.h
        src/exp.c
        src/exp.h
        src/main.c
        src/mod.c
        src/mod.h
        src/mot.c
        src/mot.h
        src/neo.c
        src/neo.h
        src/pwr.c
        src/pwr.h
        src/srv.c
        src/srv.h)

# create a fake library target
add_library(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

# include naos include paths
add_subdirectory(naos)
