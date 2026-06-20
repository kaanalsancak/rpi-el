cmake_minimum_required(VERSION 3.16)
project(rpi_el C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(rpi_info
	       src/app/main.c
)
	
