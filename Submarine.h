#pragma once
#include <GL/glut.h>
extern "C" {
#include "Vector3D.h"
}

class Submarine
{
public:
	Submarine(unsigned int, GLUquadricObj*);
	void reset();
	void tick(short powerDirection, short rotationDirection, short verticalDirection, float deltaTime);
	void draw();
	Vector3D getPosition() const;
private:
	unsigned int textureId;
	GLUquadricObj* qobj;
	float propellerRotation = 0;
	Vector3D submarinePosition;
	double submarineRotation = 0;
	
	// some rough physics
	const float propellerAcceleration = 0.8f;
	const float propellerDeceleration = 0.8f;
	float propellerSpeedPct = 0;
	const float terminalVelocity = 10;
	const float waterDrag = 1; // aka deceleration
	float horizontalVelocity = 0;
	float rotationalVelocity = 0;
	float verticalVelocity = 0;
	
	void drawSubBody();
	void drawSubTower();
	void drawFin();
	void drawSubPropeller();
};

