#pragma once
#include "Renderer30.h"
#include "Transform.h"

struct Model;
struct GPUModelHandle;

struct State {
    //Model* model;
    GPUModelHandle model_gpu;
    Transform model_transform;

    GPUModelHandle cube_model_gpu;
    Transform cube_transform;
};

