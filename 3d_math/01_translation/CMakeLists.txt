cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-translation # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/01_translation/res/")

#--------------WEBGPU-TRANSLATION-PREP
add_executable(webgpu-translation-prep
webgpu-translation-prep.c
)
target_link_libraries(webgpu-translation-prep PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-TRANSLATION
add_executable(webgpu-translation
webgpu-translation.c
)
target_link_libraries(webgpu-translation PRIVATE glfw wgpu_native create_surface framework)