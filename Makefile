CXX?=g++
CXXFLAGS?=$(shell pkg-config sdl2 --libs --cflags) $(shell pkg-config gl --libs --cflags) $(shell pkg-config glew --libs --cflags) $(shell pkg-config cal3d --libs --cflags) -std=c++11 -Wno-write-strings -Wall 

.PHONY: all msg clean fullclean

all: msg main

msg:
	@echo --- START COMPILATION ---

main: main.cpp $(shell ls *.h) $(shell ls *.hpp) Particle.o Geometry.o tga.o model.o
	${CXX} ${CXXFLAGS} -O0 -g main.cpp Particle.o Geometry.o tga.o model.o -o main

Geometry.o: Geometry.cpp Geometry.h
	g++ -c Geometry.cpp

Particle.o: Particle.cpp Particle.h
	g++ -c Particle.cpp 

model.o: model.cpp model.h
	g++ -c model.cpp

tga.o: tga.cpp tga.h
	g++ -c tga.cpp

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
