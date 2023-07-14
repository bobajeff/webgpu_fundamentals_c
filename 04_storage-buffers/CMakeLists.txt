cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-storage-buffers # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
#--------------WEBGPU-SIMPLE-TRIANGLE-STORAGE-SPLIT-MINIMAL-CHANGES
add_executable(webgpu-simple-triangle-storage-split-minimal-changes
webgpu-simple-triangle-storage-split-minimal-changes.c
)
target_link_libraries(webgpu-simple-triangle-storage-split-minimal-changes PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TRIANGLE-STORAGE-BUFFER-SPLIT
add_executable(webgpu-simple-triangle-storage-buffer-split
webgpu-simple-triangle-storage-buffer-split.c
)
target_link_libraries(webgpu-simple-triangle-storage-buffer-split PRIVATE glfw wgpu_native create_surface framework)