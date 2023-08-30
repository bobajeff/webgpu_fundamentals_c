cmake_minimum_required(VERSION 3.0...3.25)
project(
	webgpu-matrix-math # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/3d_math/04_matrix_math/res/")

#--------------WEBGPU-MATRIX-MATH-TRANSFORM-TRS-3X3
add_executable(webgpu-matrix-math-transform-trs-3x3
webgpu-matrix-math-transform-trs-3x3.c
)
target_link_libraries(webgpu-matrix-math-transform-trs-3x3 PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-MATRIX-MATH-TRANSFORM-SRT-3X3
add_executable(webgpu-matrix-math-transform-srt-3x3
webgpu-matrix-math-transform-srt-3x3.c
)
target_link_libraries(webgpu-matrix-math-transform-srt-3x3 PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-MATRIX-MATH-TRANSFORM-FIVE-FS-3X3
add_executable(webgpu-matrix-math-transform-five-fs-3x3
webgpu-matrix-math-transform-five-fs-3x3.c
)
target_link_libraries(webgpu-matrix-math-transform-five-fs-3x3 PRIVATE glfw wgpu_native create_surface framework)

#--------------WEBGPU-MATRIX-MATH-TRANSFORM-MOVE-ORIGIN-3X3
add_executable(webgpu-matrix-math-transform-move-origin-3x3
webgpu-matrix-math-transform-move-origin-3x3.c
)
target_link_libraries(webgpu-matrix-math-transform-move-origin-3x3 PRIVATE glfw wgpu_native create_surface framework)