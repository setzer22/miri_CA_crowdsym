#pragma once

#include "ObjLoader.h"

namespace renderer30 {

int shader_program;

int build_shader_program(const char* vsPath, const char* fsPath);

GPUModelHandle upload_model_to_gpu(Model* const model) {
    const Model& m = *model;
    int num_elements = 3 * m.faces.size;

    GPUModelHandle handle;
    handle.num_elements = num_elements;
    glGenVertexArrays(1, &(handle.vao_id));
    glGenBuffers(1, &(handle.vertices_buffer_id));
    glGenBuffers(1, &(handle.normals_buffer_id));
    glGenBuffers(1, &(handle.uvs_buffer_id));
    glGenBuffers(1, &(handle.indices_buffer_id));

    // Note: This method is simple but costs more memory. That memory is on the
    // GPU though.
    // TODO: We temporarily use the heap because we're going to throw away this
    // memory at the end.

    Vector3* vertices_per_tri = new Vector3[num_elements];
    Vector3* normals_per_tri = new Vector3[num_elements];
    Vector2* uvs_per_tri = new Vector2[num_elements];
    GLuint* indices_per_tri = new GLuint[num_elements];

    if (m.faces[0].uv1 == -1) {
        error("Attempting to load a model with no uvs, that's not supported.");
        GPUModelHandle m;
        return m;
    }

    for (int i = 0; i < m.faces.size; ++i) {
        const Face& f = m.faces[i];

        vertices_per_tri[3*i] = m.vertices[f.v1];
        vertices_per_tri[3*i + 1] = m.vertices[f.v2];
        vertices_per_tri[3*i + 2] = m.vertices[f.v3];

        normals_per_tri[3*i] = m.normals[f.n1];
        normals_per_tri[3*i + 1] = m.normals[f.n1];
        normals_per_tri[3*i + 2] = m.normals[f.n1];

        uvs_per_tri[3*i] = m.uvs[f.uv1];
        uvs_per_tri[3*i + 1] = m.uvs[f.uv2];
        uvs_per_tri[3*i + 2] = m.uvs[f.uv3];

        indices_per_tri[3*i] = 3*i;
        indices_per_tri[3*i + 1] = 3*i + 1;
        indices_per_tri[3*i + 2] = 3*i + 2;
    }

    glBindVertexArray(handle.vao_id);

    glBindBuffer(GL_ARRAY_BUFFER, handle.vertices_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * num_elements,
                 vertices_per_tri, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, handle.normals_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * num_elements, normals_per_tri,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, handle.uvs_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * num_elements, uvs_per_tri,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.indices_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * num_elements,
                 indices_per_tri, GL_STATIC_DRAW);

    glBindVertexArray(0);

    shader_program = build_shader_program("vs.glsl", "fs.glsl"); 
    if (shader_program == -1) {
        error("Error loading shaders:");
    }

    delete[] vertices_per_tri;
    delete[] normals_per_tri;
    delete[] uvs_per_tri;

    return handle;
}

GLuint CreateShader(GLenum eShaderType, const char* strShaderFile) {
    char shaderSource[4096];
    char inChar;
    FILE* shaderFile;
    int i = 0;

    shaderFile = fopen(strShaderFile, "r");
    while (fscanf(shaderFile, "%c", &inChar) > 0) {
        shaderSource[i++] = inChar;  // loading the file's chars into array
    }
    shaderSource[i - 1] = '\0';
    fclose(shaderFile);
    puts(shaderSource);  // print to make sure the string is loaded

    GLuint shader = glCreateShader(eShaderType);
    const char* ss = shaderSource;
    glShaderSource(shader, 1, &ss, NULL);

    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        char strShaderType[16];
        switch (eShaderType) {
            case GL_VERTEX_SHADER:
                sprintf(strShaderType, "vertex");
                break;
            case GL_GEOMETRY_SHADER:
                sprintf(strShaderType, "geometry");
                break;
            case GL_FRAGMENT_SHADER:
                sprintf(strShaderType, "fragment");
                break;
        }

        printf("Compile failure in %s shader:\n%s\n", strShaderType,
               strInfoLog);
        return -1;
    } else
        puts("Shader compiled sucessfully!");

    return shader;
}
int build_shader_program(const char* vsPath, const char* fsPath) {
    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = CreateShader(GL_VERTEX_SHADER, vsPath);
    fragmentShader = CreateShader(GL_FRAGMENT_SHADER, fsPath);

    /* So we've compiled our shaders, now we need to link them in to the program
    that will be used for rendering. */

    /*This section could be broken out into a separate function, but we're doing
    it here
    because I'm not using C++ STL features that would make this easier. Tutorial
    doing so is
    here: http://www.arcsynthesis.org/gltut/Basics/Tut01%20Making%20Shaders.html
    */

    GLuint tempProgram;
    tempProgram = glCreateProgram();

    glAttachShader(tempProgram, vertexShader);
    glAttachShader(tempProgram, fragmentShader);

    // Per-vertex info
    glBindAttribLocation(tempProgram, 0, "vertex");
    glBindAttribLocation(tempProgram, 1, "normal");
    glBindAttribLocation(tempProgram, 2, "uv");

    glLinkProgram(tempProgram);  // linking!

    // error checking
    GLint status;
    glGetProgramiv(tempProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(tempProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetProgramInfoLog(tempProgram, infoLogLength, NULL, strInfoLog);
        printf("Shader linker failure: %s\n", strInfoLog);
        return -1;
    } else
        puts("Shader linked sucessfully!");

    glDetachShader(tempProgram, vertexShader);
    glDetachShader(tempProgram, fragmentShader);

    return tempProgram;
}

void set_uniform_matrix(int shader_program, const char* name, Mat4 value) {
    int loc = glGetUniformLocation(shader_program, name); //TODO: No need to do every frame
    if(loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)(value.m));
}

void render_model(GPUModelHandle model, Mat4 model_matrix, Mat4 view_matrix, Mat4 projection_matrix) {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shader_program);

    // Uniforms
    //int loc = glGetUniformLocation(shader_program, "mvp"); //TODO: No need to do every frame
    //if(loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)(mvp.m));
    set_uniform_matrix(shader_program, "model_matrix", model_matrix);
    set_uniform_matrix(shader_program, "view_matrix", view_matrix);
    set_uniform_matrix(shader_program, "modelview_matrix", matrix::product(view_matrix, model_matrix));
    set_uniform_matrix(shader_program, "projection_matrix", projection_matrix);
    set_uniform_matrix(shader_program, "mvp", matrix::product(projection_matrix, 
                                              matrix::product(view_matrix, model_matrix)));

    glBindVertexArray(model.vao_id);
    glDrawElements(GL_TRIANGLES, model.num_elements, GL_UNSIGNED_INT,
                   (GLvoid*)0);
    glBindVertexArray(0);
}
}
