cmake_minimum_required(VERSION 3.19)

project(
    DepthsOfPower
    VERSION 0.1
    DESCRIPTION "A 2D game based on the Diamond engine"
)

include_directories("include")
include_directories("lib")
include_directories("lib/FastNoise2/include")

add_compile_definitions(DEPTHS_DEBUG)

#build shaders as well
add_custom_command(
    OUTPUT CompileShaders
    COMMAND "CompileShaders.bat"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

file(GLOB SOURCES "src/*.cpp")
file(GLOB UTILS "src/util/*.cpp")
file(GLOB GUI "src/gui/*.cpp")
# add_executable(Diamond src/testing/main.cpp)
add_executable(DepthsOfPower ${SOURCES} ${UTILS} ${GUI} "src/entry/main.cpp" CompileShaders)

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

#release
# find_library(FastNoise FastNoise "lib/FastNoise2")
# target_link_libraries(DepthsOfPower "${FastNoise}")

set_target_properties(DepthsOfPower PROPERTIES LINK_FLAGS "/ignore:4103")

# Tell C++ compiler to use C++20 features.
target_compile_features(DepthsOfPower PUBLIC cxx_std_20)