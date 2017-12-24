#pragma once

#include "tga.h"
#include "Utils.h"

#include <SDL2/SDL_opengl.h>

namespace texture {

  GLuint load_texture (char* path) {

    CTga *Tga;
    Tga = new CTga();

    // Note: This will always make a 32-bit texture
    if(Tga->ReadFile(path)==0) {
      Tga->Release();
      std::string buf("Error: File ");
      buf.append(path);
      buf.append(" not found.");
      error(buf);
      return (GLuint)-1;
    }

    //Bind texture
    int width = Tga->GetSizeX();
    int height = Tga->GetSizeY();
    int depth = Tga->Bpp() / 8;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLuint pId;

    glGenTextures(1, &pId);

    glBindTexture(GL_TEXTURE_2D, pId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, ((depth == 3) ? GL_RGB : GL_RGBA), width, height, 0, ((depth == 3) ? GL_RGB : GL_RGBA) , GL_UNSIGNED_BYTE, (char*)Tga->GetPointer() );

    Tga->Release();

    return pId;
  }


}
