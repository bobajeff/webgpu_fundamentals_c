cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-rotation # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/02_rotation/res/")

#--------------WEBGPU-ROTATION-VIA-UNIT-CIRCLE
add_executable(webgpu-rotation-via-unit-circle
webgpu-rotation-via-unit-circle.c
)
target_link_libraries(webgpu-rotation-via-unit-circle PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-ROTATION
add_executable(webgpu-rotation
webgpu-rotation.c
)
target_link_libraries(webgpu-rotation PRIVATE glfw wgpu_native create_surface framework)