#pragma once
#include "BoundingBox.h"
#include "GL/glut.h"

class Torpedo
{
public:
	Torpedo(GLUquadric* quadric, Vector3 position, Vector3 direction);
	void draw();
	void move(float deltaTime);
	void goTowards(Vector3 position);
	GLUquadric* quadric;
	BoundingBox* boundingBox;
	Vector3* direction;
	float speed = 30;
};

