#include "Vector.h"
#include <cmath>

Vector2::Vector2()
{
	
}

bool Vector2::isZero()
{
	return fabs(x) + fabs(y) == 0;
}


Vector2::Vector2(float x, float y)
{
	this->x = x;
	this->y = y;
}

Vector3::Vector3()
{
	
}

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

bool Vector3::isZero()
{
	return fabs(x) + fabs(y) + fabs(z) == 0;
}

