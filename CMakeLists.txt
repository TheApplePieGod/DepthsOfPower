cmake_minimum_required(VERSION 3.19)

project(
    DepthsOfPower
    VERSION 0.1
    DESCRIPTION "A 2D game based on the Diamond engine"
)

include_directories("include")
include_directories("lib")
include_directories("lib/FastNoise2/include")
include_directories("lib/openal-soft/include")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory("lib/openal-soft") # build the oal wrapper

add_compile_definitions(DEPTHS_DEBUG)

file(GLOB SOURCES "src/*.cpp")
file(GLOB UTILS "src/util/*.cpp")
file(GLOB GUI "src/gui/*.cpp")
# add_executable(Diamond src/testing/main.cpp)
add_executable(DepthsOfPower ${SOURCES} ${UTILS} ${GUI} "src/entry/main.cpp")

#build shaders as well
add_custom_command(
    TARGET DepthsOfPower POST_BUILD
    COMMAND "CompileShaders.bat"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Diamond
add_compile_definitions(DIAMOND_IMGUI)
set(VulkanBasePath "C:/VulkanSDK/1.2.162.1")
find_package(Diamond REQUIRED)
include_directories(${Diamond_DIR}/../../include)
include_directories(${Diamond_DIR}/../../include/lib)
include_directories(${Diamond_DIR}/../../include/lib/glfw/include)
include_directories("${VulkanBasePath}/Include")
target_link_libraries(DepthsOfPower Diamond::Diamond)

# debug
find_library(FastNoiseD FastNoiseD "lib/FastNoise2")
target_link_libraries(DepthsOfPower "${FastNoiseD}")
target_link_libraries(DepthsOfPower OpenAL)

#release
# find_library(FastNoise FastNoise "lib/FastNoise2")
# target_link_libraries(DepthsOfPower "${FastNoise}")

# Tell C++ compiler to use C++20 features.
target_compile_features(DepthsOfPower PUBLIC cxx_std_20)