#pragma once
#include "Vector.h"

class BoundingBox
{
public:
	BoundingBox(Vector3* position, Vector3* offset, Vector3* size);
	void setPosition(float, float, float);
	void shift(float, float, float);
	void rotate(Vector3*, float);
	bool intersects(BoundingBox other);
	void debugDraw();
	BoundingBox* child;
private:
	Vector3* position;
	Vector3* offset;
	Vector3* size;
};
