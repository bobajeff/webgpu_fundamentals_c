cmake_minimum_required(VERSION 3.0...3.25)
project(
	WebGPU-Inter-stage-Variables # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
#--------------WEBGPU-INTER-STAGE-VARIABLES-TRIANGLE
add_executable(webgpu-inter-stage-variables-triangle
webgpu-inter-stage-variables-triangle.c
)
target_link_libraries(webgpu-inter-stage-variables-triangle PRIVATE glfw wgpu_native create_surface framework)

#--------------INTER_STAGE_VARIABLES_CONNECT_BY_LOCATION
add_executable(inter_stage_variables_connect_by_location
inter_stage_variables_connect_by_location.c
)
target_link_libraries(inter_stage_variables_connect_by_location PRIVATE glfw wgpu_native create_surface framework)

#--------------INTER_STAGE_VARIABLES_CONNECT_BY_LOCATION
add_executable(inter_stage_variables_connect_by_location_part2
inter_stage_variables_connect_by_location_part2.c
)
target_link_libraries(inter_stage_variables_connect_by_location_part2 PRIVATE glfw wgpu_native create_surface framework)