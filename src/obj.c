#include "obj.h"

// Local helper functions
static inline int str_starts_with(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

void seek_to(FILE *fp, char* prestr) {
    char str[100];
    int line_len = 0;

    {
        size_t lenpre = strlen(prestr);
        //assert(lenpre <= 100);
    }

    while(1) {
        fgets(str, 100, fp);
        
        if(feof(fp)) {
            printf("EOF");
            break;
        }
        
        // Skip comments
        if( str[0] == '#' ) {
            continue;
        }

        if( str_starts_with(prestr, str) ) {
            line_len = strcspn(str, "\n");
            printf("seeked to %s\n", prestr);
            break;
        }
    }
    fseek(fp , -1*line_len-1 , SEEK_CUR);
}

// Get number elements needed for uvs, verts and faces.
void get_obj_type_count(FILE *fp, int *nverts, int *nuvs, int *nnormals, 
        int *nfaces_verts) {

    // Save current position
    unsigned long position_verts;
    fflush(fp);
    position_verts = ftell(fp);

    char str[100];

    // Vertecies
    seek_to(fp, "v"); fgets(str, 100, fp);
    *nverts = 0; 
    do {
        *nverts += 3; // 3 elements in a vert coord.
        fgets(str, 100, fp);
    } while(str_starts_with("v", str));
    
    // UVS
    seek_to(fp, "vt"); fgets(str, 100, fp);
    *nuvs = 0;
    do {
        *nuvs += 3; // 3 elements in a uv coord.
        fgets(str, 100, fp);
    } while(str_starts_with("vt", str));
    
    // NORMALS
    seek_to(fp, "vn"); fgets(str, 100, fp);
    *nnormals = 0;
    do {
        *nnormals += 3; // 3 elements in a normal vector.
        fgets(str, 100, fp);
    } while(str_starts_with("vn", str));
    
    // Faces
    seek_to(fp, "f"); fgets(str, 100, fp);
    *nfaces_verts = 0;
    do {
        *nfaces_verts += 3;
        fgets(str, 100, fp);
        if(feof(fp)) {
            printf("EOF");
            break;
        }
    } while(str_starts_with("f", str));
    
    // Restore position
    fseek(fp, position_verts, SEEK_SET);

    return;
}

// Main load function
Mesh load_obj(const char * filename) {
    Mesh obj;

    // Open file.
    printf("Opening file %s\n",filename);
    FILE *fp = fopen(filename, "r");
    char str[100];
    int line_len = 0;

    // Get element sizes.
    get_obj_type_count(fp, &obj.nverts, &obj.nuvs, &obj.nnormals,
            &obj.nfaces_verts);
    
    // Allocate memory for data.
    obj.verts           = malloc(obj.nverts*sizeof(float)); 
    obj.uvs             = malloc(obj.nuvs*sizeof(float)); 
    obj.normals         = malloc(obj.nnormals*sizeof(float)); 
    obj.faces_verts     = malloc(obj.nfaces_verts*sizeof(int)); 
    obj.faces_uvs       = malloc(obj.nfaces_verts*sizeof(int)); 
    obj.faces_normals   = malloc(obj.nfaces_verts*sizeof(int)); 
    
    // Save vertex data.
    seek_to(fp, "v");
    {
        int i = 0;
        while (i < obj.nverts) {
            fgets(str, 100, fp);
            //assert(str_starts_with("v", str));

            float vx, vy, vz;
            sscanf(str, "v %f %f %f", &vx, &vy, &vz);
            obj.verts[i++] = vx;
            obj.verts[i++] = vy;
            obj.verts[i++] = vz;
        }
    }
    
    // Save UV data.
    seek_to(fp, "vt");
    {
        int i = 0;
        while (i < obj.nuvs) {
            fgets(str, 100, fp);
            //assert(str_starts_with("vt", str));

            float uvx, uvy, uvz;
            sscanf(str, "vt %f %f %f", &uvx, &uvy, &uvz);
            obj.uvs[i++] = uvx;
            obj.uvs[i++] = uvy;
            obj.uvs[i++] = uvz;
        }
    }
    
    // Save Normal data.
    seek_to(fp, "vn");
    {
        int i = 0;
        while (i < obj.nnormals) {
            fgets(str, 100, fp);
            //assert(str_starts_with("vt", str));

            float nx, ny, nz;
            sscanf(str, "vn %f %f %f", &nx, &ny, &nz);
            obj.normals[i++] = nx;
            obj.normals[i++] = ny;
            obj.normals[i++] = nz;
        }
    }

    // Save faces data.
    seek_to(fp, "f");
    {
        int i = 0;
        while (i < obj.nfaces_verts) { //obj.nfaces_verts, obj.nfaces_uvs are equal.
            fgets(str, 100, fp);
            if( str[0] == '#' ) {
                printf("skipping comment comment to faces\n");
                continue;
            }
            if(feof(fp)) {
                printf("EOF");
                break;
            }

            //assert(str_starts_with("f", str));
            
            int vert_indecies[3];
            int uv_indecies[3];
            int n_indecies[3];
            sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &vert_indecies[0], &uv_indecies[0], &n_indecies[0],
                    &vert_indecies[1], &uv_indecies[1], &n_indecies[1], 
                    &vert_indecies[2], &uv_indecies[2], &n_indecies[2]
                    );
            for(int j=0; j < 3; j++) {
                obj.faces_verts[i] = vert_indecies[j] - 1;
                obj.faces_uvs[i] = uv_indecies[j] - 1;
                obj.faces_normals[i] = n_indecies[j] - 1;
                i++;
            }
        }
    }

    printf("\n\n# of vert elements %d, # of vertids: %d. \n\n\n", obj.nverts, obj.nfaces_verts);

    fclose(fp);

    return obj;
}
