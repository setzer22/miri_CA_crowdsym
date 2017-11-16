CXX?=g++
CXXFLAGS?=$(shell pkg-config sdl2 --libs --cflags) $(shell pkg-config gl --libs --cflags) $(shell pkg-config glew --libs --cflags) -Werror -std=c++11 -Wno-write-strings -Wall 

.PHONY: all msg clean fullclean

all: msg main

msg:
	@echo --- START COMPILATION ---

main: main.cpp $(shell ls *.h) Particle.o Geometry.o
	${CXX} -O0 -g main.cpp Particle.o Geometry.o ${CXXFLAGS} -o main

Geometry.o: Geometry.cpp Geometry.h
	g++ -c Geometry.cpp

Particle.o: Particle.cpp Particle.h
	g++ -c Particle.cpp 

small: main.cpp
	${CXX} -O0 -g main.cpp ${CXXFLAGS} -o main
	strip main
	sstrip main

debug: main.cpp
	${CXX} -O0 -g main.cpp ${CXXFLAGS} -o main

run: msg main
	time ./main

clean:
	rm -f main

fullclean: clean
