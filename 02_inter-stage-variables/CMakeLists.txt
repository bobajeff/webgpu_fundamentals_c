cmake_minimum_required(VERSION 3.0...3.25)
project(
	WebGPU-Inter-stage-Variables # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
#--------------WEBGPU-INTER-STAGE-VARIABLES-TRIANGLE
add_executable(webgpu-inter-stage-variables-triangle
webgpu-inter-stage-variables-triangle.c
)
target_link_libraries(webgpu-inter-stage-variables-triangle PRIVATE glfw wgpu_native create_surface framework)