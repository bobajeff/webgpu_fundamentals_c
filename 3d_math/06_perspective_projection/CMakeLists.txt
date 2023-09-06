cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-perspective-projection # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/06_perspective_projection/res/")

#--------------WEBGPU-PERSPECTIVE-PROJECTION-STEP-1-FUDGE-FACTOR
add_executable(webgpu-perspective-projection-step-1-fudge-factor
webgpu-perspective-projection-step-1-fudge-factor.c
)
target_link_libraries(webgpu-perspective-projection-step-1-fudge-factor PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-PERSPECTIVE-PROJECTION-STEP-2-GPU-DIVIDE-BY-W
add_executable(webgpu-perspective-projection-step-2-gpu-divide-by-w
webgpu-perspective-projection-step-2-gpu-divide-by-w.c
)
target_link_libraries(webgpu-perspective-projection-step-2-gpu-divide-by-w PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-PERSPECTIVE-PROJECTION-STEP-3-PERSPECTIVE-Z-TO-W
add_executable(webgpu-perspective-projection-step-3-perspective-z-to-w
webgpu-perspective-projection-step-3-perspective-z-to-w.c
)
target_link_libraries(webgpu-perspective-projection-step-3-perspective-z-to-w PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-PERSPECTIVE-PROJECTION-STEP-4-PERSPECTIVE
add_executable(webgpu-perspective-projection-step-4-perspective
webgpu-perspective-projection-step-4-perspective.c
)
target_link_libraries(webgpu-perspective-projection-step-4-perspective PRIVATE glfw wgpu_native create_surface framework)