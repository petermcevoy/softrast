default:build run

SOURCES = src/main.cpp src/tgaimage.cpp
INCLUDES= -I/usr/local/include 
LIBS    = -L/usr/local/lib -lSDL2

.PHONY: build

build:
	clang++ -g $(SOURCES) $(INCLUDES) $(LIBS) -o build/main
run:
	./build/main
clean:
	rm build/*
