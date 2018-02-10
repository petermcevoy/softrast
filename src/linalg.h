#ifndef LINALG_H_INCLUDED
#define LINALG_H_INCLUDED

#include <math.h>

#ifdef LINALG_SINGLE_PRECISION
typedef float real;
#else /* LINALG_SINGLE_PRECISION */
typedef double real;
#endif /* LINALG_SINGLE_PRECISION */

typedef struct {
    int e[2];
} Vec2i;

typedef struct {
    float e[2];
} Vec2f;

typedef struct {
    float e[3];
} Vec3f;

typedef struct {
    float e[4];
} Vec4f;

typedef struct {
    float e[2*2];
} Mat22f;

typedef struct {
    float e[3*3];
} Mat33f;

typedef struct {
    float e[4*4];
} Mat44f;

static inline int realeq(real a, real b, real eps) {
    return (fabs((double)(a - b)) < (double)eps);       
}

#define VEC_UTILS(fname, Vtype, type, dim) \
    static inline void fname##set(Vtype *target, Vtype src) {               \
        for (int i=0; i<dim; i++) {                                         \
            target->e[i] = src.e[i];                                        \
        }                                                                   \
        return;                                                             \
    }                                                                       \
                                                                            \
    static inline type* fname##el(Vtype *target, unsigned i) {              \
        return &target->e[i];                                               \
    }                                                                       \
                                                                            \
    static inline Vtype fname##add(Vtype a, Vtype b) {                      \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = a.e[i] + b.e[i];                                     \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Vtype fname##sub(Vtype a, Vtype b) {                      \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = a.e[i] - b.e[i];                                     \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Vtype fname##mul(Vtype v, type s) {                       \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = v.e[i]*s;                                            \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Vtype fname##div(Vtype v, type s) {                       \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = v.e[i]/s;                                            \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline type fname##dot(Vtype a, Vtype b) {                       \
        type val;                                                           \
        for (int i=0; i<dim; i++) {                                         \
            val += a.e[i]*b.e[i];                                           \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline type fname##norm(Vtype v) {                               \
        type val = 0;                                                       \
        for (int i=0; i<dim; i++) {                                         \
            val += v.e[i]*v.e[i];                                           \
        }                                                                   \
        return sqrt(val);                                                   \
    }                                                                       \
                                                                            \
    static inline Vtype fname##normalize(Vtype v) {                         \
        return fname##div(v, fname##norm(v));                               \
    }                                                                       \


VEC_UTILS(v2i,Vec2i,int,2)
VEC_UTILS(v2f,Vec2f,float,2)
VEC_UTILS(v3f,Vec3f,float,3)

static inline Vec3f v3fcross(Vec3f a, Vec3f b) {
    Vec3f val;
    val.e[0] = a.e[1]*b.e[2] - a.e[2]*b.e[1];
    val.e[1] = a.e[2]*b.e[0] - a.e[0]*b.e[2];
    val.e[2] = a.e[0]*b.e[1] - a.e[1]*b.e[0];
    return val;
}

#define MAT_UTILS(fname, Mtype, Vtype, type, dim) \
    static inline void fname##set(Mtype *target, Mtype src) {               \
        for (int i=0; i<dim*dim; i++) {                                     \
            target->e[i] = src.e[i];                                       \
        }                                                                   \
        return;                                                             \
    }                                                                       \
                                                                            \
    static inline void fname##setel(Mtype *target,                          \
            int r, int c, type val) {                                       \
            target->e[dim*r + c] = val;                                     \
        return;                                                             \
    }                                                                       \
                                                                            \
    static inline type fname##idx(Mtype m,                                  \
            unsigned i, unsigned j) {                                       \
        return ((type *)&m)[dim*i+j];                                       \
    }                                                                       \
                                                                            \
    static inline Vtype fname##row(Mtype m,                                 \
            int row) {                                                      \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = fname##idx(m, row, i);                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Vtype fname##col(Mtype m,                                 \
            int col) {                                                      \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = fname##idx(m, i, col);                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##ident(void) {                                \
        Mtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            for (int j=0; j<dim; j++) {                                     \
                val.e[dim*i+j] = (i == j) ? 1.f : 0.f;                      \
            }                                                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##add(Mtype a,                                 \
            Mtype b) {                                                      \
        Mtype val;                                                          \
        for (int i=0; i<dim*dim; i++) {                                     \
            val.e[i] = a.e[i] + b.e[i];                                     \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##sub(Mtype a,                                 \
            Mtype b) {                                                      \
        Mtype val;                                                          \
        for (int i=0; i<dim*dim; i++) {                                     \
            val.e[i] = a.e[i] - b.e[i];                                     \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##mul(Mtype m,                                 \
            type s) {                                                       \
        Mtype val;                                                          \
        for (int i=0; i<dim*dim; i++) {                                     \
            val.e[i] = m.e[i] * s;                                          \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##div(Mtype m,                                 \
            type s) {                                                       \
        Mtype val;                                                          \
        for (int i=0; i<dim*dim; i++) {                                     \
            val.e[i] = m.e[i] / s;                                          \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##trans(Mtype m) {                             \
        Mtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            for (int j=0; j<dim; j++) {                                     \
                val.e[dim*i+j] = m.e[dim*j+i];                              \
            }                                                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Vtype fname##v##dim(Mtype m,                              \
            Vtype v) {                                                      \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = 0;                                                   \
            for (int j=0; j<dim; j++) {                                     \
                /* c_i = a_ij*b_j */                                        \
                val.e[i] += m.e[dim*i+j]*v.e[j];                            \
            }                                                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline Mtype fname##fname(Mtype a, Mtype b) {                    \
        Mtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            for (int j=0; j<dim; j++) {                                     \
                val.e[dim*i+j] = 0;                                         \
                /* C_ij = sum_k(A_ik*B_kj) */                               \
                for (int k=0; k<dim; k++) {                                 \
                    val.e[dim*i+j] += a.e[dim*i+k]*b.e[dim*k+j];            \
                }                                                           \
            }                                                               \
        }                                                                   \
        return val;                                                         \
    }                                                                       \

MAT_UTILS(m22f,Mat22f,Vec2f,float,2)
static inline float m22fdet(Mat22f m) {
    return (m.e[2*0 + 0] * m.e[2*1 + 1] - m.e[2*0 + 1] * m.e[2*1 + 0]);
}

MAT_UTILS(m33f,Mat33f,Vec3f,float,3)
static inline float m33fdet(Mat33f m) {
    return (
        m.e[2*0 + 0]*(m.e[2*1 + 1]*m.e[2*2 + 2] - m.e[2*1 + 2]*m.e[2*2 + 1]) +
       -m.e[2*0 + 1]*(m.e[2*1 + 0]*m.e[2*2 + 2] - m.e[2*1 + 2]*m.e[2*2 + 0]) +
        m.e[2*0 + 2]*(m.e[2*1 + 0]*m.e[2*2 + 1] - m.e[2*1 + 1]*m.e[2*2 + 0])
    );
}

MAT_UTILS(m44f,Mat44f,Vec4f,float,4)

#endif /* LINALG_H_INCLUDED */
