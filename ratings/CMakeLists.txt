cmake_minimum_required(VERSION 3.4)

set(source
        basic.h
	
	elo.h
	elo.cpp

	history.h
	history.cpp

	tournament.h
	tournament.cpp
	tome_format.cpp

	system.h
	engine.h
	engine.cpp

	../include/ratings.h
	ratings.cpp
)
my_add_library(ratings ${source})