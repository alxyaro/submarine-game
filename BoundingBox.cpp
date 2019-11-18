#include "BoundingBox.h"
#include <GL/glut.h>
#include <cstdio>
#include <cmath>

#define PI acos(-1.0)

BoundingBox::BoundingBox(Vector3* position, Vector3* size)
{
	this->position = position;
	this->offset = new Vector3(-size->x/2, -size->y/2, -size->z/2);
	this->size = size;
}

BoundingBox::BoundingBox(Vector3* position, Vector3* offset, Vector3* size)
{
	this->position = position;
	this->offset = offset;
	this->size = size;
}

Vector3 BoundingBox::getPosition()
{
	return *position;
}


void BoundingBox::setPosition(float x, float y, float z)
{
	shift(x - position->x, y - position->y, z - position->z);
}

void BoundingBox::shift(float x, float y, float z)
{
	position->x += x;
	position->y += y;
	position->z += z;
	if (child != nullptr)
		child->shift(x, y, z);
}

void BoundingBox::rotate(Vector3* origin, float angle)
{
	const float cos = cosf(angle);
	const float sin = sinf(angle);

	const float xDiff = position->x - origin->x;
	const float zDiff = position->z - origin->z;

	position->x = (cos * xDiff + sin * zDiff) + origin->x;
	position->z = (-sin * xDiff + cos * zDiff) + origin->z;

	if (child != nullptr)
		child->rotate(origin, angle);
}



bool BoundingBox::intersects(BoundingBox other)
{
	const float x1 = position->x;
	const float y1 = position->y;
	const float z1 = position->z;
	const float x2 = position->x + size->x;
	const float y2 = position->y + size->y;
	const float z2 = position->z + size->z;

	const float other_x1 = other.position->x;
	const float other_y1 = other.position->y;
	const float other_z1 = other.position->z;
	const float other_x2 = other.position->x + other.size->x;
	const float other_y2 = other.position->y + other.size->y;
	const float other_z2 = other.position->z + other.size->z;

	if (x2 < other_x1 || other_x2 < x1 ||
		y2 < other_y1 || other_y2 < y1 ||
		z2 < other_z1 || other_z2 < z1)
		return child != nullptr ? child->intersects(other) : false;
	return true;
}

void BoundingBox::debugDraw()
{	
	glDisable(GL_LIGHTING);
	glColor3fv(debugColor);

	float x = position->x + offset->x;
	float y = position->y + offset->y;
	float z = position->z + offset->z;
	float x2 = x + size->x;
	float y2 = y + size->y;
	float z2 = z + size->z;
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, z);
	glVertex3f(x2, y, z);
	glVertex3f(x2, y, z2);
	glVertex3f(x, y, z2);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y2, z);
	glVertex3f(x2, y2, z);
	glVertex3f(x2, y2, z2);
	glVertex3f(x, y2, z2);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(x, y, z);
	glVertex3f(x, y2, z);
	glVertex3f(x2, y, z);
	glVertex3f(x2, y2, z);
	glVertex3f(x2, y, z2);
	glVertex3f(x2, y2, z2);
	glVertex3f(x, y, z2);
	glVertex3f(x, y2, z2);
	glEnd();
	
	glEnable(GL_LIGHTING);

	if (child != nullptr)
		child->debugDraw();
}

float BoundingBox::getLowerY()
{
	return position->y + offset->y;
}

Vector3* BoundingBox::getLowerCorner()
{
	return new Vector3(
		position->x + offset->x,
		position->y + offset->y,
		position->z + offset->z
	);
}

Vector3* BoundingBox::getUpperCorner()
{
	return new Vector3(
		position->x + offset->x + size->x,
		position->y + offset->y + size->y,
		position->z + offset->z + size->z
	);
}
