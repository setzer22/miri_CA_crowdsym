#pragma once

struct GPUModelHandle {
    GLuint vao_id;
    GLuint vertices_buffer_id;
    GLuint normals_buffer_id;
    GLuint uvs_buffer_id;
    GLuint indices_buffer_id;
    int num_elements;
};

