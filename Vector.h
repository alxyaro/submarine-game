#pragma once
class Vector
{
};

class Vector2 : public Vector
{
public:
	Vector2();
	Vector2(float, float);
	float x;
	float y;
};

class Vector3 : public Vector
{
public:
	Vector3();
	Vector3(float, float, float);
	float x;
	float y;
	float z;
	void rotateAboutYAxis(Vector2 origin, float angle);
};

