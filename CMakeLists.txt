cmake_minimum_required(VERSION 3.19)

project(
    DepthsOfPower
    VERSION 0.1
    DESCRIPTION "A 2D game based on the Diamond engine"
)

include_directories("include")
include_directories("lib/Diamond/include")
include_directories("lib/glfw-3.3.2.bin.WIN64/include")
include_directories("lib/glm")
include_directories("C:/VulkanSDK/1.2.162.1/Include")

add_compile_definitions(DEPTHS_DEBUG)
add_compile_definitions(DIAMOND_DEBUG)

#build shaders as well
add_custom_command(
    OUTPUT CompileShaders
    COMMAND "CompileShaders.bat"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

file(GLOB SOURCES "src/*.cpp")
# add_executable(Diamond src/testing/main.cpp)
add_executable(DepthsOfPower "${SOURCES}" "src/entry/main.cpp" CompileShaders)

find_library(Diamond Diamond "lib/Diamond")
find_library(glfw3 glfw3 "lib/glfw-3.3.2.bin.WIN64/lib-vc2019")
find_library(vulkan-1 vulkan-1 "C:/VulkanSDK/1.2.162.1/Lib")
target_link_libraries(DepthsOfPower "${glfw3}")
target_link_libraries(DepthsOfPower "${vulkan-1}")
target_link_libraries(DepthsOfPower "${Diamond}")

# # Tell C++ compiler to use C++20 features.
target_compile_features(DepthsOfPower PUBLIC cxx_std_20)