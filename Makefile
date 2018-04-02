default:build

CC		= clang
CFLAGS	= -g -O3
SOURCES = examples/objpreview.c src/gl.c src/obj.c
INCLUDES= -I/usr/local/include -Isrc/ -Iexamples/
LIBS    = -L/usr/local/lib /usr/local/lib/libSDL2.a -lm -liconv -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit

.PHONY: build

objpreview: examples/objpreview.c
	$(CC) $(CFLAGS) examples/objpreview.c src/gl.c src/obj.c $(INCLUDES) $(LIBS) -o build/objpreview -Wl,-v
duck: examples/duck.c
	xxd -i res/duckpoly.obj res/duckpoly_obj.c
	xxd -i res/duck.wav res/duck_wav.c
	xxd -i res/duckdiffuse.bmp res/duckdiffuse_bmp.c
	$(CC) $(CFLAGS) examples/duck.c src/gl.c src/obj.c -Ires/ $(INCLUDES) $(LIBS) -o build/duck -Wl,-v
build: objpreview
clean:
	rm build/*
