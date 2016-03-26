cmake_minimum_required(VERSION 3.4)

set(source
	main.cpp
)
my_add_executable(agot_ratings ${source})
target_link_libraries(agot_ratings LINK_PUBLIC ratings)