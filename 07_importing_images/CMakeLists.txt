cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-importing-textures # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-IMPORT-NO-MIPS
add_executable(webgpu-simple-textured-quad-import-no-mips
webgpu-simple-textured-quad-import-no-mips.c
)
target_link_libraries(webgpu-simple-textured-quad-import-no-mips PRIVATE glfw wgpu_native create_surface framework)
