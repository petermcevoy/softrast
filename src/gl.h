#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "linalg.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef enum {BUF_RGBA, BUF_Z} buffer_type;
typedef struct {
    buffer_type type;
    int depth;
    void *memory;
    int width;
    int height;
    int pitch; 
} ScreenBuffer;

typedef struct {
    Vec4f (*vertex_shader)(Vec3f, int, void*);
    int (*fragment_shader)(Vec3f, Vec3f*, void*);
    Mat44f projection;
    Mat44f modelview;
    Mat44f mvp; //modelview*projection
    Mat44f viewport;
    Mat33f varying_vertex_pos;
    Mat33f varying_vertex_post;
    Mat33f varying_vertex_normal;
} ShaderBase;

typedef struct {
    Mat44f projection;
    Mat44f modelview;
    Mat44f viewport;
    ShaderBase *shader;
    ScreenBuffer *buffers;
    int num_buffers;
} RenderContext;

float clamp(float x, float min, float max);

// Sets given pixel in the given buffer to the color
void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color);

// Sets the zbuffer at the pixel in the given buffer. 
// Returns 1 if successfully set value. 0 if not and existing value was higher.
int set_z(ScreenBuffer *buffer, int x, int y, int z);

// Draws line in buffer from (x0, y0) to (x1, y1) with color.
void line(ScreenBuffer *buffer, int x0, int y0, int x1, int y1, uint32_t color);

// Draws and fills triangle with verticies v0, v1, v2 and corresponding
// uv coordinates.
void triangle(RenderContext *ctx, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv,
        Vec3f n0, Vec3f n1, Vec3f n2, uint32_t color);

// Struct to represent 3d mesh models.
typedef struct {
    float *verts;           //stored as v0.x v0.y v0.z v1.x v.1.y ...
    float *uvs;             //stored as uv0.x uv0.y uv0.z uv1.x ...
    float *normals;         //stored as n0.x n0.y n0.z n1.x ...
    int *faces_verts;       //sets of 3, indecies for vertex positions
    int *faces_uvs;         //sets of 3, indecies for uv coordinates
    int *faces_normals;     //sets of 3, indecies for vertex normals

    int nverts;      
    int nuvs;        
    int nnormals;    
    int nfaces_verts;
} Mesh;

inline void free_model_data(Mesh obj) {
    free(obj.verts);
    free(obj.uvs);
    free(obj.normals);
    free(obj.faces_verts);
    free(obj.faces_uvs);
    return;
}

// Function to draw triangles with uv for a model file.
void draw_model(RenderContext* ctx, Mesh obj);

// Viewport projection. Maps [-1, 1]x[-1,1] to [0, w]x[0,h].
static inline Mat44f viewport(int x, int y, int w, int h) {
    int depth = 255; //TODO assign somehow.
    Mat44f m = m44fident();
    m.e[4*0 + 3] = x+w/2.f;
    m.e[4*1 + 3] = y+h/2.f;
    m.e[4*2 + 3] = 1.f;
    
    m.e[4*0 + 0] = w/2.f;
    m.e[4*1 + 1] = h/2.f;
    m.e[4*2 + 2] = depth/2.f;
    return m;
}

static inline void lookat(RenderContext* ctx, Vec3f eye, Vec3f center, Vec3f up) {
    // Camera always points along z
    Vec3f z = v3fnormalize(v3fsub(eye,center));
    Vec3f x = v3fnormalize(v3fcross(up, z));
    Vec3f y = v3fnormalize(v3fcross(z, x));

    // Make transformation matrix
    Mat44f minv = m44fident();
    Mat44f tr = m44fident();

    minv.e[4*0 + 0] = x.e[0];
    minv.e[4*0 + 1] = x.e[1];
    minv.e[4*0 + 2] = x.e[2];
    
    minv.e[4*1 + 0] = y.e[0];
    minv.e[4*1 + 1] = y.e[1];
    minv.e[4*1 + 2] = y.e[2];
    
    minv.e[4*2 + 0] = z.e[0];
    minv.e[4*2 + 1] = z.e[1];
    minv.e[4*2 + 2] = z.e[2];

    tr.e[4*0 + 3] = -center.e[0];
    tr.e[4*1 + 3] = -center.e[1];
    tr.e[4*2 + 3] = -center.e[2];

    m44fset(&ctx->modelview, m44fm44f(minv,tr));
    return;
}
