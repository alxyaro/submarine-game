#include "Torpedo.h"
#include <cstdio>
#include <cmath>

#define PI acos(-1.0)

Torpedo::Torpedo(GLUquadric* quadric, Vector3 position, Vector3 direction)
{
	this->quadric = quadric;
	this->boundingBox = new BoundingBox(new Vector3(position.x, position.y, position.z), 0.5f);
	this->direction = new Vector3(direction.x, direction.y, direction.z);
	this->direction->normalize();
}

void Torpedo::draw()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glPushMatrix();
	glTranslatef(boundingBox->getPosition().x, boundingBox->getPosition().y, boundingBox->getPosition().z);

	float angle = direction->getAngleToAxis(0,0,-1) * 180.0 / PI;
	Vector3 normal = Vector3{ 0,0,-1 }.crossProduct(*direction);
	glRotatef(angle, normal.x, normal.y, normal.z);
	
	glScalef(0.2, 0.2, 0.2);
	glTranslatef(0, 0, 0);
	gluCylinder(quadric, 1, 1, 12, 20, 20);
	glPushMatrix();
	glTranslatef(0, 0, 12);
	glScalef(1, 1, 3);
	gluSphere(quadric, 1, 20, 20);
	glPopMatrix();
	glScalef(1, 1, 1.5);
	gluSphere(quadric, 1, 20, 20);
	glPopMatrix();
}

void Torpedo::move(float deltaTime)
{
	this->boundingBox->shift(direction->x * speed * deltaTime, direction->y * speed * deltaTime, direction->z * speed * deltaTime);
}

void Torpedo::goTowards(Vector3 position)
{
	Vector3 targetDirection(position.x - boundingBox->getPosition().x, position.y - boundingBox->getPosition().y, position.z - boundingBox->getPosition().z);
	targetDirection.normalize();

	Vector3 directionChange = { targetDirection.x - direction->x, targetDirection.y - direction->y, targetDirection.z - direction->z };
	directionChange.normalize();
	/*float distance = boundingBox->getPosition().distanceTo(position);
	distance /= 25.0f;
	if (distance < 1)
		distance = 1;*/
	directionChange.mult(0.01f);

	direction->add(directionChange);
	direction->normalize();
}
