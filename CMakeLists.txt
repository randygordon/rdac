cmake_minimum_required(VERSION 3.0)

project(RDAC C)

add_executable(rdac2wav src/vs/wav.c src/vs/decode.c src/vs/main.c)

if(NOT WINDOWS)
  add_definitions(-Dstricmp=strcasecmp)
endif()

