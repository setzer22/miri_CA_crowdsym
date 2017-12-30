#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <fstream>

#include "Array.h"

namespace grid_renderer {

  enum GridCell {
    EMPTY,
    OBSTACLE,
    HIGHLIGHTED,
    HIGHLIGHTED2,
    SELECTED
  };

  typedef Array<Array<grid_renderer::GridCell> > Grid;

  float grid_cell_to_color[5][3] =
    {{0.2f, 0.4f, 0.2f}, //EMPTY
     {0.7f, 0.1f, 0.1f}, //OBSTACLE
     {0.1f, 0.1f, 0.7f}, //HIGHLIGHTED
     {0.7f, 0.1f, 0.7f}, //HIGHLIGHTED2
     {0.1f, 0.7f, 0.7f}}; //SELECTED

  Grid read_from_file(const char* path) {
    std::string line;
    std::ifstream file(path);

    std::getline(file, line);
    int grid_size = line.length();

    Array<Array<grid_renderer::GridCell> > grid = alloc_array<Array<grid_renderer::GridCell> >(20);
    for (int i = 0; i < grid_size; ++i) {
      grid[i] = alloc_array<grid_renderer::GridCell>(20);
    }

    int i = 0;
    do {
      for (int j = 0; j < line.length(); ++j) {
        if (line[j] == '#') {
          grid[i][j] = OBSTACLE;
        } else if (line[j] == '.') {
          grid[i][j] = EMPTY;
        }
      }
      i++;
    } while (getline(file, line));

    return grid;
  }

  void render(float box_size, glm::vec2 origin, Grid grid) {
    // Render Grid

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    float cell_size = box_size / (float)grid.size;
    float obstacle_height = cell_size * 2.0f;
    float line_elevation = 0.1f;

    // Draw Lines
    for (int i = 0; i < grid.size; ++i) {
      glColor3f(0.3, 0.6, 0.3);
      glBegin(GL_LINES);
      glVertex3f(origin.x,origin.y+i*cell_size,line_elevation);
      glVertex3f(origin.x+box_size,origin.y+i*cell_size,line_elevation);
      glEnd();
    }
    for (int i = 0; i < grid.size; ++i) {
      glColor3f(0.3, 0.6, 0.3);
      glBegin(GL_LINES);
      glVertex3f(origin.x+i*cell_size,origin.y,line_elevation);
      glVertex3f(origin.x+i*cell_size,origin.y+box_size,line_elevation);
      glEnd();
    }

    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

    // Draw Squares
    for (int i = 0; i < grid.size; ++i) {
      for (int j = 0; j < grid.size; ++j) {
        float* color = grid_cell_to_color[grid[i][j]];
        glColor3f(color[0], color[1], color[2]);
        glm::vec2 point = origin + cell_size*glm::vec2(j, i);

        if (grid[i][j] == OBSTACLE) {
          glBegin(GL_QUADS);

          float color[] = {1.0, 0.0, 0.0};
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

          //Ceiling
          glVertex3f(point.x, point.y, obstacle_height);
          glVertex3f(point.x+cell_size, point.y, obstacle_height);
          glVertex3f(point.x+cell_size, point.y+cell_size, obstacle_height);
          glVertex3f(point.x, point.y+cell_size, obstacle_height);
          //North Wall
          glVertex3f(point.x, point.y, 0);
          glVertex3f(point.x, point.y, obstacle_height);
          glVertex3f(point.x+cell_size, point.y, obstacle_height);
          glVertex3f(point.x+cell_size, point.y, 0);
          //South Wall
          glVertex3f(point.x, point.y+cell_size, 0);
          glVertex3f(point.x, point.y+cell_size, obstacle_height);
          glVertex3f(point.x+cell_size, point.y+cell_size, obstacle_height);
          glVertex3f(point.x+cell_size, point.y+cell_size, 0);
          //East Wall
          glVertex3f(point.x, point.y, 0);
          glVertex3f(point.x, point.y, obstacle_height);
          glVertex3f(point.x, point.y+cell_size, obstacle_height);
          glVertex3f(point.x, point.y+cell_size, 0);
          //West Wall
          glVertex3f(point.x+cell_size, point.y, 0);
          glVertex3f(point.x+cell_size, point.y, obstacle_height);
          glVertex3f(point.x+cell_size, point.y+cell_size, obstacle_height);
          glVertex3f(point.x+cell_size, point.y+cell_size, 0);
          glEnd();
        } else {
          glBegin(GL_QUADS);
          glVertex3f(point.x, point.y, 0);
          glVertex3f(point.x+cell_size, point.y, 0);
          glVertex3f(point.x+cell_size, point.y+cell_size, 0);
          glVertex3f(point.x, point.y+cell_size, 0);
          glEnd();
        }
      }
    }
  }
}
