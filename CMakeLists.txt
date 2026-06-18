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

target_compile_options(program PRIVATE
    -O3 -march=native -funroll-loops
    -ffast-math
    -fno-exceptions -fno-rtti
    -DNDEBUG
    -Wno-alloc-size-larger-than
)
target_link_libraries(program PRIVATE pthread)
target_link_options(program PRIVATE -Wno-alloc-size-larger-than)

# Link-time optimization (cross-file inlining)
set_property(TARGET program PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
