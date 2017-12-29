#include "Particle.h"
#include <stdio.h>


Particle::Particle()
{
}

Particle::Particle(const float& x, const float& y, const float& z) :
  m_previousPosition(0, 0, 0), m_force(0, 0, 0), m_velocity(0, 0, 0), m_bouncing(1), m_lifetime(50), m_mass(1), m_fixed(false)
{
  m_previousPosition.x = x;
  m_previousPosition.y = y;
  m_previousPosition.z = z;
	m_currentPosition.x = x;
	m_currentPosition.y = y;
	m_currentPosition.z = z;

}

/*
Particle::Particle(glm::vec3 pos, glm::vec3 vel, float bouncing, bool fixed, int lifetime, glm::vec3 force) :
m_currentPosition(pos), m_previousPosition(pos), m_force(force), m_velocity(vel), m_bouncing(bouncing), m_lifetime(lifetime), m_fixed(fixed)
{
}
*/

Particle::~Particle()
{
}

//setters
void Particle::setPosition(const float& x, const float& y, const float& z)
{
	glm::vec3 pos(x,y,z);
	m_currentPosition =  pos;
}
void Particle::setPosition(glm::vec3 pos)
{
	m_currentPosition = pos;
}

void Particle::setPreviousPosition(const float& x, const float& y, const float& z)
{
	glm::vec3 pos(x, y, z);
	m_previousPosition = pos;
}

void Particle::setPreviousPosition(glm::vec3 pos)
{
	m_previousPosition = pos;
}

void Particle::setForce(const float& x, const float& y, const float& z)
{
	glm::vec3 force(x, y, z);
	m_force = force;
}

void Particle::setForce(glm::vec3 force)
{
	m_force = force;
}

void Particle::addForce(const float& x, const float& y, const float& z)
{
	glm::vec3 force(x,y,z);
	m_force += force;
}

void Particle::addForce(glm::vec3 force)
{
	m_force += force;
}

void Particle::setVelocity(const float& x, const float& y, const float& z)
{
	glm::vec3 vel(x,y,z);
	m_velocity = vel;
}

void Particle::setVelocity(glm::vec3 vel)
{
	m_velocity = vel;
}

void Particle::setBouncing(float bouncing)
{
	m_bouncing = bouncing;
}

void Particle::setLifetime(float lifetime)
{
	m_lifetime = lifetime;
}

void Particle::setMass(float mass)
{
  m_mass = mass;
}

void Particle::setFixed(bool fixed)
{
	m_fixed = fixed;
}

void Particle::setRadius(float radius)
{
  m_radius = radius;
}

//getters
glm::vec3 Particle::getCurrentPosition() const
{
	return m_currentPosition;
}

glm::vec3 Particle::getPreviousPosition() const
{
	return m_previousPosition;
}

glm::vec3 Particle::getForce() const
{
	return m_force;
}

glm::vec3 Particle::getVelocity() const
{
	return m_velocity;
}

float Particle::getBouncing() const
{
	return m_bouncing;
}

float Particle::getLifetime() const
{
	return m_lifetime;
}

float Particle::getMass() const
{
	return m_mass;
}
float Particle::getRadius() const
{
	return m_radius;
}

bool Particle::isFixed() const
{
	return m_fixed;
}

void Particle::updateParticle(const float& dt, UpdateMethod method)
{
	if (!m_fixed & m_lifetime > 0)
	{
		switch (method)
		{
		case UpdateMethod::EulerOrig:
		{
			m_previousPosition = m_currentPosition;
			m_currentPosition += m_velocity*dt;
			m_velocity += (1.0f/m_mass)*m_force*dt;
		}
			break;
		case UpdateMethod::EulerSemi:
		{
			m_previousPosition = m_currentPosition;
      //printf("vel: %f, %f, %f\n", m_velocity.x, m_velocity.y, m_velocity.z);
			m_velocity += dt*m_force/m_mass;
      //printf("pos: %f, %f, %f\n", m_currentPosition.x, m_currentPosition.y, m_currentPosition.z);
			m_currentPosition += m_velocity*dt;
		}
			break;
		case UpdateMethod::Verlet:
		{
      float k = 1; //??
      glm::vec3 new_pos = m_currentPosition + k * (m_currentPosition - m_previousPosition) +
        (1.0f/m_mass) * m_force * dt*dt;
      m_velocity = (new_pos - m_currentPosition)/dt;
      m_previousPosition = m_currentPosition;
      m_currentPosition = new_pos;
		}
			break;
		}
	}
	return;
}
