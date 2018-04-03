#include <SDL2/SDL.h>
#include "gl.h"
#include "obj.h"
#include "example_shaders.h"
#include "sdlutil.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static Mesh obj;
ShaderPhong phong_shader;

// Render function, called from loop in main.
void render(RenderContext* ctx, int count) {
    // Clear main buffer to black.
    memset(ctx->buffers[0].memory, 0, ctx->buffers[0].height*ctx->buffers[0].width*(sizeof(uint32_t)));
    memset(ctx->buffers[1].memory, 0, ctx->buffers[1].height*ctx->buffers[1].width*(sizeof(int)));
    
    uint32_t *pixels = (uint32_t *)ctx->buffers[0].memory;
    float t = 0.008f*count;

    // Set Projection and view
    {
        float varx = sinf(t*1.f);
        float vary = cosf(t*0.5f);
        float varz = cosf(t*1.f);
        static float r = 2.f;
        Mat44f proj = perspective(80.f, 4.f/3.f, -2.f, -4.5f);
        m44fset(&phong_shader.base.projection, proj);

        Vec3f eye = {{r*varx, vary, r*varz}};
        Vec3f c = {{0.f,0.f,0.f}};
        Vec3f up = {{0.f,1.f,0.f}};
        lookat(&phong_shader.base.modelview, eye, c, up);
        
        phong_shader.base.mvp = m44fm44f(phong_shader.base.projection,
                phong_shader.base.modelview);
    }
    
    // Set shader vars.
    {
        Vec3f ambient_light = {{0.15f, 0.01f, 0.01f}};
        v3fset(&phong_shader.ambient_light, ambient_light);
        Vec3f light = {{0.f, 0.f, .4f}};
        v3fset(&phong_shader.light, light);
        Vec3f light_pos = {{10.f, 4.f, 6.f}};
        v3fset(&phong_shader.light_pos, light_pos);
        phong_shader.diffuse_amount = .8f;
        phong_shader.specular_amount = .2f;
        phong_shader.specular_falloff = 25.f;
        phong_shader.count = count;

    }
    
    // Assign shader pair to render context.
    ctx->shader = (ShaderBase *)&phong_shader;
    draw_model(obj, ctx, &ctx->buffers[0], &ctx->buffers[1]);
}


int main(int argc, char **argv) {
    SDL_Renderer *renderer;
    sdl_init(SCREEN_WIDTH, SCREEN_HEIGHT, "softrast OBJ preview", &renderer);

    // Make custom buffer to interface with program.
    RenderContext ctx;
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

    // Load obj file
    char *filename = argv[1];
    if (load_obj(filename, &obj) != 0) {
        printf("Error: Could not load file. Exiting.");
        return 1;
    }

    // Setup shaders
    phong_shader.base.vertex_shader = &shader_phong_vertex;
    phong_shader.base.fragment_shader = &shader_phong_fragment;
    
    m44fset(&phong_shader.base.viewport, m44fident());
    m44fsetel(&phong_shader.base.viewport, 0, 0, buffers[0].width/2);
    m44fsetel(&phong_shader.base.viewport, 1, 1, buffers[0].height/2);
    m44fsetel(&phong_shader.base.viewport, 2, 2, 0.f);
    m44fsetel(&phong_shader.base.viewport, 0, 3, buffers[0].width/2);
    m44fsetel(&phong_shader.base.viewport, 1, 3, buffers[0].height/2);

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
    
    for (int i=0; i < ctx.num_buffers; i++) {
        free(ctx.buffers[i].memory);
    }   

    return 0;
}
