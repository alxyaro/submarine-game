#pragma once
#include "Vector.h"

class BoundingBox
{
public:
	BoundingBox(Vector3* position, Vector3* offset, Vector3* size);
	Vector3 getPosition();
	void setPosition(float, float, float);
	void shift(float, float, float);
	void rotate(Vector3*, float);
	bool intersects(BoundingBox other);
	void debugDraw();
	float getLowerY();
	Vector3* getLowerCorner();
	Vector3* getUpperCorner();
	BoundingBox* child;
private:
	Vector3* position;
	Vector3* offset;
	Vector3* size;
};
