cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-importing-textures # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/basics/07_importing_images/res/")

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-IMPORT-NO-MIPS
add_executable(webgpu-simple-textured-quad-import-no-mips
webgpu-simple-textured-quad-import-no-mips.c
)
target_link_libraries(webgpu-simple-textured-quad-import-no-mips PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-IMPORT
add_executable(webgpu-simple-textured-quad-import
webgpu-simple-textured-quad-import.c
)
target_link_libraries(webgpu-simple-textured-quad-import PRIVATE glfw wgpu_native create_surface framework cglm)

#--------------WEBGPU-SIMPLE-TEXTURED-QUAD-IMPORT-VIDEO
if(HAS_FFMPEG)
  add_executable(webgpu-simple-textured-quad-import-video
  webgpu-simple-textured-quad-import-video.c
  )
  target_link_libraries(webgpu-simple-textured-quad-import-video PRIVATE glfw wgpu_native create_surface framework cglm video)
endif()