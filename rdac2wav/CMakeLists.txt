cmake_minimum_required(VERSION 3.0)

project(RDAC C)

add_executable(rdac2wav src/wav.c src/decode.c src/main.c)

if(NOT WINDOWS)
  add_definitions(-Dstricmp=strcasecmp)
endif()

