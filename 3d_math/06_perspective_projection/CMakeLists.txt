cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-perspective-projection # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/06_perspective_projection/res/")

#--------------WEBGPU-PERSPECTIVE-PROJECTION-STEP-1-FUDGE-FACTOR
add_executable(webgpu-perspective-projection-step-1-fudge-factor
webgpu-perspective-projection-step-1-fudge-factor.c
)
target_link_libraries(webgpu-perspective-projection-step-1-fudge-factor PRIVATE glfw wgpu_native create_surface framework)