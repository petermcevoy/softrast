#include <SDL2/SDL.h>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <vector>
//#include "linalg.h"
//#include "tgaimage.h"

#include "gl.h"

TGAImage *texture;
Matrix projection = Matrix::identity(4);
Matrix ModelView;

//static Matrix projection = Matrix::identity(4);
//Matrix ModelView;
//static TGAImage *texture;

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static ScreenBuffer buffer;
static int running = 1;
static Model obj;

void load_obj(const char * filename, Model obj) {

    std::vector<int> *faces_verts = obj.faces_verts;
    std::vector<int> *faces_uvs = obj.faces_uvs;
    std::vector<float> *verts = obj.verts;
    std::vector<float> *uvs = obj.uvs;

    FILE *fp = fopen(filename, "r");
    char str[100];
    int line_len = 0;
    
    //Seek to vertecies
    while(1) {
        fgets(str, 100, fp);
        if( str[0] == 'v' ) {
            line_len = strcspn(str, "\n");
            printf("seeked to verts\n");
            break;
        }
    }
    fseek(fp , -1*line_len-1 , SEEK_CUR);
    

    while(1) {
        fgets(str, 100, fp);
        if( str[0] == 'v' ) {
            float vx, vy, vz;
            sscanf(str, "v %f %f %f", &vx, &vy, &vz);
            verts->push_back(vx);
            verts->push_back(vy);
            verts->push_back(vz);
        } else {
            break;
        }
    }

    //Seek to uv coordiantes
    while(1) {
        fgets(str, 100, fp);
        if( str[0] == 'v' && str[1] == 't') {
            line_len = strcspn(str, "\n");
            printf("seeked to uv_coords\n");
            break;
        }
    }
    fseek(fp , -1*line_len-1 , SEEK_CUR);
    
    
    while(1) {
        fgets(str, 100, fp);
        if( str[0] == 'v' && str[1] == 't') {
            float uvx, uvy, uvz;
            sscanf(str, "vt %f %f %f", &uvx, &uvy, &uvz);
            uvs->push_back(uvx);
            uvs->push_back(uvy);
            uvs->push_back(uvz);
        } else {
            break;
        }
    }

    while(1) {
        fgets(str, 100, fp);
        if( str[0] == 'f' ) {
            line_len = strcspn(str, "\n");
            printf("seeked to faces\n");
            break;
        }
    }
    fseek(fp , -1*line_len-1 , SEEK_CUR);
    
    //Loop through faces
    while(1) {
        fgets(str, 100, fp);
        if( str[0] == '#' ) {
            printf("skipping comment comment to faces\n");
            continue;
        }
        if( feof(fp) )
        {
            printf("EOF");
            break;
        }
        if (str[0] == 'f' && str[1] == ' ') {
            //set faces
            int vert_indecies[3];
            int uv_indecies[3];
            sscanf(str, "f %d/%d/%*d %d/%d/%*d %d/%d/%*d", 
                    &vert_indecies[0], &uv_indecies[0],
                    &vert_indecies[1], &uv_indecies[1],
                    &vert_indecies[2], &uv_indecies[2]);
            for(int i=0; i < 3; i++) {
                faces_verts->push_back(vert_indecies[i] - 1);
                faces_uvs->push_back(uv_indecies[i] - 1);
            }
        }

    }
    
    printf("\n\n# of verts %lu, # of vertids: %lu. \n\n\n", verts->size(), faces_verts->size());

    fclose(fp);
}

void render(ScreenBuffer *buffer, int count) {
    //clear to black
    memset(buffer->memory, 0, buffer->height*buffer->width*(sizeof(uint32_t)));
    memset(buffer->zbuffer, 0, buffer->height*buffer->width*(sizeof(int)));
    
    uint32_t *pixels = (uint32_t *)buffer->memory;

    //Line draw
    uint32_t white = 0xffffffff;
    uint32_t red = 0x00ff0000;
    uint32_t green = 0x0000ff00;
    uint32_t blue = 0x000000ff;
    //line(buffer, 0, 0, 100, 100, color);

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
    
    Vec3f t3[3] = {
        Vec3f(-0.8f, 0.0f, 0.9f),
        Vec3f(0.8f, -0.02f, 0.9f),
        Vec3f(0.0f, 0.001f, 0.9f)
    }; 
    //triangle(buffer, t3[0], t3[1], t3[2], t3[0], t3[1], t3[2], 0xffffffff);
    
    //triangle(buffer, t0[0], t0[1], t0[2], red);
    //triangle(buffer, t1[0], t1[1], t1[2], green);
    //triangle(buffer, t2[0], t2[1], t2[2], blue);

    //Load file
    //Model obj;
    //obj.faces_verts = new std::vector<int>;
    //obj.faces_uvs = new std::vector<int>;
    //obj.verts = new std::vector<float>;
    //obj.uvs = new std::vector<float>;
    //load_obj("res/head.obj", obj);

    //Adjust vertex positions.
    //Matrix projection = Matrix::identity(4);
    //Matrix viewport = Matrix::identity(4);
    //projection[3][2] = -1.f/cameraZ;

    float factor = std::abs(sin(count/15.f)*0.5);
    float factor2 = sin(count/5.f);
    float offset = 0;//factor*10.f;
    float offset2 = factor2*5.f;

    float t = count/20.f;

    //projection[3][2] = -1.f/cameraZ;
    Vec3f eye = Vec3f(10.f*sin(t),0.f, 3.f);//3.f*cos(t));
    Vec3f c = Vec3f(0.f,0.f,0.f);
    //projection[3][2] = -1.f/(c.z-eye.z);
    projection[3][2] = -1.f/(eye.z - c.z);
    Vec3f up = Vec3f(0.f,1.f,0.f);
    lookat(eye, c, up);
    
    //Load texture
    //texture = new TGAImage();
    //texture->read_tga_file("res/head_diffuse.tga");
    
    //rasterize_obj(buffer, zbuffer, verts, faces);
    draw_model(buffer, obj);

    //Texture
    //  vt u v, array of texture coordinates.
    //  number between // gives texture coordinates of this vertex of that traingle
    //  f x/x/x x/x/x x/x/x
    //  interpolate triangle and multiply by width and height of texture to get color.
    
    //wireframe(buffer);
}


int main() {
    // ====================== init code ================
    //SDL init
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		//printf("\n t %s \n",(const char *)strcat("SDL_Init Error: ",(const char *)SDL_GetError()));
		return 1;
	}
    
    //Window
    SDL_Surface *screen;
    SDL_Surface *offscreenBuffer;
    SDL_Window *window;
    window = SDL_CreateWindow("softOpenGL", 64, 64,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL){
		//printf("\n %s \n",strcat("SDL_CreateWindow Error: ",SDL_GetError()));
	    SDL_Quit();
        return 1;
    }

    //SDL renderer
    SDL_Renderer *Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1"); 

    //Textures TODO NAME
    SDL_Texture* Tex = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    //Make custom buffer to interface with program.
    buffer.width = SCREEN_WIDTH;
    buffer.height = SCREEN_HEIGHT;
    buffer.pitch = SCREEN_WIDTH*32;
    buffer.memory = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*32);
    buffer.zbuffer = (int *)malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int));

    //Check endianess
    uint32_t rmask, gmask, bmask, amask;
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;

    void* pixel_mem = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*4);
    
    //Load file
    //Model obj;
    obj.faces_verts = new std::vector<int>;
    obj.faces_uvs = new std::vector<int>;
    obj.verts = new std::vector<float>;
    obj.uvs = new std::vector<float>;
    load_obj("res/head.obj", obj);

    //Load texture
    texture = new TGAImage();
    texture->read_tga_file("res/head_diffuse.tga");
    
    int count = 0;
    //Main display
    while(running) {

        //Render video
        render(&buffer, count++);

        uint32_t *pixels = (uint32_t *) pixel_mem;
        if (1) {
        //Transfer custom buffer to sdl texture
        uint32_t *pixels = (uint32_t *) pixel_mem;
        uint32_t *Pixel = (uint32_t *)buffer.memory;
        int Pitch = SCREEN_WIDTH*32;
        for(int Y = 0; Y < SCREEN_HEIGHT; ++Y) {
            for(int X = 0; X < SCREEN_WIDTH; ++X) {
                uint8_t r = *Pixel & 0xff; 
                uint8_t g = *Pixel >> 8 & 0xff;
                uint8_t b = *Pixel >> 16 & 0xff;
                Pixel++;
                pixels[ (Y * SCREEN_WIDTH) + X ] = (b << 16) + (g << 8) + (r);
            }
        }
        }

        //ZBUFF
        if(0) {
            int *Pixel = (int *)buffer.zbuffer;
            int Pitch = SCREEN_WIDTH*32;
            for(int Y = 0; Y < SCREEN_HEIGHT; ++Y) {
                for(int X = 0; X < SCREEN_WIDTH; ++X) {
                    int p = *Pixel;
                    uint8_t val = (uint8_t)p;
                    uint8_t r = val & 0xff; 
                    uint8_t g = val & 0xff;
                    uint8_t b = val & 0xff;
                    Pixel++;
                    pixels[ (Y * SCREEN_WIDTH) + X ] = (b << 16) + (g << 8) + (r);
                }
            }
        }

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
    
    free(buffer.memory);

    return 0;
}
