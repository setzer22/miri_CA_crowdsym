#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <algorithm>

#include <vector>
#include <queue>

#include "Array.h"
#include "Particle.h"
#include "gridRenderer.hpp"
#include "Navigation.hpp"

using namespace grid_renderer;
using namespace navigation;

namespace pathfinding {

  GridCell cell (Grid grid, Waypoint w) {
    return grid[w.i][w.j];
  }


  enum NeighborhoodType {
    NEIGH4,
    NEIGH8
  };

  enum HeuristicType {
    MANHATTAN,
    EUCLIDEAN
  };

  float manhattan(Waypoint w1, Waypoint w2) {
    return abs(w1.j - w2.j) + abs(w1.i - w2.i);
  }

  float euclidean(Waypoint w1, Waypoint w2) {
    return sqrt((w1.j - w2.j)*(w1.j - w2.j) + (w1.i - w2.i)*(w1.i - w2.i));
  }

  std::vector<Waypoint> basic_a_star(Grid grid, Waypoint origin, Waypoint destination) {
    const NeighborhoodType neigh_t = NEIGH8;
    const HeuristicType heur_t = EUCLIDEAN;
    const float inf = 1e20;

    Waypoint nil_w(-1, -1); // A special marker for the came_from matrix
    std::vector<std::vector<float> > cost_so_far(grid.size, std::vector<float>(grid.size, inf));
    std::vector<std::vector<Waypoint> > came_from(grid.size, std::vector<Waypoint>(grid.size, nil_w));

    cost_so_far[origin.i][origin.j] = 0;

    // Cost function
    auto cost = [&grid, origin, &cost_so_far, destination](const Waypoint w){
      return cost_so_far[w.i][w.j] + (heur_t == MANHATTAN) ? manhattan(w, destination) : euclidean(w, destination);
    };

    // Comparison function
    auto comp = [cost](const Waypoint w1, const Waypoint w2) {
      //Note: Priority and cost are opposite concepts, hence the >
      return cost(w1) > cost(w2);
    };


    auto neighbours = [&grid](const Waypoint w){
      std::vector<Waypoint> neighs;
      static int neighborhood_8[8][2]{{1,1},{-1,-1},{-1,1},{1,-1},{0,1},{0,-1},{1,0},{-1,0}};
      static int neighborhood_4[4][2]{{0,1},{0,-1},{1,0},{-1,0}};
      if (neigh_t == NEIGH8) {
        for(int n = 0; n < 8; ++n) {
          int ni = neighborhood_8[n][0];
          int nj = neighborhood_8[n][1];
          //printf("Pushing neighbour (%d,%d)\n", ni, nj);
          if(w.i+ni < grid.size and w.i+ni >= 0 and
             w.j+nj < grid.size and w.j+nj >= 0 and
             grid[w.i+ni][w.j+nj] != OBSTACLE) {
            neighs.push_back(Waypoint(w.i+ni, w.j+nj));
          }
        }
      }
      else if (neigh_t == NEIGH4) {
        for(int n = 0; n < 8; ++n) {

          int ni = neighborhood_8[n][0];
          int nj = neighborhood_8[n][1];
          printf("Pushing neighbour (%d,%d)\n", ni, nj);
          if(w.i+ni < grid.size and w.i+ni >= 0 and
             w.j+nj < grid.size and w.j+nj >= 0 and
             grid[w.i+ni][w.j+nj] != OBSTACLE) {
            neighs.push_back(Waypoint(w.i+ni, w.j+nj));
          }
        }
      }
      return neighs;
    };

    std::priority_queue<Waypoint, std::vector<Waypoint>,
                        std::function<bool(const Waypoint, const Waypoint)>>
      frontier(comp);

    frontier.push(origin);

    while(not frontier.empty()) {
      auto current = frontier.top();
      frontier.pop();

      if (current == destination) {
        break;
      }

      for (auto w : neighbours(current)) {
        //printf("Exploring neighbour (%d,%d)\n", w.i, w.j);
        float new_cost = cost_so_far[current.i][current.j]+1;
        if (new_cost < cost_so_far[w.i][w.j]) {
          grid[w.i][w.j] = HIGHLIGHTED;
          cost_so_far[w.i][w.j] = new_cost;
          frontier.push(w);
          came_from[w.i][w.j] = current;
        }
      }
    }

    std::vector<Waypoint> waypoints;
    Waypoint w = destination;
    while (w != origin and w != nil_w) {
      grid[w.i][w.j] = SELECTED;
      waypoints.push_back(w);
      w = came_from[w.i][w.j];
    }
    std::reverse(waypoints.begin(), waypoints.end());

    // Post-processing to remove redundant waypoints
    std::vector<Waypoint> waypoints_clean;
    if (waypoints.size() > 2) {

      waypoints_clean.push_back(waypoints[0]);

      Waypoint prev = waypoints[0];
      Waypoint curr = waypoints[1];
      int dir_i = curr.i - prev.i;
      int dir_j = curr.j - prev.j;

      for (int i = 2; i < waypoints.size(); ++i) {
        prev = waypoints[i-1];
        curr  = waypoints[i];

        int new_dir_i = curr.i - prev.i;
        int new_dir_j = curr.j - prev.j;

        if (new_dir_i != dir_i or new_dir_j != dir_j) {
          waypoints_clean.push_back(prev);
          grid[prev.i][prev.j] = HIGHLIGHTED2;
        }

        dir_i = new_dir_i;
        dir_j = new_dir_j;
      }
    } else {
      waypoints_clean = waypoints;
    }
    waypoints_clean.push_back(destination);
    grid[destination.i][destination.j] = HIGHLIGHTED2;

    if(w == nil_w) {printf("No path found\n");}
    //else {printf("Path found\n");}

    return waypoints_clean;
  }

}
