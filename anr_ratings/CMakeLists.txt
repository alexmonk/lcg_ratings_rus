cmake_minimum_required(VERSION 3.4)

set(source
	main.cpp
)
my_add_executable(anr_ratings ${source})
target_link_libraries(anr_ratings LINK_PUBLIC ratings)