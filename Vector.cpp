#include "Vector.h"
#include <cmath>
#include <cstdio>

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

Vector3 Vector3::copy()
{
	return { x,y,z };
}

float Vector3::getLength()
{
	return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}


void Vector3::normalize()
{
	float length = getLength();
	x /= length;
	y /= length;
	z /= length;
}

float Vector3::dotProduct(Vector3 other)
{
	return x * other.x + y * other.y + z * other.z;
}

float Vector3::angleXZ(Vector3 other)
{
	return acosf(dotProduct(other)/(getLength()*other.getLength()));
}

void Vector3::rotateAboutYAxis(float angle)
{
	const float cos = cosf(angle);
	const float sin = sinf(angle);

	const float oldX = x;
	const float oldZ = z;

	x = cos * oldX + sin * oldZ;
	z = -sin * oldX + cos * oldZ;
}

