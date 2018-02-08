#include "gl.h"

void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color) {
    uint32_t *pixels = (uint32_t *)buffer->memory;
    int ny = (buffer->height-1) - y;
    if (!(0 >= x && x < buffer->width) || !(0 >= y && y < buffer->height)) {
        //assert(false);
    }
    pixels[ny*buffer->width + x] = color;
}

int set_z(ScreenBuffer *buffer, int x, int y, int z) {
    int *zbuffer = (int *)buffer->memory;
    int ny = (buffer->height-1) - y;
    if (!(0 >= x && x < buffer->width) || !(0 >= y && y < buffer->height)) {
        //assert(false);
    }
    if (zbuffer[ny*buffer->width + x] < z){
        zbuffer[ny*buffer->width + x] = z;
        return 1;
    }
    return 0;
}

void line(ScreenBuffer *buffer, Vec2i p0, Vec2i p1, uint32_t color) {
    int x0 = p0.x;
    int x1 = p1.x;
    int y0 = p0.y;
    int y1 = p1.y;
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    float derror = std::abs(dy/float(dx)); 
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
    
    return (B.x - A.x)*(C.y - A.y) - (C.x - A.x)*(B.y - A.y);
    // This gives us the signed area of the parallellogram. (2x tri area.)
}

// Main rasterize function
void triangle(RenderContext* ctx, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv, uint32_t color) {

    ScreenBuffer *buffer_rgba = &ctx->buffers[0];
    ScreenBuffer *buffer_z = &ctx->buffers[1];
    
    Vec2i sc[3]; //screen coords
    Vec3f wc[3] = {v0, v1, v2}; //world coords
    
    //TODO Move the following out to vertex shader.
    //Matrix ViewPort = viewport(buffer_rgba->width/8, buffer_rgba->height/8, buffer_rgba->width*3/4, buffer_rgba->height*3/4);
    //Matrix ViewPort = viewport(buffer_rgba->width/8, buffer_rgba->height/8, buffer_rgba->width*3/4, buffer_rgba->height*3/4);
    Matrix ViewPort = Matrix::identity(4);
    ViewPort[0][0] = buffer_rgba->width/5;
    ViewPort[1][1] = buffer_rgba->height/5;
    ViewPort[2][2] = 1.f; // How to handle depth?

    //translation
    ViewPort[0][3] = buffer_rgba->width/2;
    ViewPort[1][3] = buffer_rgba->height/2;
    ViewPort[2][3] = 1.f;
    //ViewPort[0][3] = x+w/2.f;

    // For subpixel accuracy
    static const int sub_factor = 16;
    static const int sub_mask = sub_factor - 1;

    //set screen_cords
    for(int j=0; j < 3; j++) {
        Matrix mtmp = ctx->projection*ctx->modelview*v2m(wc[j]);
        Vec3f wctmp = m2v(mtmp);
        wc[j] = wctmp;

        // We want our screen coordinates to have subpixel accuracy.

        Vec3f sctmp = m2v(ViewPort*mtmp);
        sc[j] = Vec2i(sctmp.x*sub_factor, sctmp.y*sub_factor);
    }

    //triangle normal
    Vec3f n = cross((wc[2]-wc[0]),(wc[1]-wc[0]));
    n.normalize();
    

    //Find bounding box to loop over.
    int ymin = sc[0].y;
    if (ymin > sc[1].y) ymin = sc[1].y;
    if (ymin > sc[2].y) ymin = sc[2].y;

    int ymax = sc[2].y;
    if (sc[0].y > ymax) ymax = sc[0].y;
    if (sc[1].y > ymax) ymax = sc[1].y;
    
    int xmin = sc[0].x; 
    if (xmin > sc[1].x) xmin = sc[1].x;
    if (xmin > sc[2].x) xmin = sc[2].x;
    
    int xmax = sc[2].x;
    if (sc[0].x > xmax) xmax = sc[0].x;
    if (sc[1].x > xmax) xmax = sc[1].x;

    // For sub-pixel precision. 
    // Round min/max to next integer multiple.
            // Only sample at integer positions, if min is not integer coord. Pixel wont be hit.
    
    // Round start positions to nearest integer
    xmin = (xmin + sub_mask) & ~sub_mask;
    ymin = (ymin + sub_mask) & ~sub_mask;

    int area = barycentric(sc[0], sc[1], sc[2]); //2x tri area

    //Bounding box test
    for (int x=xmin; x<=xmax; x+=sub_factor) {
        for (int y=ymin; y<=ymax; y+=sub_factor) {
            Vec2i p = Vec2i(x, y);
            
            //barycentric (bc_clip?)
            float w0 = barycentric(sc[1],sc[2],p); //signed 2x area
            float w1 = barycentric(sc[2],sc[0],p); //signed 2x area
            float w2 = barycentric(sc[0],sc[1],p); //signed 2x area
            w0 /= area; w1 /= area; w2 /= area;

            // Within tri?
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Render pixel code

                //get z value
                float z = wc[0].z * w0 + wc[1].z * w1 + wc[2].z * w2;
                int zval = (int)(255.f*(z + 1.f)/2.f);
                
                //get texture color Test
                 Vec3f v0col = Vec3f(1.f,0,0);
                 Vec3f v1col = Vec3f(0,1.f,0);
                 Vec3f v2col = Vec3f(0,0,1.f);
                 Vec3f rgb = v0col * w0 + v1col * w1 + v2col * w2;
                 color = ((int)(rgb.x*0xff) << 16) + 
                     ((int)(rgb.y*0xff) << 8) + 
                     ((int)(rgb.z*0xff));
                
                int buf_x = ceil((float)x/sub_factor);
                int buf_y = ceil((float)y/sub_factor);
                
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
    std::vector<int> faces = *obj.faces_verts;
    static std::vector<int> faces_uvs = *obj.faces_uvs;
    std::vector<float> verts = *obj.verts;
    std::vector<float> uvs = *obj.uvs;

    int n_faces = faces.size();
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
        
        //set verts and uvs
        for(int j=0; j < 3; j++) {
            Vec3f v, uv;

            uv.x = uvs[ 3*uv_indecies[j] ];
            uv.y = uvs[ 3*uv_indecies[j] + 1];
            uv.z = uvs[ 3*uv_indecies[j] + 2];
            uv_coords[j] = uv;

            v.x = verts[ 3*vert_indecies[j] ];
            v.y = verts[ 3*vert_indecies[j] + 1];
            v.z = verts[ 3*vert_indecies[j] + 2];
            world_coords[j] = v;
        }

        //determine triangle color
        Vec3f n = cross((world_coords[2]-world_coords[0]),(world_coords[1]-world_coords[0]));
        n.normalize(); 

        Vec3f light_dir = Vec3f(0.f,0.f, -1.f);
        float intensity = dot(n, light_dir);
            triangle(ctx, world_coords[0], world_coords[1], world_coords[2],
                    uv_coords[0], uv_coords[1], uv_coords[2], 0);
    }
}
