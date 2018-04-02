#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

// Helper for setting up window and SDL.

void *pixel_mem;
SDL_Texture *window_buffer_tex;

int sdl_init(int width, int height, char *window_title, SDL_Renderer **renderer) {
    // ====================== init code ================
    int window_width = width; 
    int window_height = height;

    // SDL init
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0){
		printf("\n t %s \n",(const char *)strcat("SDL_Init Error: ",(const char *)SDL_GetError()));
		return 1;
	}
    
    // Window
    SDL_Surface *screen;
    SDL_Surface *offscreenBuffer;
    SDL_Window *window;
    window = SDL_CreateWindow(window_title, 64, 64,
            window_width, window_height, SDL_WINDOW_SHOWN);
    if (window == NULL){
		printf("\n %s \n",strcat("SDL_CreateWindow Error: ",SDL_GetError()));
	    SDL_Quit();
        return 1;
    }

    // SDL renderer
    *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"); 

    // Textures TODO NAME
    window_buffer_tex = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    pixel_mem = malloc(window_width*window_height*4);



    return 0;
}

void sdl_start_audio(void (*audio_callback)(void*, Uint8*, int)) {
    // Audio
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID audio_dev;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 11025; //48000;
    want.channels = 1;
    want.format = AUDIO_F32SYS;
    want.callback = audio_callback;

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
            SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (audio_dev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(audio_dev, 0); /* start audio playing. */
    }
    printf("freq: %d, samples: %d, chans: %d \n", have.freq, have.samples, have.channels);
}



void sdl_copy_rgb_to_window_buffer(uint32_t* read_pixel, SDL_Renderer *renderer) {
    int window_width;
    int window_height;
    SDL_GetRendererOutputSize(renderer, &window_width, &window_height);

    //Transfer custom buffer to sdl texture
    uint32_t *pixels = (uint32_t *) pixel_mem;
    int Pitch = window_width*32;
    for(int Y = 0; Y < window_height; ++Y) {
        for(int X = 0; X < window_width; ++X) {
            uint8_t r = *read_pixel & 0xff; 
            uint8_t g = *read_pixel >> 8 & 0xff;
            uint8_t b = *read_pixel >> 16 & 0xff;
            read_pixel++;
            pixels[ (Y * window_width) + X ] = (b << 16) + (g << 8) + (r);
        }
    }

    SDL_UpdateTexture(window_buffer_tex, 0, pixel_mem, 4 * window_width);
    SDL_RenderCopy(renderer, window_buffer_tex, 0, 0);
    SDL_RenderPresent(renderer);

}

inline static int sdl_is_escape_pressed() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    return 1;
                }
                break;
        }
    }

    return 0;
}
