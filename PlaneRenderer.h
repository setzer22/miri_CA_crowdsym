#pragma once

#include "Geometry.h"
#include "Texture.h"
#include <SDL2/SDL_opengl.h>

namespace plane_renderer {

int plane_tex_id;

void init() {
  plane_tex_id = texture::load_texture("./plane_texture.tga");
}

void render_plane(const Plane &p, float box_size) {

  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_COLOR_MATERIAL);

  // set the texture id we stored in the map user data
  glBindTexture(GL_TEXTURE_2D, (GLuint)plane_tex_id);

  // set the texture coordinate buffer

  glm::vec3 o = p.origin;
  glm::vec3 n = glm::normalize(p.normal);
  glm::vec3 up(0, 0, 1);
  glm::vec3 r = glm::cross(n, up);

  float hscale = box_size;
  float vscale = 0.25 * box_size;

  glm::vec3 points[4];
  points[0] = o + r * hscale + up * vscale;
  points[1] = o + r * hscale;
  points[2] = o + -r * hscale;
  points[3] = o + -r * hscale + up * vscale;

  static float tex_coords[4][2] = {
      {8.0f, 2.0f}, {8.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 2.0f}};

  glBegin(GL_QUADS);
  for (int i = 0; i < 4; ++i) {
    glNormal3f(n.x, n.y, n.z);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(tex_coords[i][0], tex_coords[i][1]);
    glVertex3f(points[i].x, points[i].y, points[i].z);
  }
  glEnd();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_COLOR_MATERIAL);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
}

} // namespace plane_renderer
