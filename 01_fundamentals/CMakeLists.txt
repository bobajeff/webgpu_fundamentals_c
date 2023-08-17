cmake_minimum_required(VERSION 3.0...3.25)
project(
	Fundamentals # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/res/01_fundamentals/")


#--------------WEBGPU-SIMPLE-TRIANGLE
add_executable(webgpu-simple-triangle
webgpu-simple-triangle.c
)
target_link_libraries(webgpu-simple-triangle PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-COMPUTE
add_executable(webgpu-simple-compute
webgpu-simple-compute.c
)
target_link_libraries(webgpu-simple-compute PRIVATE glfw wgpu_native create_surface framework)
