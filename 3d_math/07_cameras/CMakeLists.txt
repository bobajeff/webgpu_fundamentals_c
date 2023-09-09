cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-cameras # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/07_cameras/res/")

#--------------WEBGPU-CAMERAS-STEP-1-DIRECT-MATH
add_executable(webgpu-cameras-step-1-direct-math
webgpu-cameras-step-1-direct-math.c
)
target_link_libraries(webgpu-cameras-step-1-direct-math PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-CAMERAS-STEP-2-CAMERA-AIM
add_executable(webgpu-cameras-step-2-camera-aim
webgpu-cameras-step-2-camera-aim.c
)
target_link_libraries(webgpu-cameras-step-2-camera-aim PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-CAMERAS-STEP-3-LOOK-AT
add_executable(webgpu-cameras-step-3-look-at
webgpu-cameras-step-3-look-at.c
)
target_link_libraries(webgpu-cameras-step-3-look-at PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-CAMERAS-STEP-4-AIM-FS
add_executable(webgpu-cameras-step-4-aim-Fs
webgpu-cameras-step-4-aim-Fs.c
)
target_link_libraries(webgpu-cameras-step-4-aim-Fs PRIVATE glfw wgpu_native create_surface framework)