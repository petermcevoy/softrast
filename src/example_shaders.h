#pragma once
#include "gl.h"

//
// UV vector shader.
//

typedef struct {
    ShaderBase base;
    Vec3f light_dir;
    float intensity;
} ShaderUV;

Vec4f shader_uv_vertex(Vec3f point, int nthvert, void *data) {
    ShaderUV *sdata = (ShaderUV *)data;
    Vec4f vertex = {{point.e[0], point.e[1], point.e[2], 1.f}};
    vertex = m44fv4(sdata->base.mvp, vertex);
    return vertex;
}

int shader_uv_fragment(Vec3f bar, Vec3f *color, void *data) {
    ShaderUV *sdata = (ShaderUV *)data;
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
