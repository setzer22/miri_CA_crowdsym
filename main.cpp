#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <map>

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "Utils.h"
#include "StackTrace.h"
#include "Allocator.h"
#include "Array.h"
#include "SDLUtilities.h"
#include "Input.h"
#include "Particle.h"
#include "Geometry.h"
#include "model.h"
#include "PlaneRenderer.h"
#include "gridRenderer.hpp"
#include "GridGeometry.hpp"
#include "Gizmo.h"
#include "Texture.h"
#include "Animator.hpp"

#include <random>
#define rand01() ((float)std::rand()/RAND_MAX)

#define PI 3.1415926535

const float box_size = 25.0f;
const int num_particles = 5;
const int num_planes = 4;
const int num_spheres = 1;
const int num_triangles = 0;
const float gravity = 9.8f;
const float particle_mass = 0.1f;

float base_particle_velocity = 6.0f;

// Spring stuff
const float k_e = 500.0f; // Elasticity constant
const float k_d = 0.5f; // Elasticity damping constant
const float L = 0.5; // Inter-particle spring length

float dist_to_plane[num_particles][num_planes];
float get_distance_to_plane(int particle_idx, int plane_idx) {return dist_to_plane[particle_idx][plane_idx];}
void set_distance_to_plane(int particle_idx, int plane_idx, float value) {dist_to_plane[particle_idx][plane_idx] = value;}

const int MAX_TRIANGLES = 20*20*4*2;
float dist_to_tri_plane[num_particles][MAX_TRIANGLES];
float get_distance_to_tri_plane(int particle_idx, int tri_plane_idx) {return dist_to_tri_plane[particle_idx][tri_plane_idx];}
void set_distance_to_tri_plane(int particle_idx, int tri_plane_idx, float value) {dist_to_tri_plane[particle_idx][tri_plane_idx] = value;}

Particle init_particle() {
    Particle p(0, 0, 0); //initial position of the particle
    p.setLifetime(1000000);//(rand01()*10.0f+10);
    p.setBouncing(1.0f);
    p.setMass(1.0f);
    p.setRadius(1.0f);
    glm::vec3 vel = glm::normalize(glm::vec3(2*(rand01()-0.5), 2*(rand01()-0.5), 0.0f)) * base_particle_velocity;
    p.setVelocity(vel);
    return p;
}

glm::vec2 camera_pos(0.0f, 0.0f);
glm::vec3 camera_offset(0.0f, -60.0f, 30.0f);

int main() {

    print("Starting characters project");

    // ==================
    // INIT DISPLAY STUFF
    // ==================
    __default_memory_allocator = mk_allocator(256*1024*1024);

    auto models = alloc_array<Model*>(num_particles);
    auto animators = alloc_array<animator::Animator>(num_particles);
    for (int i = 0; i < num_particles; ++i) {
      auto* m = new Model();
      models[i] = m;
      models[i]->setPath("./cally/data/paladin/");
      models[i]->onInit("./cally/data/paladin.cfg");
      models[i]->setState(Model::STATE_MOTION, 0.3f);

      animator::Animator a;
      animators[i] = a;
    }


    SDL_Init_Result sdl_data = init_SDL(1280, 720);
    if (!sdl_data.success) {
        error("Failed to execute");
        exit(0);
    }
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        error("Failed to initialize GLEW");
    }



    // ==================
    //  SIMULATION STUFF
    // ==================

    // Particles Definition
    float dt = 0.01f;  //simulation time
    float initial_time = 0.0f;
    Array<Particle> particles = alloc_array<Particle>(num_particles);
    for (int i = 0; i < particles.size; ++i) {
      particles[i] = init_particle();
    }

    // Planes definition
    plane_renderer::init();


    Array<Plane> planes = alloc_array<Plane>(num_planes);
    glm::vec3 punt;
    glm::vec3 normal;
    glm::vec3 rotation;
    Plane pla;

    int current_plane = 0;
    {
      // 1. Left wall plane
      punt = glm::vec3(-box_size, 0.0f, 0.0f);
      normal = glm::vec3(1, 0, 0);
      rotation = glm::vec3(0,0,0);
      pla = Plane(punt, normal, rotation);
      planes[current_plane++] = pla;

      // 2. Right wall plane
      punt = glm::vec3(box_size, 0.0f, 0.0f);
      normal = glm::vec3(-1, 0, 0);
      rotation = glm::vec3(0,-PI,0);
      pla = Plane(punt, normal, rotation);
      planes[current_plane++] = pla;

      // 3. Back wall plane
      punt = glm::vec3(0.0f, -box_size, 0);
      normal = glm::vec3(0, 1, 0);
      rotation = glm::vec3(0,-PI/2,0);
      pla = Plane(punt, normal, rotation);
      planes[current_plane++] = pla;

      // 4. Front wall plane
      punt = glm::vec3(0.0f, box_size, 0);
      normal = glm::vec3(0, -1, 0);
      rotation = glm::vec3(0,-PI/2,0);
      pla = Plane(punt, normal, rotation);
      pla.visible = false;
      planes[current_plane++] = pla;

    }

    Plane floor(glm::vec3(0,0,0), glm::vec3(0,0,1), glm::vec3(0,0,0));

    for (int i = 0; i < num_particles; ++i) {
      for (int j = 0; j < num_planes; ++j) {
        set_distance_to_plane(i,j, planes[j].distPoint2Plane(particles[i].getCurrentPosition()));
      }
    }

    // Load the grid from file
    auto grid = grid_renderer::read_from_file("./map.dat");

    // Collision geometry (Triangles)
    Array<Triangle> geom_triangles = grid_geometry::make_planes(grid);

    float time = initial_time;

    bool quit = false;

    SDL_Event e;
    SDL_StartTextInput();


    // Main loop
    while (!quit) {

        // Keyboard events
        while (SDL_PollEvent(&e) != 0) {
          if (e.type == SDL_QUIT) {
            quit = true;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_w) {
            camera_pos.y += 100*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) {
            camera_pos.y -= 100*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_a) {
            camera_pos.x -= 100*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d) {
            camera_pos.x += 100*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q) {
            camera_offset.y -= 100*dt;
            camera_offset.z -= 60*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e) {
            camera_offset.y += 100*dt;
            camera_offset.z += 60*dt;
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_u) {
            for (int i = 0; i < particles.size; ++i) {
              printf("Speed: %f\n", base_particle_velocity);
              base_particle_velocity += 0.25f/4.0f;
              if (base_particle_velocity >= 20) {base_particle_velocity = 20;}
              particles[i].setVelocity(glm::normalize(particles[i].getVelocity())*base_particle_velocity);
            }
          } else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_j) {
            for (int i = 0; i < particles.size; ++i) {
              printf("Speed: %f\n", base_particle_velocity);
              base_particle_velocity = base_particle_velocity - 0.25f/4.0f;
              if (base_particle_velocity <= 0) {base_particle_velocity = 0.0001;}
              particles[i].setVelocity(glm::normalize(particles[i].getVelocity())*base_particle_velocity);
            }
          }

          /*else if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.sym == SDLK_w) {
            camera_pos.y += 100*dt;
            }*/
        }

        // Physics loop
        for (int i = 0; i < num_particles; ++i) {
          particles[i].updateParticle(dt, Particle::UpdateMethod::EulerSemi);

          Particle p = particles[i];
          glm::vec3 pos = p.getCurrentPosition();
          glm::vec3 old_pos = p.getPreviousPosition();
          glm::vec3 vel = p.getVelocity();

          bool collided = false;

          // Collision with other characters
          for (int cha = 0; cha < particles.size; ++cha) {
            if (cha == i) continue;

            Particle other = particles[cha];

            if(glm::distance(other.getCurrentPosition(), p.getCurrentPosition()) < other.getRadius() + p.getRadius()) {
              glm::vec3 midpoint = 0.5f * (other.getCurrentPosition() + p.getCurrentPosition());
              glm::vec3 correction_dir = glm::normalize(other.getCurrentPosition() - midpoint);
              particles[cha].setPosition(other.getCurrentPosition() + correction_dir * other.getRadius());
              particles[i].setPosition(p.getCurrentPosition() - correction_dir * p.getRadius());
            }
          }

          // Collision with outer walls
          for (int pl = 0; !collided && pl < num_planes; ++pl) {
            //float prev_distance = get_distance_to_plane(i, pl);
            float curr_distance = planes[pl].distPoint2Plane(p.getCurrentPosition());

            if (curr_distance < p.getRadius()) {


              glm::vec3 n = planes[pl].normal;
              float d = planes[pl].dconst;
              float e = p.getBouncing();


              particles[i].setVelocity(vel - (1+e)*(glm::dot(n,vel))*n);

              //if(curr_distance > 0) {particles[i].setPosition(pos + glm::normalize(n) * (p.getRadius() - curr_distance));}
              //else {particles[i].setPosition(pos - glm::normalize(n) * (p.getRadius() + curr_distance));}

            }
          }

          // Collision with obstacles
          for (int tri = 0; !collided && tri < geom_triangles.size; ++tri) {
            Triangle triangle = geom_triangles[tri];
            float curr_distance = triangle.plane.distPoint2Plane(p.getCurrentPosition());
            bool inside = triangle.isInside(p.getCurrentPosition());

            if(abs(curr_distance) < p.getRadius() and inside) {
              collided = true;

              glm::vec3 n = triangle.plane.normal;
              float d = triangle.plane.dconst;
              float e = p.getBouncing();

              if(curr_distance > 0) {particles[i].setPosition(pos + glm::normalize(n) * (p.getRadius() - curr_distance));}
              else {particles[i].setPosition(pos - glm::normalize(n) * (p.getRadius() + curr_distance));}

            }
          }

        }

        // Render Loop
        for (int i = 0; i < animators.size; ++i) {
          animator::update(particles[i], *(models[i]), animators[i], dt);
        }

        glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set the projection transformation
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float renderScale = 0.05f; models[0]->getRenderScale();
        gluPerspective(45.0f, (GLdouble)1280 / (GLdouble)720, 0.01, 5000.0);

        gizmo::render(glm::vec3(0,0,0), 5);

        grid_renderer::render(2*box_size, glm::vec2(-box_size, -box_size), grid);

        // set the light position and attributes
        const GLfloat lightPosition[] = { 1.0f, -1.0f, 1.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        const GLfloat lightColorAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightColorAmbient);
        const GLfloat lightColorDiffuse[] = { 0.52f, 0.5f, 0.5f, 1.0f };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColorDiffuse);
        const GLfloat lightColorSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };

        const GLfloat lightAttenuation[] = {0.01f};
        glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuation);


        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gluLookAt(camera_offset.x+camera_pos.x, camera_offset.y+camera_pos.y, camera_offset.z,
                  camera_pos.x, camera_pos.y, 0, 0,0,1);
        glPushMatrix();

        glDisable(GL_LIGHTING);

        // set the model transformation
        for (int i = 0; i < particles.size; ++i) {
          //glMatrixMode(GL_MODELVIEW);
          //glLoadIdentity();

          glPushMatrix();

          glLightfv(GL_LIGHT0, GL_SPECULAR, lightColorSpecular);

          // Camera transform


          // Model transform
          glm::vec3 pos = particles[i].getCurrentPosition();
          glm::vec3 vel = particles[i].getVelocity();
          glTranslatef(pos.x, pos.y, pos.z);
          glScalef(renderScale, renderScale, renderScale);

          glm::vec3 y = -glm::normalize(vel);
          glm::vec3 z = glm::vec3(0,0,1);
          glm::vec3 x = glm::cross(y, z);

          float data[] = {x.x, x.y, x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,0,0,0,1};
          glMultMatrixf(&data[0]);

          //static float the_angle = 0;
          //the_angle += 10*dt;

          /*
          glm::mat4 mat = glm::orientation(vel, glm::vec3(0,0,1));
          float mat_data[] = {mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                              mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                              mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                              mat[3][0], mat[3][1], mat[3][2], mat[3][3]};
          */

          //glMultMatrixf(&mat_data[0]);

          models[i]->onRender();

          glPopMatrix();
        }

        for (int i = 0; i < num_planes; ++i) {
          glLightfv(GL_LIGHT0, GL_SPECULAR, lightColorSpecular);
          if (i == 2) continue;
          plane_renderer::render_plane(planes[i], box_size);
        }
        plane_renderer::render_plane(floor, box_size);

        // DEBUG: Collision geometry renderer
        /*
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glColor3f(0.0, 1.0, 0.0);
        for (int i = 0; i < geom_triangles.size; ++i) {
          Triangle triangle = geom_triangles[i];
          glBegin(GL_LINES);
          glVertex3f(triangle.vertex1.x, triangle.vertex1.y, triangle.vertex1.z);
          glVertex3f(triangle.vertex2.x, triangle.vertex2.y, triangle.vertex2.z);
          glVertex3f(triangle.vertex2.x, triangle.vertex2.y, triangle.vertex2.z);
          glVertex3f(triangle.vertex3.x, triangle.vertex3.y, triangle.vertex3.z);
          glVertex3f(triangle.vertex3.x, triangle.vertex3.y, triangle.vertex3.z);
          glVertex3f(triangle.vertex1.x, triangle.vertex1.y, triangle.vertex1.z);
          glEnd();
        }
        */

        glPopMatrix();

        SDL_GL_SwapWindow(sdl_data.gWindow);

        time = time + dt; //increase time counter
    }

    return 0;
}
