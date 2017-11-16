#pragma once
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "Vector2.h"
#include "Vector3.h"


struct Face {
    int v1; int v2; int v3;
    int n1; int n2; int n3;
    int uv1; int uv2; int uv3;
};

typedef Vector3 Point;

struct Model {
    Array<Vector3> vertices;
    Array<Vector3> normals;
    Array<Vector2> uvs;
    Array<Face> faces;
};

namespace objloader {

bool starts_with(char*& str, char* keyword) {
    int i = 0;
    int len = strlen(keyword);

    while(str[i] == keyword[i] && i < len) ++i;
    if (i == len) {
        return true;
    } else {
        return false;
    }
}

void consume_keyword(char*& str, char* keyword) {
    if (starts_with(str, keyword)) {
        int len = strlen(keyword);
        str = &str[len];
    } else {
        error("Unexpexted input:");
        error(str);
        exit(0);
    }
}

//@SideEffects
float consume_float(char* & str) {
    float f = strtof(str, &str);
    return f;
}

int consume_int(char* & str) {
    int i = static_cast<int>(strtol(str, &str, 10));
    return i;
}

Vector3 consume_vector3(char*& str) {
    Vector3 v;
    v.x = consume_float(str);
    v.y = consume_float(str);
    v.z = consume_float(str);
    return v;
}

Vector2 consume_vector2(char*& str) {
    Vector2 v;
    v.x = consume_float(str);
    v.y = consume_float(str);
    return v;
}

Face consume_face(char*& str) {
    // NOTE: We subtract 1 from the index because OBJ faces are 1 indexed
    // but our arrays aren't
    Face f;
    f.v1 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.uv1 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.n1 = consume_int(str) - 1;

    f.v2 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.uv2 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.n2 = consume_int(str) - 1;

    f.v3 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.uv3 = consume_int(str) - 1;
    consume_keyword(str, "/");
    f.n3 = consume_int(str) - 1;

    return f;
}


Model* read_model(const char* path) {
    print("start reading file");
    FILE* f = fopen(path, "r");
    int buffer_size = 256;
    char line_buffer[buffer_size];
    
    int num_vertices = 0;
    int num_normals = 0;
    int num_uvs = 0;
    int num_faces = 0;

    while (fgets(line_buffer, buffer_size*sizeof(char), f)) {
        char fst = line_buffer[0];
        char snd = line_buffer[1];

        if(fst == 'v' && snd != 'n' && snd != 't') { // Vertex
            num_vertices += 1;
        } else if (fst == 'v' && snd == 'n') { // Normal
            num_normals += 1;
        } else if (fst == 'v' && snd == 't') { // UV
            num_uvs += 1;
        } else if (fst == 'f') {// Face
            num_faces += 1;
        } else if (fst == '#') {
            // Comment
        } else {
            error("Unhandled case in input line:");
            error(line_buffer);
        }
    }

    Model* m = alloc_one<Model>();
    m->vertices = alloc_array<Vector3>(num_vertices);
    m->normals = alloc_array<Vector3>(num_normals);
    m->uvs = alloc_array<Vector2>(num_uvs);
    m->faces = alloc_array<Face>(num_faces);

    rewind(f);

    int vertex_index = 0;
    int normal_index = 0;
    int uv_index = 0;
    int face_index = 0;

    //@CopyPaste
    while (fgets(line_buffer, buffer_size*sizeof(char), f)) {
        char* line = &line_buffer[0];
        char fst = line_buffer[0];
        char snd = line_buffer[1];

        if(fst == 'v' && snd != 'n' && snd != 't') { // Vertex
            consume_keyword(line, "v");
            Vector3 v = consume_vector3(line);
            m->vertices[vertex_index++] = v;
        } else if (fst == 'v' && snd == 'n') { // Normal
            consume_keyword(line, "vn");
            Vector3 vn = consume_vector3(line);
            m->normals[normal_index++] = vn;
        } else if (fst == 'v' && snd == 't') { // UV
            consume_keyword(line, "vt");
            Vector2 vt = consume_vector2(line);
            m->uvs[uv_index++] = vt;
        } else if (fst == 'f') {// Face
            consume_keyword(line, "f");
            Face face = consume_face(line);
            m->faces[face_index++] = face;
        } else if (fst == '#') {

        } else {
        }
    }

    return m;

}



}
