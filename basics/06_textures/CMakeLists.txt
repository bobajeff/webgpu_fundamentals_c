cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-vertex-buffers # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/basics/06_textures/res/")

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD
add_executable(webgpu-simple-textured-quad
webgpu-simple-textured-quad.c
)
target_link_libraries(webgpu-simple-textured-quad PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-LINEAR
add_executable(webgpu-simple-textured-quad-linear
webgpu-simple-textured-quad-linear.c
)
target_link_libraries(webgpu-simple-textured-quad-linear PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-MINFILTER
add_executable(webgpu-simple-textured-quad-minfilter
webgpu-simple-textured-quad-minfilter.c
)
target_link_libraries(webgpu-simple-textured-quad-minfilter PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-MIPMAP
add_executable(webgpu-simple-textured-quad-mipmap
webgpu-simple-textured-quad-mipmap.c
)
target_link_libraries(webgpu-simple-textured-quad-mipmap PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-MIPMAPFILTER
add_executable(webgpu-simple-textured-quad-mipmapfilter
webgpu-simple-textured-quad-mipmapfilter.c
)
target_link_libraries(webgpu-simple-textured-quad-mipmapfilter PRIVATE glfw wgpu_native create_surface framework cglm)