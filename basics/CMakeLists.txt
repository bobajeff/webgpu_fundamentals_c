project(
	Basics # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

add_subdirectory(01_fundamentals)
add_subdirectory(02_inter-stage-variables)
add_subdirectory(03_uniforms)
add_subdirectory(04_storage-buffers)
add_subdirectory(05_vertex-buffers)
add_subdirectory(06_textures)
add_subdirectory(07_importing_images)