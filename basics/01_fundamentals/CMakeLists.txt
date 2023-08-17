project(
	Fundamentals # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/basics/01_fundamentals/res/")


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
