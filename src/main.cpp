#include <SDL2/SDL.h>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <vector>
#include "linalg.h"
#include "tgaimage.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

//video buffer
typedef struct {
    void *memory;
    int *zbuffer;
    int width;
    int height;
    int pitch; 
} ScreenBuffer;

//model struct
typedef struct {
    std::vector<int> *faces_verts;  //sets of 3, indecies for vertex positions
    std::vector<int> *faces_uvs;    //sets of 3, indecies for uv coordinates
    std::vector<float> *verts;      //stored as v0.x v0.y v0.z v1.x v.1.y ...
    std::vector<float> *uvs;          //stored as uv0.x uv0.y uv0.z uv1.x ...
} Model;

static ScreenBuffer buffer;
static int running = 1;
static TGAImage *texture;
static Matrix projection = Matrix::identity(4);

void set_color(ScreenBuffer *buffer, int x, int y, uint32_t color) {
    uint32_t *pixels = (uint32_t *)buffer->memory;
    int ny = (SCREEN_HEIGHT-1) - y;
    if (!(0 >= x && x < SCREEN_WIDTH) || !(0 >= y && y < SCREEN_HEIGHT)) {
        //assert(false);
    }
    pixels[ny*buffer->width + x] = color;
}
int set_z(ScreenBuffer *buffer, int x, int y, int z) {
    int *zbuffer = (int *)buffer->zbuffer;
    int ny = (SCREEN_HEIGHT-1) - y;
    if (!(0 >= x && x < SCREEN_WIDTH) || !(0 >= y && y < SCREEN_HEIGHT)) {
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

Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}
Matrix v2m(Vec3f v) {
    Matrix m(4,1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

void triangle(ScreenBuffer *buffer, Vec3f v0, Vec3f v1, Vec3f v2,
        Vec3f v0uv, Vec3f v1uv, Vec3f v2uv, uint32_t color) {
    //TODO change to use the boundingbox/barycentric method
    
    //order points
    if (v0.y > v1.y) {std::swap(v0, v1); std::swap(v0uv, v1uv); }
    if (v0.y > v2.y) {std::swap(v0, v2); std::swap(v0uv, v2uv); }
    if (v1.y > v2.y) {std::swap(v1, v2); std::swap(v1uv, v2uv); }


    Vec2i sc[3]; //screen coords
    Vec3f wc[3] = {v0, v1, v2}; //world coords
    
    //set screen_cords
    for(int j=0; j < 3; j++) {
        Vec3f vtmp = m2v(projection*v2m(wc[j]));
        //sc[j] = Vec2i(
        //        (vtmp.x+1.f)*SCREEN_WIDTH/2.f,
        //        (vtmp.y+1.f)*SCREEN_HEIGHT/2.f);
        sc[j] = Vec2i((wc[j].x+1.f)*SCREEN_WIDTH/2.f,
                (wc[j].y+1.f)*SCREEN_HEIGHT/2.f);
    }
    
    //triangle normal
    Vec3f n = cross((wc[2]-wc[0]),(wc[1]-wc[0]));
    n.normalize(); 

    float line_a_coef = (sc[0].y-sc[1].y) ? (float)(sc[0].x - sc[1].x)/((float)(sc[0].y - sc[1].y)) : 0.f;
    float line_b_coef = (float)(sc[1].x - sc[2].x)/((float)(sc[1].y - sc[2].y));
    float line_c_coef = (float)(sc[2].x - sc[0].x)/((float)(sc[2].y - sc[0].y)); //longest
    
    int sec1_size = std::abs(sc[0].y - sc[1].y);
    int sec2_size = std::abs(sc[1].y - sc[2].y);

    int offset_y = sc[0].y;
    int offset_x = sc[0].x;
    Vec2i start_p, end_p;
    //Loop height of triangle
    for (int tmp_y = 0; tmp_y < (sec1_size + sec2_size); tmp_y++) {
        start_p.y = offset_y+tmp_y;
        end_p.y = offset_y+tmp_y;

        //set end depending on which section we are on
        start_p.x = sc[0].x + (int)(tmp_y*line_c_coef);
        if (tmp_y <= sec1_size) {
            end_p.x = sc[1].x + (int)((tmp_y-sec1_size)*line_a_coef);
        } else {
            end_p.x = sc[2].x + (int)((tmp_y-sec1_size-sec2_size)*line_b_coef);
        }

        if (start_p.x > end_p.x) std::swap(start_p, end_p);
        float yf = (float)start_p.y*2.f/((float)SCREEN_HEIGHT) - 1.f;
        float d = -n.x*v0.x - n.y*v0.y - n.z*v0.z;
        for (int x=start_p.x; x <= end_p.x; x++) {
            float xf = (float)x*2.f/((float)SCREEN_WIDTH) - 1.f;
            float z = -1.f*(n.x*xf + n.y*yf + d)/n.z;
            int zval = (int)(255.f*(z + 1.f)/2.f);
            int ret = set_z(buffer, x, start_p.y, zval);
            if (ret){
                //get texture color

                Vec2i p = Vec2i(x, start_p.y);

                //barycentric //TODO Move up
                float area = det(sc[0], sc[1], sc[2]); //2x tri area
                float w0 = det(sc[1],sc[2],p); //signed 2x area
                float w1 = det(sc[2],sc[0],p); //signed 2x area
                float w2 = det(sc[0],sc[1],p); //signed 2x area
                w0 /= area; w1 /= area; w2 /= area;
                
                
                //Vec3f rgb = v0col * w0 + v1col * w1 + v2col * w2;
                //color = ((int)(rgb.x*0xff) << 16) + 
                //    ((int)(rgb.y*0xff) << 8) + 
                //    ((int)(rgb.z*0xff));
                    

                Vec3f uv = v0uv * w0 + v1uv * w1 + v2uv * w2;
                int tex_x = (int)(uv.x*(float)texture->get_width());
                int tex_y = (int)((1.f-uv.y)*(float)texture->get_height());
                TGAColor tex_color = texture->get(tex_x, tex_y);
                color = (tex_color.bgra[0]) + 
                    (tex_color.bgra[1] << 8) + 
                    (tex_color.bgra[2] << 16);

                set_color(buffer, x, start_p.y, color);
            } 
        }
    }

    //line(buffer, sc[0], sc[1], 0xffffffff);
    //line(buffer, sc[1], sc[2], 0xffffffff);
    //line(buffer, sc[2], sc[0], 0xffffffff);
}

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
    
    verts->push_back(0.f);
    verts->push_back(0.f);
    verts->push_back(0.f);

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
    
    
    uvs->push_back(0.f);
    uvs->push_back(0.f);
    uvs->push_back(0.f);
    
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

        //set faces
        int vert_indecies[3];
        int uv_indecies[3];
        sscanf(str, "f %d/%d/%*d %d/%d/%*d %d/%d/%*d", 
                &vert_indecies[0], &uv_indecies[0],
                &vert_indecies[1], &uv_indecies[1],
                &vert_indecies[2], &uv_indecies[2]);
        for(int i=0; i < 3; i++) {
            faces_verts->push_back(vert_indecies[i]);
            faces_uvs->push_back(uv_indecies[i]);
        }

    }
    
    fclose(fp);
}

void wireframe(ScreenBuffer *buffer, std::vector<float> verts,
        std::vector<int> faces) {

    int n_faces = faces.size();
    for(int i = 0; i < n_faces; i=i+3) {
        int vert_indecies[3];
        vert_indecies[0] = faces[i];
        vert_indecies[1] = faces[i+1];
        vert_indecies[2] = faces[i+2];
        //draw lines
        for(int j=0; j < 3; j++) {
            //printf("vert_indecies[%d]: %d\n",i,vert_indecies[i]);
            float v0x = verts[ 3*vert_indecies[j] ];
            float v0y = verts[ 3*vert_indecies[j] + 1];
            float v1x = verts[ 3*vert_indecies[(j+1)%3] ];
            float v1y = verts[ 3*vert_indecies[(j+1)%3] + 1];
            //printf("i: %d, (i+1)p3 + 1: %d\n", i, (i+1)%3);
            Vec2i p0, p1;
            p0.x = (v0x+1.)*(float)SCREEN_WIDTH/2.;
            p0.y = (v0y+1.)*(float)SCREEN_HEIGHT/2.;
            p1.x = (v1x+1.)*(float)SCREEN_WIDTH/2.;
            p1.y = (v1y+1.)*(float)SCREEN_HEIGHT/2.;
            line(buffer, p0, p1, 0xffffffff);
        }

    }

}

void flat_shading(ScreenBuffer *buffer, Model obj) {
    std::vector<int> faces = *obj.faces_verts;
    std::vector<int> faces_uvs = *obj.faces_uvs;
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
        Vec2i screen_coords[3];

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

            screen_coords[j] = Vec2i((v.x+1.f)*SCREEN_WIDTH/2.f,
                (v.y+1.f)*SCREEN_HEIGHT/2.f);
        }

        //determine triangle color
        Vec3f n = cross((world_coords[2]-world_coords[0]),(world_coords[1]-world_coords[0]));
        n.normalize(); 

        Vec3f light_dir = Vec3f(0.f,0.f, -1.f);
        float intensity = dot(n, light_dir);
        if (intensity > 0) {
            uint32_t color = (uint8_t)(0xff*intensity) + 
                ((uint8_t)(0xff*intensity) << 8) +
                ((uint8_t)(0xff*intensity) << 16) +
                ((uint8_t)(0xff*intensity) << 24);
            triangle(buffer, world_coords[0], world_coords[1], world_coords[2],
                    uv_coords[0], uv_coords[1], uv_coords[2], color);
        }
    }
    
}

/*void rasterize_tri(ScreenBuffer *buffer, int *zbuffer[], Vec2i t0, Vec2i t1,
        Vec2i t2) {
    if (t0.x>t1.x) {
        std::swap(p0, p1);
    } 
    for(int x=p0.x; x<=p1.x; x++) {
        float t=(x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.f-t) + p1.y*t;
        if (ybuffer[x]<y) {
            ybuffer[x] = y;
            set_color(buffer, x, 80, color);
        }
    }
}*/

void render(ScreenBuffer *buffer) {
    //clear to black
    memset(buffer->memory, 0, buffer->height*buffer->width*(sizeof(uint32_t)));
    memset(buffer->zbuffer, 0,
            buffer->height*buffer->width*(sizeof(int)));
    
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
    
    /*Vec3f t3[3] = {
        Vec3f(-0.8f, 0.0f, 0.1f),
        Vec3f(0.8f, 0.0f, 0.1f),
        Vec3f(0.0f, 0.9f, 0.1f)
    }; 
    triangle(buffer, t3[0], t3[1], t3[2], t3[0], t3[1], t3[2], 0xffffffff);*/
    
    //triangle(buffer, t0[0], t0[1], t0[2], red);
    //triangle(buffer, t1[0], t1[1], t1[2], green);
    //triangle(buffer, t2[0], t2[1], t2[2], blue);

    //Load file
    Model obj;
    obj.faces_verts = new std::vector<int>;
    obj.faces_uvs = new std::vector<int>;
    obj.verts = new std::vector<float>;
    obj.uvs = new std::vector<float>;
    load_obj("res/head.obj", obj);

    //Adjust vertex positions.
    //Matrix projection = Matrix::identity(4);
    //Matrix viewport = Matrix::identity(4);
    projection[3][2] = -1.f/15.f;
    
    //Load texture
    texture = new TGAImage();
    texture->read_tga_file("res/head_diffuse.tga");
    
    //rasterize_obj(buffer, zbuffer, verts, faces);
    flat_shading(buffer, obj);

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
    
        //Render video
        render(&buffer);

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

    //Main display
    while(running) {
        
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


        SDL_Delay(100);
    }
    
    free(buffer.memory);

    return 0;
}
