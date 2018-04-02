#ifndef __PMMOBJ_H__
#define __PMMOBJ_H__
#include "gl.h"
#include <string.h>

// .obj Model loader.
int load_obj(const char * filename, Mesh *obj);
int load_obj_mem(FILE *fp, Mesh *obj);

#endif
