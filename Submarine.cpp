#include "Vector3D.h"

// TODO add & include header file

class Submarine {
public:
	Vector3D position;
	Submarine()
	{
		position = NewVector3D(0, 0, 0);
	};
	void setPosition(float x, float y, float z)
	{
		position.x = x;
		position.y = y;
		position.z = z;
	}
	void move(float x, float y, float z)
	{
		position.x += x;
		position.y += y;
		position.z += z;
	}
};