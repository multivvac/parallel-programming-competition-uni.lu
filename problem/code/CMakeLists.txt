cmake_minimum_required(VERSION 3.25)

# Project config
project(ppc LANGUAGES CXX CUDA)

set(GPU_ARCHS "native" CACHE STRING "CUDA GPU architectures")


# Build type default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- Languages / standards ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Enable separable compilation for CUDA (lets us have multiple .cu/.cpp units)
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)

# ---- Sources ----
file(GLOB SRC_FILES
  CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.cu"
)

add_executable(ppc ${SRC_FILES})

# Include path for your headers
target_include_directories(ppc PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# CUDA arch selection
set_target_properties(ppc PROPERTIES
  CUDA_ARCHITECTURES "${GPU_ARCHS}"
)

# Faster math. Comment out if you want strict IEEE.
# target_compile_options(ppc PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math>)

# On some toolchains, std::numeric_limits in device code needs this define:
# target_compile_definitions(ppc PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:__CUDACC_RTC__=0>)

# If you use threads or extra libs later, link them here:
# target_link_libraries(ppc PRIVATE Threads::Threads)

target_compile_options(ppc PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
)

target_compile_options(ppc PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-Wall,-Wextra,-Wno-pedantic,-Wno-error>
)

find_package(CUDAToolkit REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)
target_link_libraries(ppc PRIVATE CUDA::cudart Threads::Threads OpenMP::OpenMP_CXX)

set_target_properties(ppc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Convenience message
message(STATUS "GPU_ARCHS = ${GPU_ARCHS}")
message(STATUS "Build type = ${CMAKE_BUILD_TYPE}")

