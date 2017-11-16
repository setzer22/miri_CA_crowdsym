#pragma once
#include "Geometry.h"

#include <stdio.h>
#include <cmath>

//****************************************************
// Plane
//****************************************************
Plane::Plane(const glm::vec3& point, const glm::vec3& normalVect, const glm::vec3& rotation){
	normal = glm::normalize(normalVect);
	dconst = -glm::dot(point, normal);
  this->rotation = rotation;
  this->origin = point;
};

Plane::Plane(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& rotation){
	glm::vec3 v1 = point1 - point0;
	glm::vec3 v2 = point2 - point0;
	normal = glm::normalize(glm::cross(v1, v2));
	dconst = -glm::dot(point0, normal);
  this->rotation = rotation;
  origin = (point0 + point1 + point2) * (1/3.0f);
};

void Plane::setPosition(const glm::vec3& newPos){
	dconst = -glm::dot(newPos, normal);
};

bool Plane::isInside(const glm::vec3& point){
	float dist;
	dist = glm::dot(point, normal) + dconst;
	if (dist > 1.e-7)
		return false;
	else
		return true;
};

float Plane::distPoint2Plane(const glm::vec3& point){
	float dist;
	return dist = glm::dot(point, normal) + dconst;
};

glm::vec3 Plane::closestPointInPlane(const glm::vec3& point){
	glm::vec3 closestP;
	float r = (-dconst - glm::dot(point, normal));
	return closestP = point + r*normal;
};

bool Plane::intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall){
	if (distPoint2Plane(point1)*distPoint2Plane(point2) > 0)	return false;
	float r = (-dconst - glm::dot(point1, normal)) / glm::dot((point2 - point1), normal);
	pTall = (1 - r)*point1 + r*point2;
	return true;
};


//****************************************************
// Triangle
//****************************************************

glm::vec3 vertex1, vertex2, vertex3;
Triangle::Triangle(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2) {
  this->vertex1 = point0;
  this->vertex2 = point1;
  this->vertex3 = point2;
  this->plane = Plane(point0, point1, point2, glm::vec3(0,0,0));
}
float Triangle::area() {
  return abs((1.0f/2.0f) * glm::length(glm::cross(vertex2 - vertex1, vertex3 - vertex1)));
}
bool Triangle::isInside(const glm::vec3& point) {
  glm::vec3 p = plane.closestPointInPlane(point);
  float a1 = Triangle(p, vertex2, vertex3).area();
  float a2 = Triangle(vertex1, p, vertex3).area();
  float a3 = Triangle(vertex1, vertex2, p).area();

  float val = a1+a2+a3-area();

  return a1+a2+a3-area() < 0.1;
}
bool Triangle::intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall) {
  //NOTE: We don't need this anywhere.
  return false;
}

//****************************************************
// Sphere
//****************************************************
Sphere::Sphere(const glm::vec3& point, const float& radious) {
  this->center = point;
  this->radi = radious;
}

void Sphere::setPosition(const glm::vec3& newPos) {
  this->center = newPos;
}

bool Sphere::isInside(const glm::vec3& point) {
  return glm::distance(point, this->center) < this->radi;
}

bool Sphere::intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall) {
  glm::vec3 v = point2 - point1;
  float a = glm::dot(v,v);
  float b = glm::dot(2.0f*v, point1 - this->center);
  float c = glm::dot(this->center, this->center) + glm::dot(point1, point1) - glm::dot(2.0f*point1, this->center) - this->radi*this->radi;

  float discriminant = b*b- 4*a*c;
  float lambda = 0;

  if (discriminant < 0 || a == 0) {
    return false;
  }
  else if (discriminant == 0) {
    lambda = -b / (2*a);
  }
  else {
    float lambda1 = (-b + sqrt(discriminant)) / (2*a);
    float lambda2 = (-b - sqrt(discriminant)) / (2*a);
    lambda = lambda1 > lambda2 ? lambda2 : lambda1;
  }

  if(lambda < 0 || lambda > 1) {
    printf("%f\n", lambda);
  }
  //assert(lambda >= 0 && lambda <= 1);
  pTall = point1 + v*lambda;
  return true;
}

Plane Sphere::tangentContactPlane(const glm::vec3& point1, const glm::vec3& point2) {
  glm::vec3 plane_point;
  intersecSegment(point1, point2, plane_point);
  glm::vec3 plane_normal = glm::normalize(plane_point - this->center);
  return Plane(plane_point, plane_normal, glm::vec3(0,0,0));
}

