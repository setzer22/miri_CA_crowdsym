#pragma once

#include "math_3d.h"


typedef vec3_t Vector3;


namespace vector {

    static inline Vector3 v(float x, float y, float z) {
        Vector3 v;
        v.x = x; 
        v.y = y; 
        v.z = z; 
        return v;
    }

    static inline Vector3 zero() {
        return v(0,0,0);
    }

    static inline Vector3 one() {
        return v(1,1,1);
    }

}
