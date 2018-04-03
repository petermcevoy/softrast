default:build

CC		= clang
CFLAGS	= -g -O3
SOURCES = examples/objpreview.c src/gl.c src/obj.c
INCLUDES= -I/usr/local/include -Isrc/ -Iexamples/
LIBS    = /usr/local/lib/libSDL2.a -lm -liconv -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,CoreVideo -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit

.PHONY: build

objpreview: examples/objpreview.c
	$(CC) $(CFLAGS) examples/objpreview.c src/gl.c src/obj.c $(INCLUDES) $(LIBS) -o build/objpreview

# Get the necessary resource files duckpoly.obj, duck.wav and duckdiffuse.bmp 
# from here: https://drive.google.com/file/d/1KGDgeG7LKXui9Svlf9yfXrCqcRwHIBr0/view?usp=sharing
duck: examples/duck.c
	xxd -i res/duckpoly.obj res/duckpoly_obj.c
	xxd -i res/duck.wav res/duck_wav.c
	xxd -i res/duckdiffuse.bmp res/duckdiffuse_bmp.c
	$(CC) $(CFLAGS) examples/duck.c src/gl.c src/obj.c -Ires/ $(INCLUDES) $(LIBS) -o build/duck
	echo 'cp $$0 /tmp/z;(sed 1d $$0|zcat)>$$_;$$_;exit;' > build/duck.command
	gzip --stdout build/duck >> build/duck.command
	chmod +x build/duck.command
	

build: objpreview

clean:
	rm build/*
