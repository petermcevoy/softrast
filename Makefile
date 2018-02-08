default:build run

SOURCES = src/main.cpp src/tgaimage.cpp src/gl.cpp
INCLUDES= -I/usr/local/include 
LIBS    = -L/usr/local/lib -lSDL2

.PHONY: build

build:
	clang++ -g $(SOURCES) $(INCLUDES) $(LIBS) -o build/main -Wl,-v
run:
	./build/main
clean:
	rm build/*
