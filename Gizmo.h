#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"

namespace gizmo {

  void render(glm::vec3 position, float length) {

    glDisable(GL_LIGHTING);

    //X
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(position.x+0,position.x+0,position.z+0);
    glVertex3f(position.x+length,position.x+0,position.x+0);
    glEnd();
    //Y
    glBegin(GL_LINES);
    glColor3f(0,1,0);
    glVertex3f(position.x+0,position.x+0,position.z+0);
    glVertex3f(position.x+0,position.y+length,position.x+0);
    glEnd();

    //Z
    glBegin(GL_LINES);
    glColor3f(0,0,1);
    glVertex3f(position.x+0,position.x+0,position.z+0);
    glVertex3f(position.x+0,position.z+0,position.z+length);
    glEnd();
  }

}
