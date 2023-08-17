cmake_minimum_required(VERSION 3.0...3.25)
project(
	Triangle # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/triangle_/res/")
#--------------TRIANGLE
add_executable(triangle
main.c
)
target_link_libraries(triangle PRIVATE glfw wgpu_native create_surface framework)