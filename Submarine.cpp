#include "Submarine.h"
#include <cmath>

#define PI acos(-1.0)

Submarine::Submarine(unsigned int textureId, GLUquadricObj* qobj)
{
	this->textureId = textureId;
	this->qobj = qobj;
	this->submarinePosition = new Vector3(0, 0, 0);
}

void Submarine::reset()
{
	submarinePosition->x = 0;
	submarinePosition->y = 0;
	submarinePosition->z = 0;
	
	this->boundingBox = new BoundingBox(new Vector3(0, 0, 0), 1.4);
	BoundingBox* lastBB = boundingBox;
	for (int i = 0; i < 4; i++)
	{
		BoundingBox* newBB = new BoundingBox(new Vector3(0, 0, 0), new Vector3(-i * 1.5, 0, 0), 1.4);
		lastBB->child = newBB;
		lastBB = newBB;
	}
	for (int i = 0; i < 4; i++)
	{
		BoundingBox* newBB = new BoundingBox(new Vector3(0, 0, 0), new Vector3(i * 1.6, 0, 0), 1.4);
		lastBB->child = newBB;
		lastBB = newBB;
	}

	setPosition(0, 15, 0);
	
	submarineRotation = 0;
	rotate(-90);

	propellerSpeed = 0;
	horizontalVelocity = 0;
	rotationalVelocity = 0;
	verticalVelocity = 0;

	ticksLived = 0;
}

void Submarine::tick(short powerDirection, short rotationDirection, short verticalDirection, float deltaTime)
{
	ticksLived++;
	if (powerDirection == 1)
	{
		propellerSpeed += propellerAcceleration * deltaTime;
		if (propellerSpeed > terminalPropellerSpeed)
			propellerSpeed = terminalPropellerSpeed;
	}
	else if (powerDirection == -1)
	{
		propellerSpeed -= propellerAcceleration * deltaTime;
		if (propellerSpeed < -terminalPropellerSpeed)
			propellerSpeed = -terminalPropellerSpeed;
	}
	else if (propellerSpeed != 0)
	{
		const float amount = propellerDeceleration * deltaTime;
		if (propellerSpeed > 0)
		{
			propellerSpeed -= amount;
			if (propellerSpeed < 0)
				propellerSpeed = 0;
		}
		else
		{
			propellerSpeed += amount;
			if (propellerSpeed > 0)
				propellerSpeed = 0;
		}
	}

	propellerRotation = static_cast<float>(fmod(propellerRotation + propellerSpeed * 7.0, 360));

	horizontalVelocity += propellerSpeed/5;
	if (horizontalVelocity > terminalVelocity)
		horizontalVelocity = terminalVelocity;
	if (horizontalVelocity < -terminalVelocity)
		horizontalVelocity = -terminalVelocity;

	// apply drag
	if (horizontalVelocity > 0)
		horizontalVelocity = static_cast<float>(fmax(0, horizontalVelocity - waterDrag * deltaTime));
	else if (horizontalVelocity < 0)
		horizontalVelocity = static_cast<float>(fmin(0, horizontalVelocity + waterDrag * deltaTime));

	// rotation
	if (rotationDirection == 1)
	{
		if (rotationDirection != -1)
		{
			rotationalVelocity = static_cast<float>(fmin(1.5, rotationalVelocity + 0.3));
		}
	}
	else if (rotationDirection == -1)
	{
		rotationalVelocity = static_cast<float>(fmax(-1.5, rotationalVelocity - 0.3));
	}

	if (rotationalVelocity > 0)
		rotationalVelocity = static_cast<float>(fmax(0, rotationalVelocity - 0.1));
	else if (rotationalVelocity < 0)
		rotationalVelocity = static_cast<float>(fmin(0, rotationalVelocity + 0.1));

	if (rotationalVelocity != 0)
	{
		rotate(-rotationalVelocity);
	}
	// end rotation

	// vertical movement
	if (verticalDirection == 1)
	{
		if (verticalDirection != -1)
		{
			verticalVelocity = static_cast<float>(fmin(4.5, verticalVelocity + 0.5));
		}
	}
	else if (verticalDirection == -1)
	{
		verticalVelocity = static_cast<float>(fmax(-4.5, verticalVelocity - 0.5));
	}

	if (verticalVelocity > 0)
		verticalVelocity = static_cast<float>(fmax(0, verticalVelocity - 0.1));
	else if (verticalVelocity < 0)
		verticalVelocity = static_cast<float>(fmin(0, verticalVelocity + 0.1));

	// end vertical movement

	Vector3 velocity = getVelocity(deltaTime);
	if (!velocity.isZero())
	{
		submarinePosition->x += velocity.x;
		submarinePosition->y += velocity.y;
		submarinePosition->z += velocity.z;
	}


	syncBb();
}


Vector3 Submarine::getPosition() const
{
	return *submarinePosition;
}

Vector3 Submarine::getVelocity(float deltaTime) const
{
	const float subRotationRadians = submarineRotation * PI / 180.0;
	return {
		-1 * horizontalVelocity * cos(subRotationRadians) * deltaTime,
		verticalVelocity * 0.7f * deltaTime,
		horizontalVelocity * sin(subRotationRadians) * deltaTime
	};
}


void Submarine::setPosition(float x, float y, float z)
{
	submarinePosition->x = x;
	submarinePosition->y = y;
	submarinePosition->z = z;
	syncBb();
}

void Submarine::syncBb()
{
	boundingBox->setPosition(submarinePosition->x, submarinePosition->y, submarinePosition->z);
}


void Submarine::rotate(float angleDeg)
{
	submarineRotation = fmod(submarineRotation + angleDeg, 360);
	boundingBox->rotate(angleDeg * PI / 180);
}


void Submarine::setFast()
{
	terminalPropellerSpeed = 3;
	terminalVelocity = 10;
}

BoundingBox Submarine::getBoundingBox()
{
	return *boundingBox;
}

AISubmarine::AISubmarine(unsigned int textureId, GLUquadricObj* qobj) : Submarine(textureId, qobj){}

void AISubmarine::reset()
{
	initialized = false;
	forwardViewBb = new BoundingBox(new Vector3(0, 0, 0), new Vector3(-9, 0, 0), 6);
	forwardViewBb->debugColor[0] = 0;
	forwardViewBb->debugColor[2] = 1;
	Submarine::reset();
}

void AISubmarine::syncBb()
{
	Submarine::syncBb();
	forwardViewBb->setPosition(submarinePosition->x, submarinePosition->y, submarinePosition->z);
}

void AISubmarine::rotate(float angleDeg)
{
	Submarine::rotate(angleDeg);
	forwardViewBb->rotate(angleDeg * PI / 180);
}







// DRAW METHODS


void Submarine::draw()
{
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPushMatrix(); // begin full model

	// POSITION/ROTATION
	// TEMP: p' = CTM * T(...) * p
	glTranslatef(submarinePosition->x, submarinePosition->y, submarinePosition->z); // move into position
	// TEMP: p' = CTM * T(...) * R(...) * p
	glRotatef(submarineRotation, 0, 1, 0); // rotate

	// MAIN BODY
	drawSubBody();

	// TOWER
	drawSubTower();

	// SIDE FINS
	glPushMatrix();
	// TEMP: p' = CTM * T(...) * R(...) * T(-3.4,0,0) * p
	glTranslatef(-3.4, 0, 0); // move closer to the front of the sub
	glPushMatrix();
	// TEMP: p' = CTM * T(...) * R(...) * T(-3.4,0,0) * R(90,1,0,0) * p
	glRotatef(90, 1, 0, 0); // rotate to the right
	// TEMP: p' = CTM * T(...) * R(...) * T(-3.4,0,0) * R(90,1,0,0) * T(0,0.8,0) p
	glTranslatef(0, 0.8, 0); // distance from the submarine
	drawFin(); // right fin
	glPopMatrix();
	glPushMatrix();
	// TEMP: p' = CTM * T(...) * R(...) * T(-3.4,0,0) * R(-90,1,0,0) * p
	glRotatef(-90, 1, 0, 0); // rotate to the left
	// TEMP: p' = CTM * T(...) * R(...) * T(-3.4,0,0) * R(-90,1,0,0) * T(0,0.8,0) p
	glTranslatef(0, 0.8, 0); // distance from the submarine
	drawFin(); // left fin
	glPopMatrix();
	glPopMatrix();

	// BACK FINS
	glPushMatrix();
	// TEMP: p' = CTM * T(...) * R(...) * T(4.8,0,0) * p
	glTranslatef(4.8, 0, 0); // move to the back of the sub
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		// TEMP: p' = CTM * T(...) * R(...) * T(4.8,0,0) * R(_,1,0,0) * p
		glRotatef(90 * i, 1, 0, 0); // rotate
		// TEMP: p' = CTM * T(...) * R(...) * T(4.8,0,0) * R(_,1,0,0) * T(0,0.3,0) * p
		glTranslatef(0, 0.3, 0); // distance from the submarine
		drawFin();
		glPopMatrix();
	}
	glPopMatrix();

	// PROPELLER
	drawSubPropeller();

	glPopMatrix(); // end full model

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

void Submarine::drawSubBody()
{
	const double baseLength = 8;
	glPushMatrix();

	// TEMP: p' = CTM * T(-baseLength/2,0,0) * p
	glTranslatef(-baseLength / 2, 0, 0); // transition to the front of the sub

	glPushMatrix(); // main body
	// TEMP: p' = CTM * T(-baseLength/2,0,0) * R(90,0,1,0) * p
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj,
		1,
		1,
		baseLength - 2,
		20,
		20);
	glPopMatrix(); // end main body

	glPushMatrix(); // front sphere
	// TEMP: p' = CTM * T(-baseLength/2,0,0) * S(1.5,1,1) * p
	glScalef(1.5f, 1, 1);
	gluSphere(qobj, 1, 20, 20);
	glPopMatrix(); // end front sphere

	glPopMatrix();

	glPushMatrix(); // back sphere
	// TEMP: p' = CTM * T(2,0,0) * p
	glTranslatef(2, 0, 0);
	// TEMP: p' = CTM * T(2,0,0) * S(3.5,1,1) * p
	glScalef(3.5f, 1, 1);
	gluSphere(qobj, 1, 30, 30);
	glPopMatrix(); // end back sphere
}

void Submarine::drawSubTower()
{
	glPushMatrix();
	// TEMP: p' = CTM * T(-2,0.8,0) * p
	glTranslatef(-2, 0.8, 0); // move relatively to the top-front of the submarine

	// extra - warping the tower a bit
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * p
	glRotatef(-25, 0, 0, 1); // rotate
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * p
	glScalef(1, 1.2, 1);
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * R(20,0,0,1) * p
	glRotatef(20, 0, 0, 1);

	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * R(20,0,0,1) * S(1,1,0.5) * p
	glScalef(1, 1, 0.5); // flatten the combined cylinder/sphere

	// the body of the tower
	glPushMatrix();
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * R(20,0,0,1) * S(1,1,0.5) * R(-90,1,0,0) * p
	glRotatef(-90, 1, 0, 0); // rotate cylinder upwards
	gluCylinder(qobj, 0.5, 0.5, 0.8, 20, 20);
	glPopMatrix();

	// the top part (hat) of the tower
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * R(20,0,0,1) * S(1,1,0.5) * T(0,0.8,0) * p
	glTranslatef(0, 0.8, 0);
	// TEMP: p' = CTM * T(-2,0.8,0) * R(-25,0,0,1) * S(1,1.2,1) * R(20,0,0,1) * S(1,1,0.5) * T(0,0.8,0) * S(1,0.3,1) * p
	glScalef(1, 0.3, 1);
	gluSphere(qobj, 0.5, 20, 20);
	glPopMatrix();
}

void Submarine::drawFin()
{
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * p
	glScalef(0.8, 0.5, 0.2);
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * p
	glRotatef(-3, 0, 0, 1);

	glPushMatrix();
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * R(-90,1,0,0) * p
	glRotatef(-90, 1, 0, 0); // rotate cylinder to be upright
	gluCylinder(qobj, 0.5, 0.4, 2, 20, 20);
	glPopMatrix();

	glPushMatrix();
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * T(0,2,0) * p
	glTranslatef(0, 2, 0);
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * T(0,2,0) * S(1,0.2,1) * p
	glScalef(1, 0.2, 1);
	gluSphere(qobj, 0.4, 20, 20);
	glPopMatrix();
}

void Submarine::drawSubPropeller()
{
	const double radius = 0.52, radiusEnd = 0.45, length = 0.4, thickness = 0.07, thicknessEnd = 0.04;
	glPushMatrix();

	// TEMP: p' = CTM * T(5.4,0,0) * p
	glTranslatef(5.4, 0, 0); // move to the back of the sub

	glPushMatrix(); // ring

	// TEMP: p' = CTM * T(5.4,0,0) * R(90,0,1,0) * p
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj, radius, radiusEnd, length, 20, 20);
	gluCylinder(qobj, radius - thickness, radiusEnd - thicknessEnd, length, 20, 20);

	glutSolidTorus(thickness / 2, radius - thickness / 2, 20, 20);

	glPushMatrix();
	// TEMP: p' = CTM * T(5.4,0,0) * R(90,0,1,0) * T(0,0,length) * p
	glTranslatef(0, 0, length);
	glutSolidTorus(thicknessEnd / 2, radiusEnd - thicknessEnd / 2, 20, 20);
	glPopMatrix();

	glPopMatrix(); // end ring

	glPushMatrix(); // propeller

	glPushMatrix(); // propeller rod
	// TEMP: p' = CTM * T(5.4,0,0) * T(-0.1,0,0) * p
	glTranslatef(-0.1, 0, 0);
	// TEMP: p' = CTM * T(5.4,0,0) * T(-0.1,0,0) * R(90,0,1,0) * p
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj, 0.06, 0.06, 0.5, 10, 10);
	glPopMatrix(); // end propeller rod

	// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * p
	glTranslatef(length / 2, 0, 0); // translate to the center of the propeller ring/shell
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * p
		glRotatef(i * 90 - propellerRotation, 1, 0, 0); // rotate the blade
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * R(-30,0,1,0) * p
		glRotatef(-30, 0, 1, 0); // angle the blade outwards a bit
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * R(-30,0,1,0) * S(0.15) * p
		glScalef(0.15, 0.15, 0.15); // scale down the whole thing
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * R(-30,0,1,0) * S(0.15) * S(1,1,0.3) * p
		glScalef(1, 1, 0.3); // make the shapes thin - like a propeller blade

		glPushMatrix();
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * R(-30,0,1,0) * S(0.15) * S(1,1,0.3) * R(-90,1,0,1) * p
		glRotatef(-90, 1, 0, 0);
		gluCylinder(qobj, 0.5, 0.5, 1.5, 15, 15);
		glPopMatrix();

		glTranslatef(0, 1.2, 0);
		// TEMP: p' = CTM * T(5.4,0,0) * T(length/2,0,0) * R(_,1,0,1) * R(-30,0,1,0) * S(0.15) * S(1,1,0.3) * S(1,2.5,1) * p
		glScalef(1, 2.5, 1);
		gluSphere(qobj, 0.5, 15, 15);
		glPopMatrix();
	}

	glPopMatrix(); // end propeller

	glPopMatrix();
}