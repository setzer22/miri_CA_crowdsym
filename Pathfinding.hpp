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

#include "Array.hpp"
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

  const float inf = 1e20;
  const NeighborhoodType neigh_t = NEIGH8;
  const HeuristicType heur_t = MANHATTAN;

  float manhattan(Waypoint w1, Waypoint w2) {
    return abs(w1.j - w2.j) + abs(w1.i - w2.i);
  }

  float euclidean(Waypoint w1, Waypoint w2) {
    return sqrt((w1.j - w2.j)*(w1.j - w2.j) + (w1.i - w2.i)*(w1.i - w2.i));
  }

  std::vector<Waypoint> neighbours(Grid& grid, const Waypoint w) {
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
        
        int ni = neighborhood_4[n][0];
        int nj = neighborhood_4[n][1];
        //printf("Pushing neighbour (%d,%d)\n", ni, nj);
        if(w.i+ni < grid.size and w.i+ni >= 0 and
             w.j+nj < grid.size and w.j+nj >= 0 and
           grid[w.i+ni][w.j+nj] != OBSTACLE) {
            neighs.push_back(Waypoint(w.i+ni, w.j+nj));
        }
      }
    }
    return neighs;
  };
  
  typedef std::pair<Waypoint, Waypoint> WaypointPair;

  std::vector<Waypoint> remove_unnecesary_waypoints(const std::vector<Waypoint>& waypoints, Grid& grid) {
    std::vector<Waypoint> waypoints_clean;
    if(waypoints.size() > 2) {
      waypoints_clean.push_back(waypoints[0]);

      Waypoint prev = waypoints[0];
      Waypoint curr = waypoints[1];
      int dir_i = curr.i - prev.i;
      int dir_j = curr.j - prev.j;

      //printf("(%d, %d)+(%d, %d)", curr.i, curr.j, dir_i, dir_j);

      for (int i = 2; i < (int)waypoints.size(); ++i) {
        prev = waypoints[i-1];
        curr  = waypoints[i];

        //printf("(%d, %d)+(%d, %d)\n", curr.i, curr.j, dir_i, dir_j);

        int new_dir_i = curr.i - prev.i;
        int new_dir_j = curr.j - prev.j;

        if (new_dir_i != dir_i or new_dir_j != dir_j) {
          waypoints_clean.push_back(prev);
          grid[prev.i][prev.j] = HIGHLIGHTED3;
        }

        dir_i = new_dir_i;
        dir_j = new_dir_j;
      }
      Waypoint destination = waypoints[waypoints.size()-1];
      grid[destination.i][destination.j] = HIGHLIGHTED3;
      return waypoints_clean;
    }
    else {
      return waypoints;
    }
  }

  std::vector<Waypoint> bidirectional_search(Grid grid, Waypoint origin, Waypoint destination) {

    Waypoint nil_w(-1, -1); // A special marker for the {fw,bw_}came_from matrices
    std::vector<std::vector<float>> fw_cost_so_far(grid.size, std::vector<float>(grid.size, inf));
    std::vector<std::vector<Waypoint>> fw_came_from(grid.size, std::vector<Waypoint>(grid.size, nil_w));
    std::vector<std::vector<float>> bw_cost_so_far(grid.size, std::vector<float>(grid.size, inf));
    std::vector<std::vector<Waypoint>> bw_came_from(grid.size, std::vector<Waypoint>(grid.size, nil_w));

    fw_cost_so_far[origin.i][origin.j] = 0;
    bw_cost_so_far[destination.i][destination.j] = 0;

    // Cost function
    auto cost = [](const std::vector<std::vector<float>>& cost_so_far, const Waypoint destination, const Waypoint w){
      return cost_so_far[w.i][w.j] + ((heur_t == MANHATTAN) ? manhattan(w, destination) : euclidean(w, destination));
    };

    // Comparison function
    auto fw_comp = [&fw_cost_so_far, destination, cost](const Waypoint w1, const Waypoint w2) {
      return cost(fw_cost_so_far, destination, w1) > cost(fw_cost_so_far, destination, w2);};
    auto bw_comp = [&bw_cost_so_far, origin, cost](const Waypoint w1, const Waypoint w2) {
      return cost(bw_cost_so_far, origin, w1) > cost(bw_cost_so_far, origin, w2);};

    std::priority_queue<Waypoint, std::vector<Waypoint>, std::function<bool(const Waypoint, const Waypoint)>>
      fw_frontier(fw_comp);
    std::priority_queue<Waypoint, std::vector<Waypoint>, std::function<bool(const Waypoint, const Waypoint)>>
      bw_frontier(bw_comp);

    fw_frontier.push(origin);
    bw_frontier.push(destination);

    Waypoint met_at = nil_w;

    while(not fw_frontier.empty() and not bw_frontier.empty()) {
      Waypoint fw_current = fw_frontier.top();
      fw_frontier.pop();
      Waypoint bw_current = bw_frontier.top();
      bw_frontier.pop();

      //printf("fw: (%d,%d), bw: (%d,%d)\n", fw_current.i, fw_current.j, bw_current.i, bw_current.j);

      if(bw_cost_so_far[fw_current.i][fw_current.j] < inf) {
        //i.e. If fw current already explored on bw search
        met_at = fw_current;
        break;
      }
      else if(fw_cost_so_far[bw_current.i][bw_current.j] < inf) {
        //i.e. If fw current already explored on bw search
        met_at = bw_current;
        break;
      }
      else { // Keep searching...
        for (auto w : neighbours(grid, fw_current)) {
          //printf("FW Exploring neighbour (%d,%d)\n", w.i, w.j);
          float d = glm::distance(glm::vec2(fw_current.i, fw_current.j), glm::vec2(w.i, w.j));
          float new_cost = fw_cost_so_far[fw_current.i][fw_current.j]+d;
          if (new_cost < fw_cost_so_far[w.i][w.j]) {
            grid[w.i][w.j] = HIGHLIGHTED;
            fw_cost_so_far[w.i][w.j] = new_cost;
            fw_frontier.push(w);
            fw_came_from[w.i][w.j] = fw_current;
          }
        }
        // --- s/fw_/bw_/g ---
        for (auto w : neighbours(grid, bw_current)) {
          //printf("BW Exploring neighbour (%d,%d)\n", w.i, w.j);
          float d = glm::distance(glm::vec2(bw_current.i, bw_current.j), glm::vec2(w.i, w.j));
          float new_cost = bw_cost_so_far[bw_current.i][bw_current.j]+d;
          if (new_cost < bw_cost_so_far[w.i][w.j]) {
            grid[w.i][w.j] = HIGHLIGHTED2;
            bw_cost_so_far[w.i][w.j] = new_cost;
            bw_frontier.push(w);
            bw_came_from[w.i][w.j] = bw_current;
          }
        } // -----------------
      }
    }

    if (met_at == nil_w) {
      return std::vector<Waypoint>();
    } else {

      std::vector<Waypoint> fw_path;
      Waypoint w = met_at;
      while (w != origin and w != nil_w) {
        grid[w.i][w.j] = SELECTED;
        fw_path.push_back(w);
        w = fw_came_from[w.i][w.j];
      }
      std::reverse(fw_path.begin(), fw_path.end());

      std::vector<Waypoint> bw_path;
      w = met_at;
      while (w != origin and w != nil_w) {
        grid[w.i][w.j] = SELECTED;
        bw_path.push_back(w);
        w = bw_came_from[w.i][w.j];
      }

      fw_path.insert(fw_path.end(), bw_path.begin(), bw_path.end());
      fw_path.push_back(destination);

      return remove_unnecesary_waypoints(fw_path, grid);
    }
  }

  std::vector<Waypoint> basic_a_star(Grid grid, Waypoint origin, Waypoint destination) {

    Waypoint nil_w(-1, -1); // A special marker for the came_from matrix
    std::vector<std::vector<float> > cost_so_far(grid.size, std::vector<float>(grid.size, inf));
    std::vector<std::vector<Waypoint> > came_from(grid.size, std::vector<Waypoint>(grid.size, nil_w));

    cost_so_far[origin.i][origin.j] = 0;

    // Cost function
    auto cost = [origin, &cost_so_far, destination](const Waypoint w){
      return cost_so_far[w.i][w.j] + ((heur_t == MANHATTAN) ? manhattan(w, destination) : euclidean(w, destination));
    };

    // Comparison function
    auto comp = [cost](const Waypoint w1, const Waypoint w2) {
      //Note: Priority and cost are opposite concepts, hence the >
      return cost(w1) > cost(w2);
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

      for (auto w : neighbours(grid, current)) {
        //printf("Exploring neighbour (%d,%d)\n", w.i, w.j);
        float d = glm::distance(glm::vec2(current.i, current.j), glm::vec2(w.i, w.j));
        float new_cost = cost_so_far[current.i][current.j]+d;
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
    waypoints.push_back(destination);

    //if(w == nil_w) {printf("No path found\n");}
    //else {printf("Path found\n");}

    return remove_unnecesary_waypoints(waypoints, grid);
  }



}
