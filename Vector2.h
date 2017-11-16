#pragma once

struct Vector2 {
    float x;
    float y;
    //Vector2 operator+(Vector2 v2) {Vector2 v3; v3.x = x+v2.x; v3.y = y+v2.y;}
    //Vector2 operator-(Vector2 v2) {Vector2 v3; v3.x = x-v2.x; v3.y = y-v2.y;}
    //Vector2 operator*(int k) {Vector2 v; v.x = k*x; v.y = k*y;}
};

inline Vector2 vec2(float x, float y) {
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}
