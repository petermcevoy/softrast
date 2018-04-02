#include "obj.h"
#include <unistd.h>

// Local helper functions
static inline int str_starts_with(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

int seek_to(FILE *fp, char* prestr) {
    fseek(fp, 0, SEEK_SET); // Start from beginning.
    char buffer[100]; char *str;
    str = buffer;
    int line_len = 0;
    {
        size_t lenpre = strlen(prestr);
        //assert(lenpre <= 100);
    }

    while(1) {
        fgets(str, 100, fp);
        
        if(feof(fp)) {
            return 0;
        }

        // Get first non-white space charachter
        for (int i=0; i < 100; i++) {
            str = &str[i];
            if (str[0] != ' ')
                break; //break out of for loop.
        }
        
        // Skip comments
        if( str[0] == '#' ) {
            continue;
        }

        if( str_starts_with(prestr, str) ) {
            line_len = strcspn(str, "\n");
            break;
        }
    }
    fseek(fp , -1*line_len-1 , SEEK_CUR);
    return 1;
}

int get_next_obj_entry(FILE *fp, const char *entry, char **buffer) {
    if(feof(fp)) {
        return 0;
    }

    int seeking = 1;
    char* str;
    while(seeking) {
        // Get new line
        fgets(*buffer, 100, fp);
        
        // Get first non-white space charachter
        for (int i=0; i < 100; i++) {
            str = &(*buffer[i]);
            if (str[0] != ' ')
                break; //break out of for loop.
        }

        if( str[0] == '#' ) { // Skip comments
            continue;
        }

        if (str_starts_with(entry, str)) {
            break;
        }

        if(feof(fp)) { // Stop if we are at the end.
            break;
            return 0;
        }
    }
    return 1;
}

// Get number elements needed for uvs, verts and faces.
void get_obj_type_count(FILE *fp, int *nverts, int *nuvs, int *nnormals, 
        int *nfaces_verts) {

    // Save current position
    unsigned long position_verts;
    fflush(fp);
    position_verts = ftell(fp);

    char buffer[100];
    char *str = buffer;

    // Vertecies
    *nverts = 0; 
    seek_to(fp, "v "); fgets(str, 100, fp); //go to first, and skip first
    while(get_next_obj_entry(fp, "v ", &str)) {
        *nverts += 3; // 3 elements in a vert coord.
    } 
    
    // UVS
    *nuvs = 0;
    seek_to(fp, "vt "); fgets(str, 100, fp);
    while(get_next_obj_entry(fp, "vt ", &str)) {
        *nuvs += 3; // 3 elements in a uv coord.
    }
    
    // NORMALS
    *nnormals = 0;
    seek_to(fp, "vn "); fgets(str, 100, fp);
    while(get_next_obj_entry(fp, "vn ", &str)) {
        *nnormals += 3; // 3 elements in a normal vector.
    }
    
    // Faces
    *nfaces_verts = 0;
    seek_to(fp, "f "); fgets(str, 100, fp);
    while(get_next_obj_entry(fp, "f ", &str)) {
        *nfaces_verts += 3;
    }
    
    // Restore position
    fseek(fp, position_verts, SEEK_SET);

    return;
}


// Main load function
int load_obj(const char * filename, Mesh *obj) {
    // Open file.
    printf("Opening file %s\n",filename);
    if(access(filename, 0) != 0) {
        printf("File %s, does not exist.\n", filename);
        return 1;
    }
    FILE *fp = fopen(filename, "r");

    return load_obj_mem(fp, obj);
}
int load_obj_mem(FILE *fp, Mesh *obj) {

    char buffer[100];
    char *str = buffer;
    int line_len = 0;

    // Get element sizes.
    get_obj_type_count(fp, &obj->nverts, &obj->nuvs, &obj->nnormals,
            &obj->nfaces_verts);
    
    // Allocate memory for data.
    obj->verts           = malloc(obj->nverts*sizeof(float)); 
    obj->uvs             = malloc(obj->nuvs*sizeof(float)); 
    obj->normals         = malloc(obj->nnormals*sizeof(float)); 
    obj->faces_verts     = malloc(obj->nfaces_verts*sizeof(int)); 
    obj->faces_uvs       = malloc(obj->nfaces_verts*sizeof(int)); 
    obj->faces_normals   = malloc(obj->nfaces_verts*sizeof(int)); 
    
    // Save vertex data.
    seek_to(fp, "v ");
    {
        int i = 0;
        while (i < obj->nverts) {
            get_next_obj_entry(fp, "v ", &str);

            float vx, vy, vz;
            sscanf(str, "v %f %f %f", &vx, &vy, &vz);
            obj->verts[i++] = vx;
            obj->verts[i++] = vy;
            obj->verts[i++] = vz;
        }
    }
    
    // Save UV data.
    seek_to(fp, "vt");
    {
        int i = 0;
        while (i < obj->nuvs) {
            get_next_obj_entry(fp, "vt ", &str);

            float uvx, uvy, uvz;
            sscanf(str, "vt %f %f %f", &uvx, &uvy, &uvz);
            obj->uvs[i++] = uvx;
            obj->uvs[i++] = uvy;
            obj->uvs[i++] = uvz;
        }
    }
    
    // Save Normal data.
    seek_to(fp, "vn");
    {
        int i = 0;
        while (i < obj->nnormals) {
            get_next_obj_entry(fp, "vn ", &str);

            float nx, ny, nz;
            sscanf(str, "vn %f %f %f", &nx, &ny, &nz);
            obj->normals[i++] = nx;
            obj->normals[i++] = ny;
            obj->normals[i++] = nz;
        }
    }

    // Save faces data.
    seek_to(fp, "f ");
    {
        int i = 0;
        while (i < obj->nfaces_verts) { //obj.nfaces_verts, obj.nfaces_uvs are equal.
            get_next_obj_entry(fp, "f ", &str);
            if( str[0] == '#' ) {
                continue;
            }
            if(feof(fp)) {
                break;
            }

            //assert(str_starts_with("f", str));
            
            int vert_indecies[3];
            int uv_indecies[3] = {-1, -1, -1};
            int n_indecies[3] = {-1, -1, -1};
            if ((obj->nuvs > 0) && (obj->nnormals > 0)) {
                sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                        &vert_indecies[0], &uv_indecies[0], &n_indecies[0],
                        &vert_indecies[1], &uv_indecies[1], &n_indecies[1], 
                        &vert_indecies[2], &uv_indecies[2], &n_indecies[2]);
            } else if ((obj->nuvs == 0) && (obj->nnormals > 0)) {
                sscanf(str, "f %d//%d %d//%d %d//%d", 
                        &vert_indecies[0], &n_indecies[0],
                        &vert_indecies[1], &n_indecies[1], 
                        &vert_indecies[2], &n_indecies[2]);
            } else if ((obj->nuvs > 0) && (obj->nnormals == 0)) {
                sscanf(str, "f %d/%d/ %d/%d/ %d/%d/", 
                        &vert_indecies[0], &uv_indecies[0],
                        &vert_indecies[1], &uv_indecies[1], 
                        &vert_indecies[2], &uv_indecies[2]);
            } else {
                sscanf(str, "f %d// %d// %d//", 
                        &vert_indecies[0],
                        &vert_indecies[1], 
                        &vert_indecies[2]);
            }

            for(int j=0; j < 3; j++) {
                obj->faces_verts[i] = vert_indecies[j] - 1;
                obj->faces_uvs[i] = uv_indecies[j] - 1;
                obj->faces_normals[i] = n_indecies[j] - 1;
                i++;
            }
        }
    }

    printf("Number of vert elements %d, Number of vertids: %d. \n\n\n", obj->nverts, obj->nfaces_verts);
    fclose(fp);
    return 0;
}
