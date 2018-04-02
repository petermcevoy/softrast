#include <SDL2/SDL.h>
#include "gl.h"
#include "obj.h"
#include "example_shaders.h"
#include "sdlutil.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#define PI 3.14159265359

#include "duck_wav.c"
#include "duckdiffuse_bmp.c"
#include "duckpoly_obj.c"

static Uint8* duck_buf;
static Uint32 duck_buf_len;

static Mesh obj;

typedef struct {
    ShaderBase base;
    SDL_Surface* diffuse_tex;
    Vec3f ambient_light;
    Vec3f light;
    Vec3f light_pos;
    float diffuse_amount;
    float specular_amount;
    float specular_falloff;
} ShaderDuck;
int shader_duck_fragment(Vec3f bar, Vec3f *color, void *data) {
    ShaderDuck *sdata = (ShaderDuck *)data;
    Vec3f normal = m33fv3(sdata->base.varying_vertex_normal, bar);
    Vec3f pos = m33fv3(sdata->base.varying_vertex_post, bar);

    Vec3f L = v3fnormalize(v3fsub(sdata->light_pos,pos));
    Vec3f E = v3fnormalize(v3fmul(pos, 1.f)); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    Vec3f R = v3fnormalize(v3fmul(reflect(L,normal), -1.f)); 
    
    // Get texture pixel value.
    Vec3f uv = m33fv3(sdata->base.varying_vertex_uv, bar);
    Uint32 x = uv.e[0]*(float)sdata->diffuse_tex->w;
    Uint32 y = (1.f-uv.e[1])*(float)sdata->diffuse_tex->h;
    Uint8 *col = &sdata->diffuse_tex->pixels[y*sdata->diffuse_tex->pitch + x*sdata->diffuse_tex->format->BytesPerPixel];
    float r = (float)(*(col + 2))/255.f;
    float g = (float)(*(col + 1))/255.f;
    float b = (float)(*(col + 0))/255.f;
    Vec3f texrgb = {{r, g, b}};

    Vec3f white = {{.5f, .5f, .5f}};
    Vec3f ambient  = sdata->ambient_light;
    Vec3f diffuse  = v3fmul(sdata->light, clamp(v3fdot(normal, L), 0.f, 1.f));
    Vec3f specular = v3fmul(f2v3f(sdata->specular_amount),
            pow(max(v3fdot(R, E), 0.f),sdata->specular_falloff));
    
    Vec3f rgb = v3femul(texrgb, ambient);
    rgb = v3fadd(rgb, v3femul(texrgb, diffuse));
    //rgb = v3fadd(rgb, specular);
    
    v3fset(color, rgb);

    return 0;
}
ShaderDuck duck_shader;

static SDL_Surface *diffuse_tex;

// Render function, called from loop in main.
void render(RenderContext* ctx, int count) {
    // Clear main buffer to black.
    uint32_t bgcol = 0x444444;
    memset(ctx->buffers[0].memory, bgcol, ctx->buffers[0].height*ctx->buffers[0].width*(sizeof(uint32_t)));
    memset(ctx->buffers[1].memory, 0, ctx->buffers[1].height*ctx->buffers[1].width*(sizeof(int)));

    uint32_t *pixels = (uint32_t *)ctx->buffers[0].memory;
    float t = 0.008f*count;

    {
        uint32_t stripecol = 0xA64C08;
        for (int x = 0; x < ctx->buffers[0].width; x++) {
            for (int y = 0; y < ctx->buffers[0].height; y++) {
                int val = (int)((float)x + (float)y + t*250) % 200;
                if (50 <= val && val <= 100) {
                    set_color(&ctx->buffers[0], x, y, stripecol);
                }
            }
        }
    }
    

    // Set Projection and view
    {
        float varx = sinf(t*1.f);
        float varz = cosf(t*1.f);
        static float r = 5.f;
        Mat44f proj = perspective(50.f, 4.f/3.f, -4.f, -6.5f);
        m44fset(&ctx->projection, proj);

        Vec3f eye = {{r*varx, -1.f, r*varz}};
        Vec3f c = {{0.f,0.3f,0.f}};
        Vec3f up = {{0.f,1.f,0.f}};
        lookat(ctx, eye, c, up);
    }
    
    // Set shader vars.
    {
        Vec3f ambient_light = {{0.15f, 0.15f, 0.15f}};
        v3fset(&duck_shader.ambient_light, ambient_light);
        Vec3f light = {{.4f, .4f, .4f}};
        v3fset(&duck_shader.light, light);
        Vec3f light_pos = {{10.f, 4.f, 6.f}};
        v3fset(&duck_shader.light_pos, light_pos);
        duck_shader.diffuse_amount = .8f;
        duck_shader.specular_amount = .2f;
        duck_shader.specular_falloff = 25.f;

        duck_shader.diffuse_tex = diffuse_tex;
        duck_shader.base.modelview = ctx->modelview;
        duck_shader.base.projection = ctx->projection;
        duck_shader.base.mvp = m44fm44f(ctx->projection, ctx->modelview);
        duck_shader.base.viewport = ctx->viewport;
    }
    
    // Assign shader pair to render context.
    ctx->shader = (ShaderBase *)&duck_shader;
    draw_model(ctx, obj);
}

typedef struct {
    int tick;
    int n;
} SongEntry;

#define key_a -3 + 0
#define key_b -1 + 0
#define key_c 0  + 0
#define key_d 2  + 0
#define key_e 4  + 0
#define key_f 5  + 0
#define key_g 7  + 0
SongEntry song[] = {
    {0  +   0,      key_e},
    {0  +   4,      key_b},
    {0  +   6,      key_c},
    {0  +   8,      key_d},
    {0  +   10,     key_e},
    {0  +   11,     key_d},
    {0  +   12,     key_c},
    {0  +   14,     key_b},
    {0  +   16,     key_a},
    {0  +   20,     key_a},
    {0  +   22,     key_c},
    {0  +   24,     key_e},
    
    {26 +   2,      key_d},
    {26 +   4,      key_c},
    {26 +   6,      key_b},
    {26 +   10,     key_b},
    {26 +   11,     key_b},
    {26 +   12,     key_c},
    {26 +   14,     key_d},
    {26 +   18,     key_e},
    {26 +   22,     key_c},
    
    {52 +   0,      key_a},
    {52 +   4,      key_a},
    {52 +   12,     key_d},
    {52 +   16,     key_d},
    {52 +   18,     key_f},
    {52 +   20,     key_a + 12},
    {52 +   24,     key_g},
    
    {78 +   0,      key_f},
    {78 +   2,      key_e},
    {78 +   6,      key_e},
    {78 +   8,      key_c},
    {78 +   10,     key_e},
    {78 +   14,     key_d},
    {78 +   16,     key_c},
    {78 +   18,     key_b},
    {78 +   22,     key_b},
    {78 +   23,     key_b},
    {78 +   24,     key_c},
    
    {104 +   0,     key_d},
    {104 +   4,     key_e},
    {104 +   8,     key_c},
    {104 +   12,    key_a},
    {104 +   16,    key_a},
    {104 +   24,    key_e},
};


static int songsampleticks = 0;
static int songticks = 0;
static int samplespertick = 1500; //11025/4;
static int entryindex = 0; //11025/4;
void song_trigger(int *phase, float *phase_fraction, float *playback_speed) {
    
    // Get next song entry. See if it is meant for current tick.
    SongEntry entry = song[entryindex];
    if (entry.tick == songticks) {
        // Determine playback speed.
        // fn = f0 * (a)^n
        float fn = 0.0f + powf(1.059463094359, (float)entry.n);
        *phase = 0;
        *phase_fraction = 0.f;
        *playback_speed = fn;
        entryindex++;
    }

    songsampleticks++;
    if (songsampleticks >= samplespertick) {
        songticks += songsampleticks/samplespertick;
        songsampleticks = songsampleticks % samplespertick;

        if ((songticks % 128) == 0) {
            entryindex = 0;
            songticks = 0;
        }
    }
    return;
}

static inline float lerp(float a, float b, float t) {
    return a*(1.f-t) + t*b;
}

static int phase_count = 0;
static int phase = 0;
static float phase_fraction = 0; 
static float playback_speed = 1.f; 
void audio_callback(void *userdata, Uint8 *stream, int len) {
    int nsamples = len/sizeof(float);
    float *streamf = (float *) stream;
    int buf_len = duck_buf_len;

    for (int i=0; i < nsamples; i++) {
        // Call song_trigger to make sound to retrigger and/or playback speed 
        // should change.

        song_trigger(&phase, &phase_fraction, &playback_speed);
        
        if (phase <= buf_len/sizeof(Uint8)) {


            // Generate start/end envelope to avoid clicking.
            static int margin = 360;
            float envelope = 1.f;
            if (phase >= 0 && phase <= margin) {
                envelope = (float)phase/margin;
            } else if (phase >= buf_len-margin && phase <= buf_len) {
                envelope = (float)(buf_len-phase)/margin;
            }

            // Get sample.
            int current_phase = phase;
            int next_phase = phase + (int)ceil(playback_speed);

            float current_val = duck_buf[current_phase]/255.f;
            float next_val = duck_buf[next_phase]/255.f;

            float samplevalue = lerp(current_val, next_val, phase_fraction);
            //float t = (float)(phase_count++)/44100.f;
            //samplevalue = sin(440.f*t*playback_speed*2*PI);

            // Set new phase.
            phase = phase + (int)(floor(playback_speed + phase_fraction));
            double integer;
            phase_fraction = modf(phase_fraction + playback_speed, &integer);
            
            // Linear interpolate the next sample if needed.

            streamf[i] = 0.5*samplevalue * envelope;
        } else {
            streamf[i] = 0.f;
        }
    }
}

int main(int argc, char **argv) {
    SDL_Renderer *renderer;
    sdl_init(SCREEN_WIDTH, SCREEN_HEIGHT, "duck", &renderer);

    // Make custom buffer to interface with program.
    RenderContext ctx;
    ctx.projection = m44fident();
    ctx.modelview = m44fident();
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

    m44fset(&ctx.viewport, m44fident());
    m44fsetel(&ctx.viewport, 0, 0, buffers[0].width/2);
    m44fsetel(&ctx.viewport, 1, 1, buffers[0].height/2);
    m44fsetel(&ctx.viewport, 2, 2, 0.f);
    // Translation
    m44fsetel(&ctx.viewport, 0, 3, buffers[0].width/2);
    m44fsetel(&ctx.viewport, 1, 3, buffers[0].height/2);

    // Load obj file
    FILE *fp = fmemopen(res_duckpoly_obj, res_duckpoly_obj_len, "r");
    if (load_obj_mem(fp, &obj) != 0) {
        printf("Error: Could not load file. Exiting.");
        return 1;
    }

    // Load textures
    SDL_RWops *texsrc = SDL_RWFromConstMem(res_duckdiffuse_bmp, res_duckdiffuse_bmp_len);
    diffuse_tex = SDL_LoadBMP_RW(texsrc, 0);
    //diffuse_tex = SDL_LoadBMP("examples/duckdiffuse.bmp");
    
    // Setup shaders
    duck_shader.base.vertex_shader = &shader_phong_vertex;
    duck_shader.base.fragment_shader = &shader_duck_fragment;

    // Start Audio
    sdl_start_audio(audio_callback);
    SDL_RWops *src = SDL_RWFromConstMem(res_duck_wav, res_duck_wav_len);
    SDL_AudioSpec spec;
    SDL_memset(&spec, 0, sizeof(spec));
    spec.freq = 11025; //48000;
    spec.channels = 1;
    spec.format = AUDIO_U8; //AUDIO_F32SYS;
    
    Uint8* audio_buf;
    SDL_LoadWAV_RW(src, 0, &spec, &audio_buf, &duck_buf_len);
    //SDL_LoadWAV("examples/KBoC_018.wav", &spec, &audio_buf, &duck_buf_len);
    printf("WAV freq: %d, samples: %d, chans: %d, format: %d \n", spec.freq, spec.samples, spec.channels, spec.format);
    duck_buf = (Uint8*)audio_buf;
    

    // Main display
    int running = 1;
    int count = 0;
    while(running) {
        //Render video
        render(&ctx, count++);
        uint32_t *pixel = (uint32_t *)ctx.buffers[0].memory;
        
        sdl_copy_rgb_to_window_buffer(pixel, renderer);

        running = (sdl_is_escape_pressed() == 0);
    }
    
    //for (int i=0; i < ctx.num_buffers; i++) {
    //    free(ctx.buffers[i].memory);
    //}   

    return 0;
}
