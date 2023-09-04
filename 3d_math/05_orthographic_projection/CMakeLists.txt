cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-orthographic-projection # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/05_orthographic_projection/res/")

#--------------WEBGPU-ORTHOGRAPHIC-PROJECTION-STEP-1-FLAT-F
add_executable(webgpu-orthographic-projection-step-1-flat-f
webgpu-orthographic-projection-step-1-flat-f.c
)
target_link_libraries(webgpu-orthographic-projection-step-1-flat-f PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-ORTHOGRAPHIC-PROJECTION-STEP-2-3D-F
add_executable(webgpu-orthographic-projection-step-2-3d-f
webgpu-orthographic-projection-step-2-3d-f.c
)
target_link_libraries(webgpu-orthographic-projection-step-2-3d-f PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-ORTHOGRAPHIC-PROJECTION-STEP-3-COLORED-3D-f
add_executable(webgpu-orthographic-projection-step-3-colored-3d-f
webgpu-orthographic-projection-step-3-colored-3d-f.c
)
target_link_libraries(webgpu-orthographic-projection-step-3-colored-3d-f PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-ORTHOGRAPHIC-PROJECTION-STEP-4-CULLMODE-BACK
add_executable(webgpu-orthographic-projection-step-4-cullmode-back
webgpu-orthographic-projection-step-4-cullmode-back.c
)
target_link_libraries(webgpu-orthographic-projection-step-4-cullmode-back PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-ORTHOGRAPHIC-PROJECTION-STEP-5-ORDER-FIXED
add_executable(webgpu-orthographic-projection-step-5-order-fixed
webgpu-orthographic-projection-step-5-order-fixed.c
)
target_link_libraries(webgpu-orthographic-projection-step-5-order-fixed PRIVATE glfw wgpu_native create_surface framework)