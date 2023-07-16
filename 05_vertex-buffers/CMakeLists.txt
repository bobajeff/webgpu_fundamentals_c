cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-vertex-buffers # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

#--------------WEBGPU-VERTEX-BUFFERS
add_executable(webgpu-vertex-buffers
webgpu-vertex-buffers.c
)
target_link_libraries(webgpu-vertex-buffers PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-VERTEX-BUFFERS-2-ATTRIBUTES
add_executable(webgpu-vertex-buffers-2-attributes
webgpu-vertex-buffers-2-attributes.c
)
target_link_libraries(webgpu-vertex-buffers-2-attributes PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-VERTEX-BUFFERS-2-ATTRIBUTES-8BIT-COLORS
add_executable(webgpu-vertex-buffers-2-attributes-8bit-colors
webgpu-vertex-buffers-2-attributes-8bit-colors.c
)
target_link_libraries(webgpu-vertex-buffers-2-attributes-8bit-colors PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-VERTEX-BUFFERS-2-BUFFERS
add_executable(webgpu-vertex-buffers-2-buffers
webgpu-vertex-buffers-2-buffers.c
)
target_link_libraries(webgpu-vertex-buffers-2-buffers PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-VERTEX-BUFFERS-INDEX-BUFFER
add_executable(webgpu-vertex-buffers-index-buffer
webgpu-vertex-buffers-index-buffer.c
)
target_link_libraries(webgpu-vertex-buffers-index-buffer PRIVATE glfw wgpu_native create_surface framework)