project(
	webgpu-compute-shaders # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/compute_shaders/01_compute_shaders_basics/res/")

#--------------WEBGPU-COMPUTE-SHADERS-BUILTINS
add_executable(webgpu-compute-shaders-builtins
webgpu-compute-shaders-builtins.c
)
target_link_libraries(webgpu-compute-shaders-builtins PRIVATE glfw wgpu_native create_surface framework)
