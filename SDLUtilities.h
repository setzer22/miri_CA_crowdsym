#pragma once

#include "GLUtilities.h"

struct SDL_Init_Result {
    SDL_Window* gWindow;
    SDL_GLContext gContext;
    bool success;
};

SDL_Init_Result init_SDL(int screen_width, int screen_height) {
    bool success = true;
    SDL_Window* gWindow = NULL;
    SDL_GLContext gContext;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not init_SDLialize! SDL Error: %s\n", SDL_GetError());
        success = false;
        goto early_exit;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gWindow = SDL_CreateWindow(
        "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screen_width, screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (gWindow == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
        goto early_exit;
    }

    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
               SDL_GetError());
        success = false;
        goto early_exit;
    }

    SDL_GL_MakeCurrent(gWindow, gContext);

    // VSync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    if (!init_GL()) {
        printf("Unable to initialize OpenGL!\n");
        success = false;
        goto early_exit;
    }

early_exit:
    SDL_Init_Result result;
    result.success = success;
    result.gWindow = gWindow;
    result.gContext = gContext;
    return result;
}
