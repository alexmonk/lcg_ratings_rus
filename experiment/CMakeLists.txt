cmake_minimum_required(VERSION 3.4)

set(source
	main.cpp
)
my_add_executable(experiment ${source})
target_link_libraries(experiment)