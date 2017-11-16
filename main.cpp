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
#include "ObjLoader.h"
#include "Renderer20.h"
#include "Renderer30.h"
#include "Particle.h"
#include "Geometry.h"

#include <random>
#define rand01() ((float)std::rand()/RAND_MAX)

#define PI 3.1415926535

const float box_size = 10.0f;
const int num_particles = 20;
const int num_planes = 5;
const int num_spheres = 1;
const int num_triangles = 0;
const float gravity = 9.8f;
const float particle_mass = 0.1f;

// Spring stuff
const float k_e = 500.0f; // Elasticity constant
const float k_d = 0.5f; // Elasticity damping constant
const float L = 0.5; // Inter-particle spring length

void pause_program() {
    std::cout << "Press enter to continue ...";
    std::cin.get();
}


void render_particle(const Particle& p, GPUModelHandle gpu_model) {
    glm::vec3 pos = p.getCurrentPosition();

    //TODO: CopyPasted
    Vector3 camera_pos = vector::v(0, 10, 50); //TODO: CopyPasted
    Mat4 model; //TODO: CopyPasted
    Mat4 view = matrix::look_at(camera_pos, vector::zero(), vector::v(0,1,0)); //TODO: CopyPasted
    Mat4 projection = matrix::perspective(35, 16.0/9.0, 0.01, 100); //TODO: CopyPasted

    Vector3 location = vector::v(pos.x, pos.y, pos.z);
    Vector3 rotation = vector::zero();
    Vector3 scale = vector::v(0.2f, 0.2f, 0.2f);

    Transform particle_transform = transform::make_transform(location, rotation, scale);
    model = transform::transform_to_matrix(particle_transform);
    renderer30::render_model(gpu_model, model, view, projection);
}

void render_sphere(const Sphere& p, GPUModelHandle gpu_model) {
    glm::vec3 pos = p.center;

    //TODO: CopyPasted
    Vector3 camera_pos = vector::v(0, 10, 50); //TODO: CopyPasted
    Mat4 model; //TODO: CopyPasted
    Mat4 view = matrix::look_at(camera_pos, vector::zero(), vector::v(0,1,0)); //TODO: CopyPasted
    Mat4 projection = matrix::perspective(35, 16.0/9.0, 0.01, 100); //TODO: CopyPasted

    Vector3 location = vector::v(pos.x, pos.y, pos.z);
    Vector3 rotation = vector::zero();
    Vector3 scale = vector::v(p.radi, p.radi, p.radi);

    Transform particle_transform = transform::make_transform(location, rotation, scale);
    model = transform::transform_to_matrix(particle_transform);
    renderer30::render_model(gpu_model, model, view, projection);
}

void render_plane(const Plane& p, GPUModelHandle gpu_model) {
    if (!p.visible) return;

    glm::vec3 point = p.origin;
    glm::vec3 rot = p.rotation;


    Vector3 camera_pos = vector::v(0, 10, 50); //TODO: CopyPasted
    Mat4 model; //TODO: CopyPasted
    Mat4 view = matrix::look_at(camera_pos, vector::zero(), vector::v(0,1,0)); //TODO: CopyPasted
    Mat4 projection = matrix::perspective(35, 16.0/9.0, 0.01, 100); //TODO: CopyPasted

    Vector3 location = vector::v(point.x, point.y, point.z);
    Vector3 rotation = vector::v(rot.x, rot.y, rot.z);
    Vector3 scale = vector::v(box_size, box_size, box_size);

    Transform transform = transform::make_transform(location, rotation, scale);
    model = transform::transform_to_matrix(transform);
    renderer30::render_model(gpu_model, model, view, projection);
}

void render_triangle(const Triangle& p, GPUModelHandle gpu_model) {
    Vector3 camera_pos = vector::v(0, 10, 50); //TODO: CopyPasted
    Mat4 model; //TODO: CopyPasted
    Mat4 view = matrix::look_at(camera_pos, vector::zero(), vector::v(0,1,0)); //TODO: CopyPasted
    Mat4 projection = matrix::perspective(35, 16.0/9.0, 0.01, 100); //TODO: CopyPasted

    Vector3 location = vector::zero();
    Vector3 rotation = vector::zero();
    Vector3 scale = vector::one();

    Transform transform = transform::make_transform(location, rotation, scale);
    model = transform::transform_to_matrix(transform);
    renderer30::render_model(gpu_model, model, view, projection);
}

float dist_to_plane[num_particles][num_planes];
float get_distance_to_plane(int particle_idx, int plane_idx) {return dist_to_plane[particle_idx][plane_idx];}
void set_distance_to_plane(int particle_idx, int plane_idx, float value) {dist_to_plane[particle_idx][plane_idx] = value;}

float dist_to_tri_plane[num_particles][num_triangles];
float get_distance_to_tri_plane(int particle_idx, int tri_plane_idx) {return dist_to_tri_plane[particle_idx][tri_plane_idx];}
void set_distance_to_tri_plane(int particle_idx, int tri_plane_idx, float value) {dist_to_tri_plane[particle_idx][tri_plane_idx] = value;}

Particle init_particle(int i, int j) {
    Particle p(-5 + i * L, 5.0f, -5 + j * L); //initial position of the particle
    p.setLifetime(10000);//(rand01()*10.0f+10);
    p.setBouncing(0.1f);
    p.setMass(particle_mass);
    p.setForce(0, -gravity*particle_mass, 0);
    p.setVelocity(0,0,0);
    return p;
}

Model triangle_model(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
    Model m;

    Vector3 V1 = vector::v(v1.x, v1.y, v1.z);
    Vector3 V2 = vector::v(v2.x, v2.y, v2.z);
    Vector3 V3 = vector::v(v3.x, v3.y, v3.z);

    glm::vec3 normal = glm::normalize(glm::cross(v1,v2));
    Vector3 Normal = vector::v(normal.x, normal.y, normal.z);

    Vector2 uv = vec2(0,0);

    m.vertices = alloc_array<Vector3>(3);
    m.vertices[0] = V1;
    m.vertices[1] = V2;
    m.vertices[2] = V3;

    m.normals = alloc_array<Vector3>(3);
    m.normals[0] = Normal;
    m.normals[1] = Normal;
    m.normals[2] = Normal;

    m.uvs = alloc_array<Vector2>(3);
    m.uvs[0] = uv;
    m.uvs[1] = uv;
    m.uvs[2] = uv;

    m.faces = alloc_array<Face>(1);
    Face f;
    f.v1 = 0; f.v2 = 1; f.v3 = 2;
    f.n1 = 0; f.n2 = 1; f.n3 = 2;
    f.uv1 = 0; f.uv2 = 1; f.uv3 = 2;
    m.faces[0] = f;

    return m;

}

int main() {

    print("Starting particles project");

    // ==================
    // INIT DISPLAY STUFF
    // ==================
    __default_memory_allocator = mk_allocator(256*1024*1024);

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

    Model* sphere_model = objloader::read_model("./sphere.obj");
    Model* sphereHD_model = objloader::read_model("./sphere_hd.obj");
    Model* plane_model = objloader::read_model("./plane.obj");
    GPUModelHandle sphere_model_gpu = renderer30::upload_model_to_gpu(sphere_model);
    GPUModelHandle sphereHD_model_gpu = renderer30::upload_model_to_gpu(sphereHD_model);
    GPUModelHandle plane_model_gpu = renderer30::upload_model_to_gpu(plane_model);

    // ==================
    //  SIMULATION STUFF
    // ==================

    float dt = 0.01f;  //simulation time
    float initial_time = 0.0f;
    float end_time = 20.0f; //final time of simulation

    // particle initialization
    Array<Array<Particle> > particles = alloc_array<Array<Particle> >(num_particles);
    for (int i = 0; i < num_particles; ++i) {
        particles[i] = alloc_array<Particle>(num_particles);
        for (int j = 0; j < num_particles; ++j) {
            particles[i][j] = init_particle(i,j);
        }
    }
    for (int i = 0; i < num_particles; ++i) {
      particles[i][0].setFixed(true);
    }
    //particles[0].setFixed(true);

    // Planes definition
    Array<Plane> planes = alloc_array<Plane>(num_planes);
    glm::vec3 punt;
    glm::vec3 normal;
    glm::vec3 rotation;
    Plane pla;

    int current_plane = 0;

    // 1. Floor plane
    punt = glm::vec3(0.0f, -box_size, 0.0f);
    normal = glm::vec3(0, 1, 0);
    rotation = glm::vec3(0,0,PI/2);
    pla = Plane(punt, normal, rotation);
    planes[current_plane++] = pla;

    // 2. Left wall plane
    punt = glm::vec3(-box_size, 0.0f, 0.0f);
    normal = glm::vec3(1, 0, 0);
    rotation = glm::vec3(0,0,0);
    pla = Plane(punt, normal, rotation);
    planes[current_plane++] = pla;

    // 3. Right wall plane
    punt = glm::vec3(box_size, 0.0f, 0.0f);
    normal = glm::vec3(-1, 0, 0);
    rotation = glm::vec3(0,-PI,0);
    pla = Plane(punt, normal, rotation);
    planes[current_plane++] = pla;

    // 4. Back wall plane
    punt = glm::vec3(0.0f, 0.0f, -box_size);
    normal = glm::vec3(0, 0, -1);
    rotation = glm::vec3(0,-PI/2,0);
    pla = Plane(punt, normal, rotation);
    planes[current_plane++] = pla;

    // 5. Front wall plane
    punt = glm::vec3(0.0f, 0.0f, box_size);
    normal = glm::vec3(0, 0, -1);
    rotation = glm::vec3(0,-PI/2,0);
    pla = Plane(punt, normal, rotation);
    pla.visible = false;
    planes[current_plane++] = pla;

    assert(current_plane == num_planes);

    Array<Triangle> triangles = alloc_array<Triangle>(num_triangles);
  

    // Spheres
    Array<Sphere> spheres = alloc_array<Sphere>(num_spheres);
    int current_sphere = 0;
    spheres[current_sphere++] = Sphere(glm::vec3(0,-1,0), 4.0f);

    assert(current_sphere == num_spheres);


    // simulation loop
    //int count = 0;
    float time = initial_time;

    for (int i = 0; i < num_particles; ++i) {
        for (int ii = 0; ii < num_particles; ++ii) {
            for (int j = 0; j < num_planes; ++j) {
                set_distance_to_plane(i,j, planes[j].distPoint2Plane(particles[i][ii].getCurrentPosition()));
            }
        }
    }
    for (int i = 0; i < num_particles; ++i) {
        for (int ii = 0; ii < num_particles; ++ii) {
            for (int j = 0; j < num_triangles; ++j) {
                set_distance_to_tri_plane(i,j, triangles[j].plane.distPoint2Plane(particles[i][ii].getCurrentPosition()));
            }
        }
    }

    bool quit = false;

    SDL_Event e;
    SDL_StartTextInput();


    // Main loop
    while (time <= end_time && !quit) {

        // Keyboard events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } //else if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.sym == SDLK_l) {
            // foo();
            //}
        }

        const int neighbours[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        // Spring forces loop
        for (int i = 0; i < num_particles; ++i) {
            for (int j = 0; j < num_particles; ++j) {
                Particle particle = particles[i][j];
                glm::vec3 P_p = particle.getCurrentPosition();
                glm::vec3 V_p = particle.getVelocity();

                glm::vec3 f_g(0, -gravity*particle_mass, 0);
                glm::vec3 f_s(0,0,0);

                for (int n = 0; n < 4; ++n) {

                    int ni = i + neighbours[n][0];
                    int nj = j + neighbours[n][1];

                    if (ni < 0 || ni >= num_particles) continue;
                    if (nj < 0 || nj >= num_particles) continue;

                    Particle neighbour = particles[ni][nj];
                    glm::vec3 P_n = neighbour.getCurrentPosition();
                    glm::vec3 V_n = neighbour.getVelocity();

                    glm::vec3 dir = glm::normalize(P_n - P_p);

                    f_s += k_e * (glm::length(P_n - P_p) - L) * dir;
                    f_s += k_d * glm::dot(V_n - V_p, dir) * dir;
                }

                particles[i][j].setForce(f_s + f_g);
            }
        }

        // Physics loop
        for (int i = 0; i < num_particles; ++i) {
            for (int j = 0; j < num_particles; ++j) {
                if (particles[i][j].getLifetime() > 0) {

                    particles[i][j].updateParticle(dt, Particle::UpdateMethod::Verlet);

                    Particle p = particles[i][j];
                    glm::vec3 pos = p.getCurrentPosition();
                    glm::vec3 old_pos = p.getPreviousPosition();
                    glm::vec3 vel = p.getVelocity();

                    particles[i][j].setLifetime(p.getLifetime() - dt);

                    bool collided = false;

                    // Plane collisions
                    for (int pl = 0; !collided && pl < num_planes; ++pl) {
                        float prev_distance = get_distance_to_plane(i, pl);
                        float curr_distance = planes[pl].distPoint2Plane(p.getCurrentPosition());

                        if (prev_distance*curr_distance < 0.0f) {
                            //TODO: CopyPasted plane collision code

                            glm::vec3 n = planes[pl].normal;
                            float d = planes[pl].dconst;
                            float e = p.getBouncing();

                            particles[i][j].setPosition(pos - (1+e)*(glm::dot(n,pos) + d)*n);
                            particles[i][j].setVelocity(vel - (1+e)*(glm::dot(n,vel))*n);

                            set_distance_to_plane(i, pl, -curr_distance);

                        }
                    }

                    // Triangle collisions
                    for (int tr = 0; !collided && tr < num_triangles; ++tr) {
                        //TODO: CopyPasted plane collision code
                        Triangle t = triangles[tr];
                        float prev_distance = get_distance_to_tri_plane(i, tr);
                        float curr_distance = triangles[tr].plane.distPoint2Plane(p.getCurrentPosition());

                        if (prev_distance*curr_distance < 0.0f && t.isInside(pos)) {

                            glm::vec3 n = triangles[tr].plane.normal;
                            float d = triangles[tr].plane.dconst;
                            float e = p.getBouncing();

                            particles[i][j].setPosition(pos - (1+e)*(glm::dot(n,pos) + d)*n);
                            particles[i][j].setVelocity(vel - (1+e)*(glm::dot(n,vel))*n);

                            set_distance_to_tri_plane(i, tr, -curr_distance);
                        }
                        else {
                            set_distance_to_tri_plane(i, tr, curr_distance);
                        }


                    }

                    // Sphere collisions
                    for (int sp = 0; !collided && sp < num_spheres; ++sp) {
                        Sphere sphere = spheres[sp];
                        float dist = glm::length(p.getCurrentPosition() - sphere.center);

                        if (dist < sphere.radi) {
                            //TODO: CopyPasted plane collision code
                            Plane tangent_plane = sphere.tangentContactPlane(old_pos, sphere.center);

                            glm::vec3 n = tangent_plane.normal;
                            float d = tangent_plane.dconst;
                            float e = p.getBouncing();

                            particles[i][j].setPosition(pos - (1+e)*(glm::dot(n,pos) + d)*n);
                            particles[i][j].setVelocity(vel - (1+e)*(glm::dot(n,vel))*n);

                        }
                    }

                    //NOTE: We assume a particle can only collide with a plane in a
                    //     given timestep. This could potentially lead to issues but
                    //     it should be fine for our purposes.


                } else {
                    particles[i][j] = init_particle(0,0); // Never used
                }
            }
        }

        // Render Loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < num_particles; ++i) {
            for (int j = 0; j < num_particles; ++j) {
                render_particle(particles[i][j], sphere_model_gpu);
            }
        }
        for (int i = 0; i < num_planes; ++i) {
            render_plane(planes[i], plane_model_gpu);
        }
        for (int i = 0; i < num_triangles; ++i) {
            render_triangle(triangles[i], *(triangles[i].model));
        }
        for (int i = 0; i < num_spheres; ++i) {
            render_sphere(spheres[i], sphereHD_model_gpu);
        }
        SDL_GL_SwapWindow(sdl_data.gWindow);

        time = time + dt; //increase time counter
    }

    return 0;
}
