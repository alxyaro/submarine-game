#pragma once
#include <GL/glut.h>
extern "C" {
#include "Vector3D.h"
#include "BoundingBox.h"
}

class Submarine
{
public:
	Submarine(unsigned int, GLUquadricObj*);
	virtual void reset();
	void tick(short powerDirection, short rotationDirection, short verticalDirection, float deltaTime);
	void draw();
	Vector3 getPosition() const;
	virtual void setPosition(float, float, float);
	virtual void rotate(float);
	void setFast();
	BoundingBox getBoundingBox();
	unsigned long int ticksLived;
private:
	unsigned int textureId;
	GLUquadricObj* qobj;
	float propellerRotation = 0;
	Vector3* submarinePosition;
	double submarineRotation = 0;
	
	// some rough physics
	const float propellerAcceleration = 0.8f;
	const float propellerDeceleration = 0.8f;
	float propellerSpeed = 0;
	float terminalPropellerSpeed = 1;
	float terminalVelocity = 4;
	const float waterDrag = 1; // aka deceleration
	float horizontalVelocity = 0;
	float rotationalVelocity = 0;
	float verticalVelocity = 0;

	BoundingBox* boundingBox;
	
	void drawSubBody();
	void drawSubTower();
	void drawFin();
	void drawSubPropeller();
};

class AISubmarine : public Submarine
{
public:
	bool initialized;
	void reset() override;
	void setPosition(float, float, float) override;
	void rotate(float) override;
private:
	short powerDirection;
	short rotationDirection;
	short verticalDirection;
	unsigned int recalcCooldown;
};

