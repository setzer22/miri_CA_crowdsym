#pragma once 
#include "Vector3.h"
//TODO #include "Quaternion.h"
#include "TransformMatrix.h"


struct Transform {
    Vector3 location;
    Vector3 rotation;
    Vector3 scale;
};

namespace transform {
    Transform make_transform(Vector3 location, Vector3 rotation, Vector3 scale) {
        Transform t;
        t.location = location;
        t.rotation = rotation;
        t.scale = scale;
        return t;
    }

    Mat4 transform_to_matrix (Transform t) {
        Mat4 rot = matrix::product(matrix::Rx(t.rotation.x),
                                   matrix::Ry(t.rotation.y),
                                   matrix::Rz(t.rotation.z));
        Mat4 trans = matrix::translation(t.location);
        Mat4 scale = matrix::scaling(t.scale);
        return matrix::product(trans, rot, scale);
    }
}
