cmake_minimum_required(VERSION 3.23)
project(MetodyOpt)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(program)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cpp)

target_sources(program
  PRIVATE
    main.cpp
    ${SRC_FILES}
)

target_include_directories(program
  PRIVATE
    include/
)

target_compile_options(program PRIVATE -O3 -march=native -funroll-loops)
target_link_libraries(program PRIVATE pthread)
