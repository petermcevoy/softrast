#ifndef __PMMGL_H__
#define __PMMGL_H__

#include <cstdlib>
#include <cmath>
#include <vector>
#include "linalg.h"
#include "tgaimage.h"

extern TGAImage *texture;
extern Matrix projection;
extern Matrix ModelView;


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
    Matrix projection;
    Matrix modelview;
    ScreenBuffer* buffers;
    int num_buffers;
} RenderContext;

// Sets given pixel in the given buffer to the color
void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color);

// Sets the zbuffer at the pixel in the given buffer.
int set_z(ScreenBuffer *buffer, int x, int y, int z);

// Draws line in buffer from p0 to p1 with color.
void line(ScreenBuffer *buffer, Vec2i p0, Vec2i p1, uint32_t color);

// Draws and fills triangle with verticies v0, v1, v2 and corresponding
// uv coordinates.
void triangle(ScreenBuffer *buffer, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv, uint32_t color);

// Struct to help store models.
typedef struct {
    std::vector<int> *faces_verts;  //sets of 3, indecies for vertex positions
    std::vector<int> *faces_uvs;    //sets of 3, indecies for uv coordinates
    std::vector<float> *verts;      //stored as v0.x v0.y v0.z v1.x v.1.y ...
    std::vector<float> *uvs;          //stored as uv0.x uv0.y uv0.z uv1.x ...
} Model;


// Function to draw triangles with uv for a model file.
void draw_model(RenderContext* ctx, Model obj);

// Projection transform. Turns 4x4 matrix into a 3x1 vector with coordinates
// scaled according to projection.
static inline Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
};

// Turns 3x1 vector into 4x1 vector.
static inline Matrix v2m(Vec3f v) {
    Matrix m(4,1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

// Viewport projection. Maps [-1, 1]x[-1,1] to [0, w]x[0,h].
static inline Matrix viewport(int x, int y, int w, int h) {
    int depth = 255; //TODO assign somehow.
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = 1.f;
    
    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

static inline void lookat(RenderContext* ctx, Vec3f eye, Vec3f center, Vec3f up) {
    //camera always points along z
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();

    //make transformation matrix
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    Minv[0][0] = x.x;
    Minv[0][1] = x.y;
    Minv[0][2] = x.z;
    
    Minv[1][0] = y.x;
    Minv[1][1] = y.y;
    Minv[1][2] = y.z;
    
    Minv[2][0] = z.x;
    Minv[2][1] = z.y;
    Minv[2][2] = z.z;

    Tr[0][3] = -center.x;
    Tr[1][3] = -center.y;
    Tr[2][3] = -center.z;

    //return Minv*Tr;

    ctx->modelview = Minv*Tr;
}

#endif
