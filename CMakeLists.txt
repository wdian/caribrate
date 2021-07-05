cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 11)

set(proj_name Calibrate)
project(${proj_name})

message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(inc_dir include)
set(src_dir src)

set(lib_target_name  ${proj_name})
set(exec_target_name  main)


include_directories(${inc_dir})
foreach(build_src  ${src_dir})
    aux_source_directory(${build_src} build_list)
endforeach()

add_library(${lib_target_name} SHARED ${build_list})

add_executable(${exec_target_name} ${build_list})
install(TARGETS   ${lib_target_name} DESTINATION ./)
install(TARGETS   ${exec_target_name} DESTINATION ./)