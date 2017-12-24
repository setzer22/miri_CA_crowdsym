#pragma once

#include <SDL2/SDL_opengl.h>
#include "glm/glm.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include "global.h"

#include "model.h"
#include "Array.h"
#include "Particle.h"
#include "Geometry.h"

namespace animator {

  enum PaladinAnimations {
    IDLE = 0,
    WALK = 1,
    JOG = 2,
  };
  const int NUM_ANIMATIONS = 3;

  struct Animator {
    float animation_speed = 1.0f;
    float pose_interpolation[NUM_ANIMATIONS] = {1.0f, 0.0f, 0.0f};
  };

  Animator mk_animator() {
    return Animator();
  }

  const float idle_speed = 0.001f;
  const float almost_run_speed = 8.0f;
  const float run_speed = 10.0f;


  void update(Particle& particle, Model& model, Animator& animator, float dt) {

    // Update Animation State based on particle speed
    float speed = glm::length(particle.getVelocity());
    animator.pose_interpolation[IDLE] = 0;
    animator.pose_interpolation[WALK] = 0;
    animator.pose_interpolation[JOG] = 0;
    // ==== IDLE ====
    if (speed <= idle_speed) {
      animator.pose_interpolation[IDLE] = 1.0f;
    }
    // ==== WALK ====
    else if (speed > idle_speed and speed < almost_run_speed) {
      animator.pose_interpolation[WALK] = 1.0f;
      animator.animation_speed = speed / 6.0f;
    }
    // ==== ALMOST RUN ====
    else if (speed >= almost_run_speed and speed < run_speed) {
      animator.pose_interpolation[WALK] = 0.2f;
      animator.pose_interpolation[JOG] = 0.8f;
      animator.animation_speed = speed / 8.0f;
    }
    // ==== RUN ====
    else if (speed >= run_speed) {
      animator.pose_interpolation[JOG] = 1.0f;
      animator.animation_speed = speed / 10.0f;
    }

    CalModel* cal_model = model.getCal3DModel();
    //cal_model->getMixer();
    //for (int i = 0; i < 3; ++i) {
    //cal_model->getMixer()->clearCycle(i, animator.pose_interpolation[i]);
    //}
    cal_model->getMixer()->blendCycle(0, animator.pose_interpolation[IDLE], 0);
    cal_model->getMixer()->blendCycle(2, animator.pose_interpolation[WALK], 0);
    cal_model->getMixer()->blendCycle(9, animator.pose_interpolation[JOG], 0);

    model.onUpdate(dt * animator.animation_speed);
  }
}
