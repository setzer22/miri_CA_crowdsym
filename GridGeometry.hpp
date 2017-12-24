#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <fstream>

#include "Array.h"
#include "Geometry.h"
#include "gridRenderer.hpp"

#include <vector>
#include<set>

namespace grid_geometry {

  typedef std::pair<int,int> GridPoint;
  typedef std::vector<std::vector<GridPoint > > ObstacleList;

  template <typename T>
  bool contains (std::set<T> set, T element) {
    return set.find(element) != set.end();
  }

  void get_connected_obstacles(const grid_renderer::Grid& grid,
                               std::vector<GridPoint>& obstacles,
                               std::pair<int, int> current,
                               int direction,
                               std::set<std::pair<int,int> >& visited) {
    static int neighbours[2][2][2] = {{{0,1}, {0,-1}}, {{-1,0}, {1,0}}};

    obstacles.push_back(current);

    for (int dir = 0; dir <= 1; ++dir) {
      if (direction == -1 or dir == direction) {
        for (int i = 0; i < 2; ++i) {
          GridPoint n = GridPoint(current.first + neighbours[dir][i][0],
                                  current.second + neighbours[dir][i][1]);
          if (!contains(visited, n) and grid[n.first][n.second] == grid_renderer::OBSTACLE) {
            visited.insert(n);
            get_connected_obstacles(grid, obstacles, n, dir, visited);
          }
        }
      }
    }
  }


  struct BoundingBox {
    glm::vec2 min, max;
  };

  BoundingBox gridPoint_bb (GridPoint p) {
    //TODO: Hardcoded value should come from outside
    float cell_size = 50.0 / 20.0;
    glm::vec2 origin(-25, -25);

    glm::vec2 box_origin = cell_size * glm::vec2(p.second, p.first);

    BoundingBox b;
    b.min = origin + box_origin;
    b.max = origin + box_origin + glm::vec2(cell_size, cell_size);
    return b;
  }

  template <typename T> T min (T x, T y) {
    return x < y ? x : y;
  }
  template <typename T> T max (T x, T y) {
    return x > y ? x : y;
  }


  BoundingBox compute_bounding_box(std::vector<GridPoint> obstacle_group) {
    BoundingBox bb = gridPoint_bb(obstacle_group[0]);

    for (int i = 1; i < obstacle_group.size(); ++i) {
      BoundingBox o_bb = gridPoint_bb(obstacle_group[i]);
      bb.min.x = min(bb.min.x, o_bb.min.x);
      bb.min.y = min(bb.min.y, o_bb.min.y);
      bb.max.x = max(bb.max.x, o_bb.max.x);
      bb.max.y = max(bb.max.y, o_bb.max.y);
    }
    return bb;
  }

  std::vector<Triangle> compute_bb_triangles(BoundingBox bb) {
    const float height = 5.0f; //TODO: Hardcoded constant
    //WIL TODO
    std::vector<Triangle> triangles;

    // Given a bounding box, compute the 8 triangles for its side walls
    glm::vec3 v1, v2, v3;

    // =========== Front Wall ==================
    {
      v1 = glm::vec3(bb.min.x, bb.min.y, 0);
      v2 = glm::vec3(bb.max.x, bb.min.y, 0);
      v3 = glm::vec3(bb.max.x, bb.min.y, height);
      //printf("v1(%f,%f,%f)\n", v1.x, v1.y, v1.z);
      //printf("v2(%f,%f,%f)\n", v2.x, v2.y, v2.z);
      //printf("v3(%f,%f,%f)\n", v3.x, v3.y, v3.z);
      triangles.push_back(Triangle(v1, v2, v3));

      v1 = glm::vec3(bb.min.x, bb.min.y, 0);
      v2 = glm::vec3(bb.max.x, bb.min.y, height);
      v3 = glm::vec3(bb.min.x, bb.min.y, height);
      triangles.push_back(Triangle(v1, v2, v3));
    }
    // =========== Back Wall ==================
    {
      v1 = glm::vec3(bb.min.x, bb.max.y, 0);
      v2 = glm::vec3(bb.max.x, bb.max.y, 0);
      v3 = glm::vec3(bb.max.x, bb.max.y, height);
      triangles.push_back(Triangle(v1, v2, v3));

      v1 = glm::vec3(bb.min.x, bb.max.y, 0);
      v2 = glm::vec3(bb.max.x, bb.max.y, height);
      v3 = glm::vec3(bb.min.x, bb.max.y, height);
      triangles.push_back(Triangle(v1, v2, v3));
    }
    // =========== Left Wall ==================
    {
      v1 = glm::vec3(bb.min.x, bb.min.y, 0);
      v2 = glm::vec3(bb.min.x, bb.max.y, 0);
      v3 = glm::vec3(bb.min.x, bb.max.y, height);
      triangles.push_back(Triangle(v1, v2, v3));

      v1 = glm::vec3(bb.min.x, bb.min.y, 0);
      v2 = glm::vec3(bb.min.x, bb.max.y, height);
      v3 = glm::vec3(bb.min.x, bb.min.y, height);
      triangles.push_back(Triangle(v1, v2, v3));
    }
    // =========== Right Wall ==================
    {
      v1 = glm::vec3(bb.max.x, bb.min.y, 0);
      v2 = glm::vec3(bb.max.x, bb.max.y, 0);
      v3 = glm::vec3(bb.max.x, bb.max.y, height);
      triangles.push_back(Triangle(v1, v2, v3));

      v1 = glm::vec3(bb.max.x, bb.min.y, 0);
      v2 = glm::vec3(bb.max.x, bb.max.y, height);
      v3 = glm::vec3(bb.max.x, bb.min.y, height);
      triangles.push_back(Triangle(v1, v2, v3));
    }

    assert(triangles.size() == 8);

    return triangles;
  }

  Array<Triangle> make_planes(grid_renderer::Grid grid) {
    ObstacleList obstacles;
    std::set<GridPoint> visited;

    // Get list of obstacles connected components
    for(int i = 0; i < grid.size; ++i) {
      for(int j = 0; j < grid.size; ++j) {

        GridPoint p(i,j);

        if (!contains(visited, p) and grid[p.first][p.second] == grid_renderer::OBSTACLE) {
          std::vector<GridPoint> connected;
          get_connected_obstacles(grid, connected, p, -1, visited);
          obstacles.push_back(connected);
        }
      }
    }


    //DEBUG:
    //for (int i = 0; i < obstacles.size(); ++i) {
    //std::cout << "Obstacle Group " << i;
    //for (int j = 0; j < obstacles[i].size(); ++j) {
    //std::cout << "(" << obstacles[i][j].first << ", " << obstacles[i][j].second << ") ";
    //}
    //std::cout << std::endl;
    //}

    // Get bounding box of obstacle CCs
    std::vector<BoundingBox> bbs;
    for (int i = 0; i < obstacles.size(); ++i) {
      bbs.push_back(compute_bounding_box(obstacles[i]));
    }

    //DEBUG:
    //for (int i = 0; i < bbs.size(); ++i) {
    //std::cout << "Bounding Box " << i << " min(" << bbs[i].min.x << ", " << bbs[i].min.y << "), max(" << bbs[i].max.x << ", " << bbs[i].max.y << ")" << std::endl;
    //}

    // Create 8 triangles for each of the bounding boxes
    Array<Triangle> triangles = alloc_array<Triangle>(bbs.size()*8);
    for (int i = 0; i < bbs.size(); ++i) {
      auto tris = compute_bb_triangles(bbs[i]);
      for (int j = 0; j < tris.size(); ++j) {
        triangles[i*8 + j] = tris[j];
      }
    }

    return triangles;
  }

}
