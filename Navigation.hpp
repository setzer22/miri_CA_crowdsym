#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <fstream>

#include <vector>

#include "Array.h"
#include "Particle.h"

namespace navigation {

  glm::vec2 grid_to_space(int i, int j) {
    //TODO: Hardcoded value should come from outside
    float cell_size = 50.0 / 20.0;
    glm::vec2 origin(-25, -25);

    return origin + cell_size * glm::vec2(j, i) + glm::vec2(cell_size/2, cell_size/2);
  }

  struct Waypoint {
    int i, j;
    glm::vec2 center;

    Waypoint(int i, int j) {
      this->i = i;
      this->j = i;
      this->center = grid_to_space(i,j);
    }
  };

  struct Navigator {
    std::vector<Waypoint> waypoints;
    int current_waypoint;
    Waypoint top() {return waypoints[current_waypoint];}
    bool empty() {return current_waypoint >= waypoints.size();}
    void pop() {++current_waypoint;}
  };

  const float epsilon = 0.5f;

  void update (Navigator& nav, Array<Particle>& particles, int p_idx) {
    if(nav.empty()) {return;}

    Particle p = particles[p_idx];
    glm::vec3 center(nav.top().center.x, nav.top().center.y, p.getCurrentPosition().z);
    float dist = glm::distance(center, p.getCurrentPosition());

    if(dist <= epsilon) {nav.pop();}
    if(nav.empty()) {
      particles[p_idx].setVelocity(glm::vec3(0,0,0));
      return;
    }

    float max_speed = 10.0f;
    glm::vec3 desired_velocity = glm::normalize(center - p.getCurrentPosition()) * max_speed;
    glm::vec3 steer = desired_velocity - p.getVelocity();

    float avoid_radius = 10.0f;
    glm::vec3 avoid(0,0,0);
    for (int i = 0; i < particles.size; ++i) {
      if (i == p_idx) continue;

      Particle other = particles[i];
      glm::vec3 right = glm::cross(glm::normalize(p.getCurrentPosition()),
                                   glm::vec3(0,0,1));
      float d = glm::distance(p.getCurrentPosition(), other.getCurrentPosition());
      if (d <= avoid_radius) {
        glm::vec3 away = (p.getCurrentPosition() - other.getCurrentPosition());
        float max_repulsion = 100000.0;
        float offset = (1.0/max_repulsion) - 2.20f;
        float curve = 1.5f/(d + offset);
        printf("curve:%f\n",curve);
        printf("d:%f\n",d);
        avoid += away * curve + right * curve * 0.1f;
      }
    }

    //printf("dist: %f\n", dist);
    //printf("sf: %f, %f, %f\n", seek_force.x, seek_force.y, seek_force.z);
    //printf("nav_i: %d\n", nav.current_waypoint);

    particles[p_idx].setVelocity(p.getVelocity() + steer * 0.10f + avoid * 0.1f);
  }
}
