#include "gl.h"

void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color) {
    uint32_t *pixels = (uint32_t *)buffer->memory;
    int ny = (buffer->height-1) - y;

    if (!(x >= 0 && x < buffer->width) || !(ny >= 0 && ny < buffer->height)) {
        printf("OUT OF BOUNDS!\n");
        //assert(0);
    }

    pixels[ny*buffer->width + x] = color;
}

int set_z(ScreenBuffer *buffer, int x, int y, int z) {
    int *zbuffer = (int *)buffer->memory;
    int ny = (buffer->height-1) - y;
    if (!(x >= 0 && x < buffer->width) || !(ny >= 0 && ny < buffer->height)) {
        printf("OUT OF BOUNDS!\n");
        //assert(false);
    }
    if (zbuffer[ny*buffer->width + x] < z){
        zbuffer[ny*buffer->width + x] = z;
        return 1;
    }
    return 0;
}

static inline void swap(int *a, int *b) {
    int *tmpa = a;
    a = b;
    b = tmpa;
    return;
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

int barycentric(Vec2i A, Vec2i B, Vec2i C) {
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
void triangle(RenderContext* ctx, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv, uint32_t color) {

    // For the moment, assume we want to draw into these buffers...
    ScreenBuffer *buffer_rgba = &ctx->buffers[0];
    ScreenBuffer *buffer_z = &ctx->buffers[1];
    
    Vec2i sc[3]; // screen coords
    Vec3f wc[3] = {v0, v1, v2}; // world coords
    
    //TODO Move the following out to vertex shader.
    Mat44f viewport = m44fident();
    m44fsetel(&viewport, 0, 0, buffer_rgba->width/5);
    m44fsetel(&viewport, 1, 1, buffer_rgba->height/5);
    m44fsetel(&viewport, 2, 2, 1.f);

    // Translation
    m44fsetel(&viewport, 0, 3, buffer_rgba->width/2);
    m44fsetel(&viewport, 1, 3, buffer_rgba->height/2);
    m44fsetel(&viewport, 2, 3, 1.f);

    // For subpixel accuracy
    static const int sub_factor = 16;
    static const int sub_mask = sub_factor - 1;

    // Set screen_cords
    for(int j=0; j < 3; j++) {
        Mat44f mtmp = m44fm44f(ctx->projection, m44fm44f(ctx->modelview,v2m(wc[j])));
        Vec3f wctmp = m2v(mtmp);
        wc[j] = wctmp;

        // Make screen coordinates have subpixel accuracy.
        {
            Vec3f sctmp = m2v(m44fm44f(viewport,mtmp));
            Vec2i sctmp2 = {{sctmp.e[0]*sub_factor, sctmp.e[1]*sub_factor}};
            sc[j] = sctmp2;
        }
    }

    // Triangle normal
    Vec3f n = v3fcross(v3fsub(wc[2], wc[0]), v3fsub(wc[1], wc[0]));
    n = v3fnormalize(n);

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

            // Check if within triangle.
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Render pixel code

                // Get z value by interpolating using barycentric weights.
                float z = wc[0].e[2] * w0 + wc[1].e[2] * w1 + wc[2].e[2] * w2;
                int zval = (int)(255.f*(z + 1.f)/2.f);
                
                // UV texture color Test
                Vec3f v0col = {{1.f,0,0}};
                Vec3f v1col = {{0,1.f,0}};
                Vec3f v2col = {{0,0,1.f}};
                Vec3f rgb = v3fmul(v0col, w0);
                rgb = v3fadd(rgb, v3fmul(v1col, w1));
                rgb = v3fadd(rgb, v3fmul(v2col, w2));

                color = ((int)(rgb.e[0]*0xff) << 16) + 
                    ((int)(rgb.e[1]*0xff) << 8) + 
                    ((int)(rgb.e[2]*0xff));

                int buf_x = ceil((float)x/sub_factor);
                int buf_y = ceil((float)y/sub_factor);

                // Try to draw pixel to z buffer. If successfull, draw into 
                // rgba buffer.
                if(set_z(buffer_z, buf_x, buf_y, zval)) {
                    /*Vec3f uv = v0uv * w0 + v1uv * w1 + v2uv * w2;
                      int tex_x = (int)(uv.x*(float)texture->get_width());
                      int tex_y = (int)((1.f-uv.y)*(float)texture->get_height());
                      TGAColor tex_color = texture->get(tex_x, tex_y);
                      color = (tex_color.bgra[0]) + 
                      (tex_color.bgra[1] << 8) + 
                      (tex_color.bgra[2] << 16);*/
                    set_color(buffer_rgba, buf_x, buf_y, color);
                }
            }
        }
    }
}

void draw_model(RenderContext* ctx, Model obj) {
    int *faces = obj.faces_verts;
    int *faces_uvs = obj.faces_uvs;
    float *verts = obj.verts;
    float *uvs = obj.uvs;

    int n_faces = obj.nfaces_verts;
    for(int i = 0; i < n_faces; i=i+3) {
        int vert_indecies[3], uv_indecies[3];

        uv_indecies[0] = faces_uvs[i];
        uv_indecies[1] = faces_uvs[i+1];
        uv_indecies[2] = faces_uvs[i+2];
        Vec3f uv_coords[3];

        vert_indecies[0] = faces[i];
        vert_indecies[1] = faces[i+1];
        vert_indecies[2] = faces[i+2];

        Vec3f world_coords[3];
        
        // Set verts and uvs
        for(int j=0; j < 3; j++) {
            Vec3f v, uv;

            uv.e[0] = uvs[ 3*uv_indecies[j] ];
            uv.e[1] = uvs[ 3*uv_indecies[j] + 1];
            uv.e[2] = uvs[ 3*uv_indecies[j] + 2];
            uv_coords[j] = uv;

            v.e[0] = verts[ 3*vert_indecies[j] ];
            v.e[1] = verts[ 3*vert_indecies[j] + 1];
            v.e[2] = verts[ 3*vert_indecies[j] + 2];
            world_coords[j] = v;
        }

        // Determine triangle color
        Vec3f n = v3fcross(v3fsub(world_coords[2], world_coords[0]),
                v3fsub(world_coords[1], world_coords[0]));
        n = v3fnormalize(n);

        Vec3f light_dir = {{0.f,0.f, -1.f}};
        float intensity = v3fdot(n, light_dir);
            triangle(ctx, world_coords[0], world_coords[1], world_coords[2],
                    uv_coords[0], uv_coords[1], uv_coords[2], 0);
    }
}
