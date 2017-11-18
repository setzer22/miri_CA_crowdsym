#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <map>

#include "Utils.h"
#include "StackTrace.h"
#include "Allocator.h"
#include "Array.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "SDLUtilities.h"
#include "GLUtilities.h"
#include "Input.h"
#include "GPUModelHandle.h"
#include "Particle.h"
#include "Geometry.h"
#include "model.h"

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

// Spring stuff
const float k_e = 500.0f; // Elasticity constant
const float k_d = 0.5f; // Elasticity damping constant
const float L = 0.5; // Inter-particle spring length

float dist_to_plane[num_particles][num_planes];
float get_distance_to_plane(int particle_idx, int plane_idx) {return dist_to_plane[particle_idx][plane_idx];}
void set_distance_to_plane(int particle_idx, int plane_idx, float value) {dist_to_plane[particle_idx][plane_idx] = value;}

float dist_to_tri_plane[num_particles][num_triangles];
float get_distance_to_tri_plane(int particle_idx, int tri_plane_idx) {return dist_to_tri_plane[particle_idx][tri_plane_idx];}
void set_distance_to_tri_plane(int particle_idx, int tri_plane_idx, float value) {dist_to_tri_plane[particle_idx][tri_plane_idx] = value;}

Particle init_particle() {
    Particle p(0, 0, 0); //initial position of the particle
    p.setLifetime(1000000);//(rand01()*10.0f+10);
    p.setBouncing(1.0f);
    p.setMass(1.0f);
    p.setVelocity(2*(rand01()-0.5)*10.0f, 2*(rand01()-0.5)*10.0f, 0.0f);
    return p;
}

void render_plane(const Plane& p) {

  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glm::vec3 o = p.origin;
  glm::vec3 n = glm::normalize(p.normal);
  glm::vec3 up(0,0,1);
  glm::vec3 r = glm::cross(n, up);

  float hscale = box_size;
  float vscale = 0.25*box_size;

  glm::vec3 points[4];
  points[0] = o + r*hscale + up*vscale;
  points[1] = o + r*hscale;
  points[2] = o + -r*hscale;
  points[3] = o + -r*hscale + up*vscale;

  glBegin(GL_QUADS);
  for (int i = 0; i < 4; ++i) {
    glNormal3f(n.x, n.y, n.z);
    glColor3f(0.4, 0.0, 0.2);
    glVertex3f(points[i].x, points[i].y, points[i].z);
  }
  glEnd();

  glDisable(GL_DEPTH_TEST);

}

int main() {

    print("Starting characters project");

    // ==================
    // INIT DISPLAY STUFF
    // ==================
    __default_memory_allocator = mk_allocator(256*1024*1024);

    Model m;
    m.setPath("./cally/data/paladin/");
    m.onInit("./cally/data/paladin.cfg");

    m.setState(Model::STATE_MOTION, 0.3f);
    

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


    for (int i = 0; i < num_particles; ++i) {
      for (int j = 0; j < num_planes; ++j) {
        set_distance_to_plane(i,j, planes[j].distPoint2Plane(particles[i].getCurrentPosition()));
      }
    }

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
            } //else if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.sym == SDLK_l) {
            // foo();
            //}
        }

        // Physics loop
        for (int i = 0; i < num_particles; ++i) {
          particles[i].updateParticle(dt, Particle::UpdateMethod::EulerSemi);

          Particle p = particles[i];
          glm::vec3 pos = p.getCurrentPosition();
          glm::vec3 old_pos = p.getPreviousPosition();
          glm::vec3 vel = p.getVelocity();

          bool collided = false;

          for (int pl = 0; !collided && pl < num_planes; ++pl) {
            float prev_distance = get_distance_to_plane(i, pl);
            float curr_distance = planes[pl].distPoint2Plane(p.getCurrentPosition());

            if (prev_distance*curr_distance < 0.0f) {


              glm::vec3 n = planes[pl].normal;
              float d = planes[pl].dconst;
              float e = p.getBouncing();

              particles[i].setPosition(pos - (1+e)*(glm::dot(n,pos) + d)*n);
              particles[i].setVelocity(vel - (1+e)*(glm::dot(n,vel))*n);

              set_distance_to_plane(i, pl, -curr_distance);

            }
          }

        }

        // Render Loop
        m.onUpdate(dt);

        glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set the projection transformation
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float renderScale = 0.05f; m.getRenderScale();
        gluPerspective(45.0f, (GLdouble)1280 / (GLdouble)720, 0.01, 5000.0);

        static float camera_distance(100.0f);
        static glm::vec2 camera_pos(-60.0f, 1.0f);

        // set the light position and attributes
        const GLfloat lightPosition[] = { 1.0f, -1.0f, 1.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        const GLfloat lightColorAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightColorAmbient);
        const GLfloat lightColorDiffuse[] = { 0.52f, 0.5f, 0.5f, 1.0f };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColorDiffuse);
        const GLfloat lightColorSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };

        // set the model transformation
        for (int i = 0; i < particles.size; ++i) {
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();

          glLightfv(GL_LIGHT0, GL_SPECULAR, lightColorSpecular);

          // Camera transform
          gluLookAt(camera_pos.x,camera_pos.y,camera_distance,0,0,0,0,0,1);


          // Model transform
          glm::vec3 pos = particles[i].getCurrentPosition();
          glm::vec3 vel = particles[i].getVelocity();
          glTranslatef(pos.x, pos.y, pos.z);
          glScalef(renderScale, renderScale, renderScale);

          m.onRender();
        }

        for (int i = 0; i < num_planes; ++i) {
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
          gluLookAt(camera_pos.x,camera_pos.y,camera_distance,0,0,0,0,0,1);
          glLightfv(GL_LIGHT0, GL_SPECULAR, lightColorSpecular);
          render_plane(planes[i]);
        }




        SDL_GL_SwapWindow(sdl_data.gWindow);

        time = time + dt; //increase time counter
    }

    return 0;
}
