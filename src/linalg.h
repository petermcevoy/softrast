#pragma once
#include <math.h>

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
static float q_rsqrt( float number ) {
	union {
		float f;
		long i;
	} conv;
	
	float x2;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	conv.f  = number;
	conv.i  = 0x5f3759df - ( conv.i >> 1 );
	conv.f  = conv.f * ( threehalfs - ( x2 * conv.f * conv.f ) );
	return conv.f;
}
static inline float i_rsqrt( float number ) {
	return 1.f/sqrt(number);
}

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
    int e[4];
} Vec4i;

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
    static inline Vtype fname##emul(Vtype v1, Vtype v2) {                   \
        Vtype val;                                                          \
        for (int i=0; i<dim; i++) {                                         \
            val.e[i] = v1.e[i]*v2.e[i];                                     \
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
        type val = 0.f;                                                     \
        for (int i=0; i<dim; i++) {                                         \
            val += a.e[i]*b.e[i];                                           \
        }                                                                   \
        return val;                                                         \
    }                                                                       \
                                                                            \
    static inline type fname##norm(Vtype v) {                               \
        type val = 0.f;                                                     \
        for (int i=0; i<dim; i++) {                                         \
            val += v.e[i]*v.e[i];                                           \
        }                                                                   \
        return sqrt(val);                                                   \
    }                                                                       \
                                                                            \
    static inline Vtype fname##normalize(Vtype v) {                         \
        /* Use fast inverese square root q_rsqrt. */                        \
        /* return fname##mul(v, q_rsqrt(fname##dot(v,v)));    */                 \
        return fname##mul(v, i_rsqrt(fname##dot(v,v)));                 \
    }                                                                       \


VEC_UTILS(v2i,Vec2i,int,2)
VEC_UTILS(v2f,Vec2f,float,2)
VEC_UTILS(v3f,Vec3f,float,3)
VEC_UTILS(v4i,Vec4i,int,4)
VEC_UTILS(v4f,Vec4f,float,4)

static inline Vec3f v4f2v3f(Vec4f a) {
    Vec3f val = {{a.e[0], a.e[1], a.e[2]}};
    return val;
}
static inline Vec3f f2v3f(float a) {
    Vec3f val = {{a, a, a}};
    return val;
}

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
            target->e[i] = src.e[i];                                        \
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
    static inline void fname##setcol(Mtype *target, int c, Vtype val) {     \
            for (int i=0; i<dim; i++) {                                     \
                target->e[dim*i + c] = val.e[i];                            \
            }                                                               \
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

// TODO LU decomposition
static inline Mat44f m44finv(Mat44f m) {
    Mat44f inv;

    inv.e[0] = m.e[5]  * m.e[10] * m.e[15] - 
        m.e[5]  * m.e[11] * m.e[14] - 
        m.e[9]  * m.e[6]  * m.e[15] + 
        m.e[9]  * m.e[7]  * m.e[14] +
        m.e[13] * m.e[6]  * m.e[11] - 
        m.e[13] * m.e[7]  * m.e[10];

    inv.e[4] = -m.e[4]  * m.e[10] * m.e[15] + 
        m.e[4]  * m.e[11] * m.e[14] + 
        m.e[8]  * m.e[6]  * m.e[15] - 
        m.e[8]  * m.e[7]  * m.e[14] - 
        m.e[12] * m.e[6]  * m.e[11] + 
        m.e[12] * m.e[7]  * m.e[10];

    inv.e[8] = m.e[4]  * m.e[9] * m.e[15] - 
        m.e[4]  * m.e[11] * m.e[13] - 
        m.e[8]  * m.e[5] * m.e[15] + 
        m.e[8]  * m.e[7] * m.e[13] + 
        m.e[12] * m.e[5] * m.e[11] - 
        m.e[12] * m.e[7] * m.e[9];

    inv.e[12] = -m.e[4]  * m.e[9] * m.e[14] + 
        m.e[4]  * m.e[10] * m.e[13] +
        m.e[8]  * m.e[5] * m.e[14] - 
        m.e[8]  * m.e[6] * m.e[13] - 
        m.e[12] * m.e[5] * m.e[10] + 
        m.e[12] * m.e[6] * m.e[9];

    inv.e[1] = -m.e[1]  * m.e[10] * m.e[15] + 
        m.e[1]  * m.e[11] * m.e[14] + 
        m.e[9]  * m.e[2] * m.e[15] - 
        m.e[9]  * m.e[3] * m.e[14] - 
        m.e[13] * m.e[2] * m.e[11] + 
        m.e[13] * m.e[3] * m.e[10];

    inv.e[5] = m.e[0]  * m.e[10] * m.e[15] - 
        m.e[0]  * m.e[11] * m.e[14] - 
        m.e[8]  * m.e[2] * m.e[15] + 
        m.e[8]  * m.e[3] * m.e[14] + 
        m.e[12] * m.e[2] * m.e[11] - 
        m.e[12] * m.e[3] * m.e[10];

    inv.e[9] = -m.e[0]  * m.e[9] * m.e[15] + 
        m.e[0]  * m.e[11] * m.e[13] + 
        m.e[8]  * m.e[1] * m.e[15] - 
        m.e[8]  * m.e[3] * m.e[13] - 
        m.e[12] * m.e[1] * m.e[11] + 
        m.e[12] * m.e[3] * m.e[9];

    inv.e[13] = m.e[0]  * m.e[9] * m.e[14] - 
        m.e[0]  * m.e[10] * m.e[13] - 
        m.e[8]  * m.e[1] * m.e[14] + 
        m.e[8]  * m.e[2] * m.e[13] + 
        m.e[12] * m.e[1] * m.e[10] - 
        m.e[12] * m.e[2] * m.e[9];

    inv.e[2] = m.e[1]  * m.e[6] * m.e[15] - 
        m.e[1]  * m.e[7] * m.e[14] - 
        m.e[5]  * m.e[2] * m.e[15] + 
        m.e[5]  * m.e[3] * m.e[14] + 
        m.e[13] * m.e[2] * m.e[7] - 
        m.e[13] * m.e[3] * m.e[6];

    inv.e[6] = -m.e[0]  * m.e[6] * m.e[15] + 
        m.e[0]  * m.e[7] * m.e[14] + 
        m.e[4]  * m.e[2] * m.e[15] - 
        m.e[4]  * m.e[3] * m.e[14] - 
        m.e[12] * m.e[2] * m.e[7] + 
        m.e[12] * m.e[3] * m.e[6];

    inv.e[10] = m.e[0]  * m.e[5] * m.e[15] - 
        m.e[0]  * m.e[7] * m.e[13] - 
        m.e[4]  * m.e[1] * m.e[15] + 
        m.e[4]  * m.e[3] * m.e[13] + 
        m.e[12] * m.e[1] * m.e[7] - 
        m.e[12] * m.e[3] * m.e[5];

    inv.e[14] = -m.e[0]  * m.e[5] * m.e[14] + 
        m.e[0]  * m.e[6] * m.e[13] + 
        m.e[4]  * m.e[1] * m.e[14] - 
        m.e[4]  * m.e[2] * m.e[13] - 
        m.e[12] * m.e[1] * m.e[6] + 
        m.e[12] * m.e[2] * m.e[5];

    inv.e[3] = -m.e[1] * m.e[6] * m.e[11] + 
        m.e[1] * m.e[7] * m.e[10] + 
        m.e[5] * m.e[2] * m.e[11] - 
        m.e[5] * m.e[3] * m.e[10] - 
        m.e[9] * m.e[2] * m.e[7] + 
        m.e[9] * m.e[3] * m.e[6];

    inv.e[7] = m.e[0] * m.e[6] * m.e[11] - 
        m.e[0] * m.e[7] * m.e[10] - 
        m.e[4] * m.e[2] * m.e[11] + 
        m.e[4] * m.e[3] * m.e[10] + 
        m.e[8] * m.e[2] * m.e[7] - 
        m.e[8] * m.e[3] * m.e[6];

    inv.e[11] = -m.e[0] * m.e[5] * m.e[11] + 
        m.e[0] * m.e[7] * m.e[9] + 
        m.e[4] * m.e[1] * m.e[11] - 
        m.e[4] * m.e[3] * m.e[9] - 
        m.e[8] * m.e[1] * m.e[7] + 
        m.e[8] * m.e[3] * m.e[5];

    inv.e[15] = m.e[0] * m.e[5] * m.e[10] - 
        m.e[0] * m.e[6] * m.e[9] - 
        m.e[4] * m.e[1] * m.e[10] + 
        m.e[4] * m.e[2] * m.e[9] + 
        m.e[8] * m.e[1] * m.e[6] - 
        m.e[8] * m.e[2] * m.e[5];

    float det = m.e[0] * inv.e[0] + m.e[1] * inv.e[4] + 
                m.e[2] * inv.e[8] + m.e[3] * inv.e[12];

    if (det == 0)
        printf("WARNING: det = 0");


    return m44fmul(inv, 1/det);
}
