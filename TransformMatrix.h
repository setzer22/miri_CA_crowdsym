#pragma once
#include "math_3d.h"
#include "Vector3.h"

typedef mat4_t Mat4;

namespace matrix {

    inline Mat4 identity() {
        return m4_identity();
    }

    inline Mat4 make_mat4(float m00, float m01, float m02, float m03,
                     float m10, float m11, float m12, float m13,
                     float m20, float m21, float m22, float m23,
                     float m30, float m31, float m32, float m33) {
        return mat4(m00, m10, m20, m30, 
                            m01, m11, m21, m31,
                            m02, m12, m22, m32,
                            m03, m13, m23, m33);
    }

    inline Mat4 product(Mat4 A, Mat4 B) {
        return m4_mul(A, B);
    }
    inline Mat4 product(Mat4 A, Mat4 B, Mat4 C) {
        return product(A, product(B, C));
    }
    inline Mat4 product(Mat4 A, Mat4 B, Mat4 C, Mat4 D) {
        return product(A, product(B, product(C, D)));
    }

    inline Mat4 translation (Vector3 offset) {
        return m4_translation(offset);
    }

    inline Mat4 scaling (Vector3 scale) {
        return m4_scaling(scale);
    }

    inline Mat4 look_at (Vector3 from, Vector3 to, Vector3 up) {
        return m4_look_at(from, to, up);
    }

    inline Mat4 perspective (float vertical_fov_in_deg, float aspect_ratio, float znear, float zfar) {
        return m4_perspective(vertical_fov_in_deg, aspect_ratio, znear, zfar);
    }

    inline Mat4 Rx (float angle_in_rad) {
        return m4_rotation_x(angle_in_rad);
    }
    inline Mat4 Ry (float angle_in_rad) {
        return m4_rotation_y(angle_in_rad);
    }
    inline Mat4 Rz (float angle_in_rad) {
        return m4_rotation_z(angle_in_rad);
    }

    void printm (Mat4& m) {
        for (int i = 0; i < 4; ++i) {
            std::cout << m.m[i][0] << " " << m.m[i][1] << " " << m.m[i][2] << " " << m.m[i][3] << "\n";
        }
    }




}
