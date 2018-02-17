default:build

CC		= clang
CFLAGS	= -g -O3
SOURCES = examples/objpreview.c src/gl.c src/obj.c
INCLUDES= -I/usr/local/include -Isrc/ -Iexamples/
LIBS    = -L/usr/local/lib -lSDL2

.PHONY: build

objpreview: examples/objpreview.c
	$(CC) $(CFLAGS) examples/objpreview.c src/gl.c src/obj.c $(INCLUDES) $(LIBS) -o build/objpreview -Wl,-v
build: objpreview
clean:
	rm build/*
