#pragma once
#include "Vector.h"

struct bounds
{
	float x1;
	float y1;
	float z1;
	float x2;
	float y2;
	float z2;
};

class BoundingBox
{
public:
	BoundingBox(Vector3* position, float size); // centered at position
	BoundingBox(Vector3* position, Vector3* size); // centered at position
	BoundingBox(Vector3* position, Vector3* offset, float size); // centered at position+offset
	BoundingBox(Vector3* position, Vector3* offset, Vector3* size); // centered at position+offset
	Vector3 getPosition();
	void setPosition(float, float, float);
	void shift(float, float, float);
	void rotate(float);
	bool collidesWith(BoundingBox other);
	bool collidesWith(BoundingBox other,bool);
	void debugDraw();
	float getLowerY();
	bounds getBounds();
	BoundingBox* child;
	float debugColor[3] = {1,0,0};
private:
	Vector3* position;
	Vector3* offset;
	Vector3* size;
};