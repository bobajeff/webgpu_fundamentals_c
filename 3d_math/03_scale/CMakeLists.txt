cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-scale # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/03_scale/res/")

#--------------WEBGPU-SCALE
add_executable(webgpu-scale
webgpu-scale.c
)
target_link_libraries(webgpu-scale PRIVATE glfw wgpu_native create_surface framework)
