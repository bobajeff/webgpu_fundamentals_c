project(
	3DMath # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

add_subdirectory(01_translation)
add_subdirectory(02_rotation)
add_subdirectory(03_scale)
add_subdirectory(04_matrix_math)
add_subdirectory(05_orthographic_projection)
