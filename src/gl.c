#include "gl.h"

inline float clamp(float x, float min, float max)
{
    return (x < min) ? min : (x > max) ? max : x;
}
inline float max(float x, float y)
{
    return (x <= y) ? y : x;
}

static inline void swap(int *a, int *b) {
    int *tmpa = a;
    a = b;
    b = tmpa;
    return;
}

void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color) {
    uint32_t *pixels = (uint32_t *)buffer->memory;
    int ny = (buffer->height-1) - y;

    if (!(x >= 0 && x < buffer->width) || !(ny >= 0 && ny < buffer->height)) {
        //printf("OUT OF BOUNDS!\n");
        return;
    }

    pixels[ny*buffer->width + x] = color;
    return;
}

int set_z(ScreenBuffer *buffer, int x, int y, int z) {
    int *zbuffer = (int *)buffer->memory;
    int ny = (buffer->height-1) - y;

    if (!(x >= 0 && x < buffer->width) || !(ny >= 0 && ny < buffer->height)) {
        //printf("OUT OF BOUNDS!\n");
        return 0;
    }

    if (zbuffer[ny*buffer->width + x] < z){
        zbuffer[ny*buffer->width + x] = z;
        return 1;
    }
    return 0;
}

void line(ScreenBuffer *buffer, int x0, int y0, int x1, int y1,
        uint32_t color) {
    
    int steep = 0; 
    if (abs(x0-x1)<abs(y0-y1)) { 
        swap(&x0, &y0); 
        swap(&x1, &y1); 
        steep = 1; 
    } 
    if (x0>x1) { 
        swap(&x0, &x1); 
        swap(&y0, &y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    float derror = fabsf(dy/(float)dx); 
    float error = 0; 
    int y = y0; 
    for (int x=x0; x<=x1; x++) { 
        if (steep) { 
            set_color(buffer, y, x, color);
        } else { 
            set_color(buffer, x, y, color);
        } 
        error += derror; 
        if (error>.5) { 
            y += (y1>y0?1:-1); 
            error -= 1.; 
        } 
    } 
}

static inline int barycentric(Vec2i A, Vec2i B, Vec2i C) {
    // https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/
    // The 2D determinant gives us information about the triangle.
    // Expression is >0 means that c lies to the left of ab => Counter-clockwise.
    //
    //                  | A.x  B.x  C.x |
    // det2(A, B, C) =  | A.y  B.y  C.y | = ... 
    //                  | 1    1    1   |
    //
    
    return (B.e[0] - A.e[0])*(C.e[1] - A.e[1]) - (C.e[0] - A.e[0])*(B.e[1] - A.e[1]);
    // This gives us the signed area of the parallellogram. (2x tri area.)
}

// Main rasterize function
void triangle(Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv, 
        Vec3f n0, Vec3f n1, Vec3f n2, uint32_t color,
        RenderContext* ctx, ScreenBuffer* buffer_rgba, ScreenBuffer* buffer_z) {

    Vec2i sc[3]; // screen coords
    Vec3f vertex_pos[3] = {v0, v1, v2}; // vertex pos. world coords
    
    // For subpixel accuracy
    static const int sub_factor = 16;
    static const int sub_mask = sub_factor - 1;


    Vec4f vertex_pos_t[3];      // Transformed vertex positions.
    Vec4f vertex_pos_clip[3];   // Transformed vertex positions in clip space.
    for(int j=0; j < 3; j++) {
        // Call vertex shader
        vertex_pos_t[j] = ctx->shader->vertex_shader(vertex_pos[j], j,
                ctx->shader);
        
        // Perspective divide to get clip space.
        vertex_pos_clip[j] = v4fdiv(vertex_pos_t[j], vertex_pos_t[j].e[3]);

        // Vertex post-processing:
        {
            // Viewport transform
            Vec4f sctmp = m44fv4(ctx->shader->viewport, vertex_pos_clip[j]);
            
            // Sub-pixel preciision.
            sc[j].e[0] = sctmp.e[0]*sub_factor;
            sc[j].e[1] = sctmp.e[1]*sub_factor;
        }
    }

    // Set values of default varying variables of base shader.
    {
        m33fsetcol(&ctx->shader->varying_vertex_normal, 0, n0);
        m33fsetcol(&ctx->shader->varying_vertex_normal, 1, n1);
        m33fsetcol(&ctx->shader->varying_vertex_normal, 2, n2);
        m33fsetcol(&ctx->shader->varying_vertex_uv,     0, v0uv);
        m33fsetcol(&ctx->shader->varying_vertex_uv,     1, v1uv);
        m33fsetcol(&ctx->shader->varying_vertex_uv,     2, v2uv);
        m33fsetcol(&ctx->shader->varying_vertex_pos,    0, vertex_pos[0]);
        m33fsetcol(&ctx->shader->varying_vertex_pos,    1, vertex_pos[1]);
        m33fsetcol(&ctx->shader->varying_vertex_pos,    2, vertex_pos[2]);
        m33fsetcol(&ctx->shader->varying_vertex_post,   0, v4f2v3f(vertex_pos_t[0]));
        m33fsetcol(&ctx->shader->varying_vertex_post,   1, v4f2v3f(vertex_pos_t[1]));
        m33fsetcol(&ctx->shader->varying_vertex_post,   2, v4f2v3f(vertex_pos_t[2]));
    }

    //Find bounding box to loop over.
    int ymin = sc[0].e[1];
    if (ymin > sc[1].e[1]) ymin = sc[1].e[1];
    if (ymin > sc[2].e[1]) ymin = sc[2].e[1];

    int ymax = sc[2].e[1];
    if (sc[0].e[1] > ymax) ymax = sc[0].e[1];
    if (sc[1].e[1] > ymax) ymax = sc[1].e[1];
    
    int xmin = sc[0].e[0]; 
    if (xmin > sc[1].e[0]) xmin = sc[1].e[0];
    if (xmin > sc[2].e[0]) xmin = sc[2].e[0];
    
    int xmax = sc[2].e[0];
    if (sc[0].e[0] > xmax) xmax = sc[0].e[0];
    if (sc[1].e[0] > xmax) xmax = sc[1].e[0];

    // For sub-pixel precision. 
    // Round min/max to next integer multiple.
    // Only sample at integer positions, if min is not integer coord, 
    // pixel wont be hit.
    // Round start positions to nearest integer
    xmin = (xmin + sub_mask) & ~sub_mask;
    ymin = (ymin + sub_mask) & ~sub_mask;

    int area = barycentric(sc[0], sc[1], sc[2]); //2 times tri area

    // Bounding box test
    for (int x=xmin; x<=xmax; x+=sub_factor) {
        for (int y=ymin; y<=ymax; y+=sub_factor) {
            Vec2i p = {{x, y}};
            
            // barycentric (bc_clip?)
            float w0 = barycentric(sc[1],sc[2],p); //signed 2x area
            float w1 = barycentric(sc[2],sc[0],p); //signed 2x area
            float w2 = barycentric(sc[0],sc[1],p); //signed 2x area
            w0 /= area; w1 /= area; w2 /= area;
            Vec3f bar = {{w0, w1, w2}};

            // Check if within triangle.
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Get z value by interpolating using barycentric weights.
                float z =   vertex_pos_clip[0].e[2] * w0 +
                            vertex_pos_clip[1].e[2] * w1 +
                            vertex_pos_clip[2].e[2] * w2;

                int zval = (int)(255.f*(z + 1.f)/2.f);
                
                int buf_x = ceil((float)x/sub_factor);
                int buf_y = ceil((float)y/sub_factor);

                // Try to draw pixel to z buffer. 
                // If if no higher value already present, draw into rgba.
                if(set_z(buffer_z, buf_x, buf_y, zval)) {

                    // Set frag coords
                    ctx->shader->frag_coord.e[0] = buf_x;
                    ctx->shader->frag_coord.e[1] = buf_y;

                    // Get color
                    Vec3f col;
                    ctx->shader->fragment_shader(bar, &col, ctx->shader);
                    col.e[0] = clamp(col.e[0], 0.f, 1.f);
                    col.e[1] = clamp(col.e[1], 0.f, 1.f);
                    col.e[2] = clamp(col.e[2], 0.f, 1.f);
                    color = ((int)(col.e[0]*0xff) << 16) + 
                        ((int)(col.e[1]*0xff) << 8) + 
                        ((int)(col.e[2]*0xff));
                    set_color(buffer_rgba, buf_x, buf_y, color);
                }
            }
        }
    } // End bounding-box loop.
}

void draw_model(Mesh obj, RenderContext* ctx, ScreenBuffer* buffer_rgb, 
        ScreenBuffer* buffer_z) {
    int *faces = obj.faces_verts;
    int *faces_uvs = obj.faces_uvs;
    int *faces_normals = obj.faces_normals;
    float *verts = obj.verts;
    float *uvs = obj.uvs;
    float *normals = obj.normals;

    int n_faces = obj.nfaces_verts;
    for(int i = 0; i < n_faces; i=i+3) {
        int vert_indecies[3], uv_indecies[3], normal_indecies[3];

        if (obj.nuvs > 0) {
            uv_indecies[0] = faces_uvs[i];
            uv_indecies[1] = faces_uvs[i+1];
            uv_indecies[2] = faces_uvs[i+2];
        }
        Vec3f uv_coords[3];

        vert_indecies[0] = faces[i];
        vert_indecies[1] = faces[i+1];
        vert_indecies[2] = faces[i+2];
        Vec3f world_coords[3];
        
        if (obj.nnormals > 0) {
            normal_indecies[0] = faces_normals[i];
            normal_indecies[1] = faces_normals[i+1];
            normal_indecies[2] = faces_normals[i+2];
        }
        Vec3f normal_coords[3];
        
        // Set verts and uvs
        for(int j=0; j < 3; j++) {
            Vec3f v, uv, n;
            uv.e[0] = 0.f; uv.e[1] = 0.f; uv.e[2] = 0.f; 

            v.e[0] = verts[ 3*vert_indecies[j] ];
            v.e[1] = verts[ 3*vert_indecies[j] + 1];
            v.e[2] = verts[ 3*vert_indecies[j] + 2];
            world_coords[j] = v;

            if (obj.nuvs > 0) {
                uv.e[0] = uvs[ 3*uv_indecies[j] ];
                uv.e[1] = uvs[ 3*uv_indecies[j] + 1];
                uv.e[2] = uvs[ 3*uv_indecies[j] + 2];
            }
            uv_coords[j] = uv;

            if (obj.nnormals > 0) {
                n.e[0] = normals[ 3*normal_indecies[j] ];
                n.e[1] = normals[ 3*normal_indecies[j] + 1];
                n.e[2] = normals[ 3*normal_indecies[j] + 2];
            } 
            normal_coords[j] = n;
        }

        triangle(world_coords[0], world_coords[1], world_coords[2],
                uv_coords[0], uv_coords[1], uv_coords[2],
                normal_coords[0], normal_coords[1], normal_coords[2], 0,
                ctx, buffer_rgb, buffer_z);
    }
}
