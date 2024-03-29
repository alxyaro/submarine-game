#pragma once
class Vector
{
public:
	virtual bool isZero() = 0; // pure virtual
};

class Vector2 : public Vector
{
public:
	Vector2();
	Vector2(float, float);
	float x;
	float y;
	bool isZero() override;
};

class Vector3 : public Vector
{
public:
	Vector3();
	Vector3(float, float, float);
	float x;
	float y;
	float z;
	bool isZero() override;
	Vector3 copy();
	void mult(float factor);
	void add(Vector3);
	float getLength();
	void normalize();
	float dotProduct(Vector3 other);
	Vector3 crossProduct(Vector3 other);
	float angleXZ(Vector3 other);
	void rotateAboutYAxis(float angle);
	float getAngleToAxis(float x, float y, float z);
	float distanceTo(Vector3);
};

