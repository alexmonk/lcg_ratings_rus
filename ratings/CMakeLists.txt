cmake_minimum_required(VERSION 3.4)

set(source
	elo.h
	elo.cpp
	tournament.h
	tournament.cpp

	../include/ratings.h
	ratings.cpp
)
my_add_library(ratings ${source})