#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "glm/glm.hpp"

struct Geometry{
	virtual void setPosition(const glm::vec3& newPos) = 0;
	virtual bool isInside(const glm::vec3& point) = 0;
};

struct GPUModelHandle;

struct Plane : public Geometry {
	glm::vec3 normal;
	float dconst;
  glm::vec3 rotation; // The plane rotation with the implicit normal (1,0,0) assumed in the plane model
  glm::vec3 origin; //A pane point, stored for rendering purposes
  bool visible = true;
	Plane(){};
	~Plane() {};
	Plane(const glm::vec3& point, const glm::vec3& normalVect, const glm::vec3& rotation);
	Plane(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& rotation);

	void setPosition(const glm::vec3& newPos);
	bool isInside(const glm::vec3& point);
	float distPoint2Plane(const glm::vec3& point);
	glm::vec3 closestPointInPlane(const glm::vec3& point);
	bool intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall);
};	

struct Triangle : public Plane {
	glm::vec3 vertex1, vertex2, vertex3;
  Plane plane;
  GPUModelHandle* model;
	Triangle(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2);
	~Triangle() {};
  float area();
	bool isInside(const glm::vec3& point);
	bool intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall);
};

struct Sphere : public Geometry {
	glm::vec3 center;
	float radi;
	Sphere(const glm::vec3& point, const float& radious);
	~Sphere() {};
	void setPosition(const glm::vec3& newPos);
	bool isInside(const glm::vec3& point);
	bool intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall);

  Plane tangentContactPlane(const glm::vec3& point1, const glm::vec3& point2);
};

#endif
