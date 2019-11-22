#include "BoundingBox.h"
#include <GL/glut.h>
#include <cstdio>
#include <cmath>

#define PI acos(-1.0)

BoundingBox::BoundingBox(Vector3* position, float size)
{
	this->position = position;
	this->offset = new Vector3(0, 0, 0);
	this->size = new Vector3(size, size, size);
}

BoundingBox::BoundingBox(Vector3* position, Vector3* size)
{
	this->position = position;
	this->offset = new Vector3(0, 0, 0);
	this->size = size;
}

BoundingBox::BoundingBox(Vector3* position, Vector3* offset, float size)
{
	this->position = position;
	this->offset = offset;
	this->size = new Vector3(size, size, size);
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

void BoundingBox::rotate(float angle)
{
	if (offset->x != 0 || offset->z != 0)
		offset->rotateAboutYAxis(angle);

	if (child != nullptr)
		child->rotate(angle);
}

bounds BoundingBox::getBounds()
{
	const float x1 = position->x + offset->x - size->x / 2;
	const float y1 = position->y + offset->y - size->y / 2;
	const float z1 = position->z + offset->z - size->z / 2;
	const float x2 = x1 + size->x;
	const float y2 = y1 + size->y;
	const float z2 = z1 + size->z;
	return {x1,y1,z1,x2,y2,z2};
}

float BoundingBox::getLowerY()
{
	return position->y + offset->y;
}

bool BoundingBox::collidesWith(BoundingBox other)
{
	return collidesWith(other, true);
}

bool BoundingBox::collidesWith(BoundingBox other, bool checkOthersChildren)
{
	const bounds b1 = getBounds();
	const bounds b2 = other.getBounds();

	if (b1.x2 < b2.x1 || b2.x2 < b1.x1 ||
		b1.y2 < b2.y1 || b2.y2 < b1.y1 ||
		b1.z2 < b2.z1 || b2.z2 < b1.z1)
	{
		if (this->child != nullptr && this->child->collidesWith(other, false))
			return true;
		if (other.child != nullptr && checkOthersChildren)
			return collidesWith(*other.child, true);
		return false;
	}
	return true;
}

void BoundingBox::debugDraw()
{	
	glDisable(GL_LIGHTING);
	glColor3fv(debugColor);

	const bounds b = getBounds();
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(b.x1, b.y1, b.z1);
	glVertex3f(b.x2, b.y1, b.z1);
	glVertex3f(b.x2, b.y1, b.z2);
	glVertex3f(b.x1, b.y1, b.z2);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(b.x1, b.y2, b.z1);
	glVertex3f(b.x2, b.y2, b.z1);
	glVertex3f(b.x2, b.y2, b.z2);
	glVertex3f(b.x1, b.y2, b.z2);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(b.x1, b.y1, b.z1);
	glVertex3f(b.x1, b.y2, b.z1);
	glVertex3f(b.x2, b.y1, b.z1);
	glVertex3f(b.x2, b.y2, b.z1);
	glVertex3f(b.x2, b.y1, b.z2);
	glVertex3f(b.x2, b.y2, b.z2);
	glVertex3f(b.x1, b.y1, b.z2);
	glVertex3f(b.x1, b.y2, b.z2);
	glEnd();
	
	glEnable(GL_LIGHTING);

	if (child != nullptr)
		child->debugDraw();
}