#include <SDL2/SDL.h>
#include "gl.h"
#include "obj.h"

// TODO:    *   Clean up triangle function, to fully use bary
//          *   Clean up api. Have rendering context and support vertex and
//              fragment shaders.

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static int running = 1;
static Model obj;
    
static uint32_t white = 0xffffffff;
static uint32_t red = 0x00ff0000;
static uint32_t green = 0x0000ff00;
static uint32_t blue = 0x000000ff;

// Render function, called from loop in main.
void render(RenderContext* ctx, int count) {
    // Clear main buffer to black.
    memset(ctx->buffers[0].memory, 0, ctx->buffers[0].height*ctx->buffers[0].width*(sizeof(uint32_t)));
    memset(ctx->buffers[1].memory, 0, ctx->buffers[1].height*ctx->buffers[1].width*(sizeof(int)));
    
    uint32_t *pixels = (uint32_t *)ctx->buffers[0].memory;
    ScreenBuffer buffer = ctx->buffers[0];

    // Line draw
    //line(&buffer, 0, 0, 100, 100, white);

    // Set Projection
    float t = 0.1f*count;
    Vec3f eye = {{5.f*sin(0.1*t),2.f*cos(0.2*t), 3.f}};//3.f*cos(t));
    Vec3f c = {{0.f,0.f,0.f}};
    m44fsetel(&ctx->projection, 3, 2, -1.f/(eye.e[2] - c.e[2]));
    Vec3f up = {{0.f,1.f,0.f}};
    lookat(ctx, eye, c, up);
    
    // Load texture
    // TODO...
    
    draw_model(ctx, obj);
}


int main() {
    // ====================== init code ================
    // SDL init
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("\n t %s \n",(const char *)strcat("SDL_Init Error: ",(const char *)SDL_GetError()));
		return 1;
	}
    
    // Window
    SDL_Surface *screen;
    SDL_Surface *offscreenBuffer;
    SDL_Window *window;
    window = SDL_CreateWindow("softGL", 64, 64,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL){
		printf("\n %s \n",strcat("SDL_CreateWindow Error: ",SDL_GetError()));
	    SDL_Quit();
        return 1;
    }

    // SDL renderer
    SDL_Renderer *Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"); 

    // Textures TODO NAME
    SDL_Texture* Tex = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Make custom buffer to interface with program.
    RenderContext ctx;
    ctx.projection = m44fident();
    ScreenBuffer buffers[2];
    ctx.buffers = buffers;

    buffers[0].type = BUF_RGBA;
    buffers[0].depth = sizeof(uint32_t);
    buffers[0].width = SCREEN_WIDTH;
    buffers[0].height = SCREEN_HEIGHT;
    buffers[0].pitch = SCREEN_WIDTH*buffers[0].depth;
    buffers[0].memory = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*buffers[0].depth);
    ctx.num_buffers++;

    buffers[1].type = BUF_Z;
    buffers[1].depth = sizeof(int);
    buffers[1].width = SCREEN_WIDTH;
    buffers[1].height = SCREEN_HEIGHT;
    buffers[1].pitch = SCREEN_WIDTH*buffers[1].depth;
    buffers[1].memory = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*buffers[1].depth);
    ctx.num_buffers++;

    // TODO: Check endianess
    uint32_t rmask = 0xff000000;
    uint32_t gmask = 0x00ff0000;
    uint32_t bmask = 0x0000ff00;
    uint32_t amask = 0x000000ff;

    void* pixel_mem = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*4);
    
    // Load obj file
    obj = load_obj("res/box.obj");

    //Load texture
    // TODO...
    
    // Main display
    int count = 0;
    while(running) {
        //Render video
        render(&ctx, count++);
        
        
        //Transfer custom buffer to sdl texture
        uint32_t *pixels = (uint32_t *) pixel_mem;
        uint32_t *pixel = (uint32_t *)ctx.buffers[0].memory;
        int Pitch = SCREEN_WIDTH*32;
        for(int Y = 0; Y < SCREEN_HEIGHT; ++Y) {
            for(int X = 0; X < SCREEN_WIDTH; ++X) {
                uint8_t r = *pixel & 0xff; 
                uint8_t g = *pixel >> 8 & 0xff;
                uint8_t b = *pixel >> 16 & 0xff;
                pixel++;
                pixels[ (Y * SCREEN_WIDTH) + X ] = (b << 16) + (g << 8) + (r);
            }
        }

        // Display ZBUFF
        //int *pixel = (int *)ctx.buffers[1].memory;
        //int pitch = SCREEN_WIDTH*32;
        //for(int Y = 0; Y < SCREEN_HEIGHT; ++Y) {
        //    for(int X = 0; X < SCREEN_WIDTH; ++X) {
        //        int p = *pixel;
        //        uint8_t val = (uint8_t)p;
        //        uint8_t r = val & 0xff; 
        //        uint8_t g = val & 0xff;
        //        uint8_t b = val & 0xff;
        //        pixel++;
        //        pixels[ (Y * SCREEN_WIDTH) + X ] = (b << 16) + (g << 8) + (r);
        //    }
        //}

        SDL_UpdateTexture(Tex, 0, pixel_mem, 4 * SCREEN_WIDTH);
        SDL_RenderCopy(Renderer, Tex, 0, 0);
        SDL_RenderPresent(Renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
              switch (event.type) {
                  case SDL_KEYDOWN:
                      if (event.key.keysym.sym == SDLK_ESCAPE) {
                          running = 0;
                      }
                      break;
              }
        }

        //SDL_Delay(100);
    }
    
    for (int i=0; i < ctx.num_buffers; i++) {
        free(ctx.buffers[i].memory);
    }   

    return 0;
}
