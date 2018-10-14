all: main

main: main.cpp
	g++ -g --std=c++17 main.cpp -o main -lglfw -lGLEW -lGL

clean: 
	rm main

run: all
	./main

# mispelled this way too much...
urn: run

gdb: all
	gdb main
