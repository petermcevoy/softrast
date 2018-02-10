default:build run

#SOURCES = src/main.cpp src/tgaimage.cpp src/gl.cpp src/obj.c
#SOURCES = linalg_test.c
SOURCES = src/main.c src/gl.c src/obj.c
INCLUDES= -I/usr/local/include 
LIBS    = -L/usr/local/lib -lSDL2

.PHONY: build

build:
	clang -g $(SOURCES) $(INCLUDES) $(LIBS) -O3 -o build/main -Wl,-v
run:
	./build/main
clean:
	rm build/*
