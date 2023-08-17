cmake_minimum_required(VERSION 3.0...3.25)
project(
	WebGPU-Inter-stage-Variables # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/basics/02_inter-stage-variables/res/")

#--------------WEBGPU-INTER-STAGE-VARIABLES-TRIANGLE
add_executable(webgpu-inter-stage-variables-triangle
webgpu-inter-stage-variables-triangle.c
)
target_link_libraries(webgpu-inter-stage-variables-triangle PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-INTER-STAGE-VARIABLES-TRIANGLE-BY-FN-PARAM
add_executable(webgpu-inter-stage-variables-triangle-by-fn-param
webgpu-inter-stage-variables-triangle-by-fn-param.c
)
target_link_libraries(webgpu-inter-stage-variables-triangle-by-fn-param PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-FRAGMENT-SHADER-BUILTIN-POSITION
add_executable(webgpu-fragment-shader-builtin-position
webgpu-fragment-shader-builtin-position.c
)
target_link_libraries(webgpu-fragment-shader-builtin-position PRIVATE glfw wgpu_native create_surface framework)