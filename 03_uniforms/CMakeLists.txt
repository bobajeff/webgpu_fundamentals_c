cmake_minimum_required(VERSION 3.0...3.25)
project(
	WebGPU-Uniforms # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

#--------------WEBGPU-SIMPLE-TRIANGLE-UNIFORMS
add_executable(webgpu-simple-triangle-uniforms
webgpu-simple-triangle-uniforms.c
)
target_link_libraries(webgpu-simple-triangle-uniforms PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TRIANGLE-UNIFORMS-MULTIPLE
add_executable(webgpu-simple-triangle-uniforms-multiple
webgpu-simple-triangle-uniforms-multiple.c
)
target_link_libraries(webgpu-simple-triangle-uniforms-multiple PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TRIANGLE-UNIFORMS-SPLIT
add_executable(webgpu-simple-triangle-uniforms-split
webgpu-simple-triangle-uniforms-split.c
)
target_link_libraries(webgpu-simple-triangle-uniforms-split PRIVATE glfw wgpu_native create_surface framework)
