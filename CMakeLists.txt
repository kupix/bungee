cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

add_custom_command(
    OUTPUT version.cpp _
    COMMAND ${CMAKE_COMMAND} -DVERSION_NAMESPACE=Bungee -DGIT_CWD=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/version.cmake)

project(bungee)

add_library(libbungee STATIC
  src/Synthesis.cpp
  src/Fourier.cpp
  src/Fourier.cpp
  src/Grain.cpp
  src/Grains.cpp
  src/Input.cpp
  src/Output.cpp
  src/Partials.cpp
  src/Stretch.cpp
  src/Stretcher.cpp
  src/Window.cpp
  src/Assert.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

target_include_directories(libbungee PUBLIC .)
set_target_properties(libbungee PROPERTIES OUTPUT_NAME bungee)

target_include_directories(libbungee PRIVATE submodules submodules/eigen)

set(BUNGEE_SELF_TEST 0 CACHE STRING "Enable self tests (0=off, 1=fast, 2=full)")
target_compile_definitions(libbungee PRIVATE BUNGEE_SELF_TEST=${BUNGEE_SELF_TEST})
target_compile_definitions(libbungee PRIVATE eigen_assert=BUNGEE_ASSERT1)

set(KISSFFT_PKGCONFIG OFF CACHE INTERNAL "" FORCE)
set(KISSFFT_STATIC ON CACHE INTERNAL "" FORCE)
set(KISSFFT_TEST OFF CACHE INTERNAL "" FORCE)
set(KISSFFT_TOOLS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(submodules/kissfft EXCLUDE_FROM_ALL)
target_link_libraries(libbungee PRIVATE kissfft)

target_compile_options(libbungee PRIVATE
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fwrapv>
)

add_executable(bungee cmd/main.cpp)

target_include_directories(bungee PRIVATE submodules/cxxopts/include)

target_link_libraries(bungee PRIVATE libbungee)

target_include_directories(bungee PRIVATE submodules/cxxopts/include)
