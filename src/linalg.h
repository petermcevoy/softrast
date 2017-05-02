#ifndef __LINALG_H__
#define __LINALG_H__
#include <cassert>

template <size_t DIM, typename T> struct Vec;

template <typename T> struct Vec2 {
    Vec2() : x(T()), y(T()) {}
    Vec2(T X, T Y) : x(X), y(Y) {}
    T x,y;
};

template <typename T> struct Vec3 {
    Vec3() : x(T()), y(T()), z(T()) {}
    Vec3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

    Vec3<T> normalize() {
        float val = std::sqrt(x*x + y*y + z*z);

        this->x *= 1.f/val;
        this->y *= 1.f/val;
        this->z *= 1.f/val;
        return *this;
    }
    float norm() {
        return std::sqrt(x*x + y*y + z*z);
    }

    T x,y,z;
};

template <typename T> Vec3<T> operator-(Vec3<T> lhs, const Vec3<T>& rhs) {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    lhs.z -= rhs.z;
    return lhs;
}

template <typename T> Vec3<T> operator+(Vec3<T> lhs, const Vec3<T>& rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    lhs.z += rhs.z;
    return lhs;
}

template <typename T> Vec3<T> cross(Vec3<T> v0, Vec3<T> v1) {
    return Vec3<T>(
            v0.y*v1.z - v0.z*v1.y,
            v0.z*v1.x - v0.x*v1.z,
            v0.x*v1.y - v0.y*v1.x
            );
}

template <typename T> float dot(Vec3<T> v0, Vec3<T> v1) {
    return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;
}

template <typename T> Vec3<T> operator*(Vec3<T> lhs, float factor) {
    lhs.x *= factor; lhs.y *= factor; lhs.z *= factor;
    return lhs;
}


////////

typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;

static float det(Vec3f a, Vec3f b, Vec3f c) {
    return a.x*(b.y-c.y) - a.y*(b.x - c.x) + (b.x*c.y - b.y*c.x);
}

static float det(Vec2i a, Vec2i b, Vec2i c) {
    return a.x*(b.y-c.y) - a.y*(b.x - c.x) + (b.x*c.y - b.y*c.x);
}


//Matrix
const int DEFAULT_ALLOC=4;
struct Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;
    
    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC) : m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), rows(r), cols(c) {}

    static Matrix identity(int dimensions) {
        Matrix E(dimensions, dimensions);
        for (int i=0; i<dimensions; i++) {
            for (int j=0; j<dimensions; j++) {
                E[i][j] = (i==j ? 1.f : 0.f);
            }
        }
        return E;
    
    }

    std::vector<float>& operator[](const int i) {
        assert(i>=0 && i<rows);
        return m[i];
    }

    Matrix operator*(const Matrix& a) {
        assert(cols == a.rows);
        Matrix result(rows, a.cols);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                result.m[i][j] = 0.f;
                for (int k=0; k<cols; k++) {
                    result.m[i][j] += m[i][k]*a.m[k][j];
                }
            }
        }
        return result;
    }

    Matrix transpose() {
        Matrix result(cols,rows);
        for (int i=0; i<rows; i++) {
            for(int j=0; j<cols; j++) {
                result[j][i] = m[i][j];
            }
        }
        return result;
    }

    //Matrix inverse() {
    //    assert(rows==cols);
    //    TODO
    //}
};

#endif // __LINALG_H__
