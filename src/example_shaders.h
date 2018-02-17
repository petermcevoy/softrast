#pragma once
#include "gl.h"

static inline Vec3f reflect(Vec3f incident, Vec3f normal) {
    // incident - 2*dot(normal, incident)*normal
    return v3fsub(incident, v3fmul(normal, 2.f*v3fdot(incident, normal)));
}

//
// Below are example shaders
//

//
// UV vector shader
//

typedef struct {
    ShaderBase base;
    float intensity;
} ShaderNormal;

Vec4f shader_normal_vertex(Vec3f point, int nthvert, void *data) {
    ShaderNormal *sdata = (ShaderNormal *)data;
    Vec4f vertex = {{point.e[0], point.e[1], point.e[2], 1.f}};
    vertex = m44fv4(sdata->base.mvp, vertex);
    return vertex;
}

int shader_normal_fragment(Vec3f bar, Vec3f *color, void *data) {
    ShaderNormal *sdata = (ShaderNormal *)data;
    Vec3f v0col = {{1.f,0,0}};
    Vec3f v1col = {{0,1.f,0}};
    Vec3f v2col = {{0,0,1.f}};

    Vec3f rgb = m33fv3(sdata->base.varying_vertex_normal, bar);
    rgb = v3fmul(rgb, 0.5f);
    Vec3f tmp = {{0.5f, 0.5f, 0.5f}};
    rgb = v3fadd(rgb, tmp);

    v3fset(color, rgb);
    return 0;
}

//
// Phong shader
//

typedef struct {
    ShaderBase base;
    Vec3f ambient_light;
    Vec3f light;
    Vec3f light_pos;
    float diffuse_amount;
    float specular_amount;
    float specular_falloff;
    int count;
} ShaderPhong;

Vec4f shader_phong_vertex(Vec3f point, int nthvert, void *data) {
    ShaderPhong *sdata = (ShaderPhong *)data;
    Vec4f vertex = {{point.e[0], point.e[1], point.e[2], 1.f}};
    vertex = m44fv4(sdata->base.mvp, vertex);
    return vertex;
}

int shader_phong_fragment(Vec3f bar, Vec3f *color, void *data) {
    ShaderPhong *sdata = (ShaderPhong *)data;
    Vec3f normal = m33fv3(sdata->base.varying_vertex_normal, bar);
    Vec3f pos = m33fv3(sdata->base.varying_vertex_post, bar);

    Vec3f L = v3fnormalize(v3fsub(sdata->light_pos,pos));
    Vec3f E = v3fnormalize(v3fmul(pos, 1.f)); // we are in Eye Coordinates, so EyePos is (0,0,0)  
    Vec3f R = v3fnormalize(v3fmul(reflect(L,normal), -1.f)); 

    Vec3f white = {{.5f, .5f, .5f}};
    Vec3f ambient  = sdata->ambient_light;
    Vec3f diffuse  = v3fmul(sdata->light,
            clamp(sdata->diffuse_amount*v3fdot(normal, L), 0.f, 1.f));
    Vec3f specular = v3fmul(f2v3f(sdata->specular_amount),
            pow(clamp(v3fdot(R, E), 0.f, 1.f),sdata->specular_falloff));
    
    Vec3f rgb = ambient;
    rgb = v3fadd(rgb, diffuse);
    rgb = v3fadd(rgb, specular);
        
    v3fset(color, rgb);
    return 0;
}
