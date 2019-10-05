#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"

#define PI acos(-1.0)

#define KEY_MOVEMENT_FORWARD 0b00000001
#define KEY_MOVEMENT_BACKWARD 0b00000010
#define KEY_ROTATION_LEFT 0b00000100
#define KEY_ROTATION_RIGHT 0b00001000
#define KEY_MOVEMENT_UP 0b00010000
#define KEY_MOVEMENT_DOWN 0b00100000

int keyMask = 0;

const int viewportWidth = 650;
const int viewportHeight = 500;

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat mat_ambient[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_specular[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_diffuse[] = { 0.8F, 0.8F, 0.8F, 1.0F };
static GLfloat mat_shininess[] = { 0.1F };

const int meshSize = 16;
static QuadMesh groundMesh;

GLUquadricObj* qobj;

float propellerRotation = 0;

double submarinePosition[] = { 0,5,8 };
double submarineRotation = -60;


// some rough physics

const float propellerAcceleration = 0.8f;
const float propellerDeceleration = 0.2f;
float propellerSpeedPct = 0;

const float terminalVelocity = 3;
const float waterDrag = 0.3f; // aka deceleration
float horizontalVelocity = 0;

float rotationalVelocity = 0;
float verticalVelocity = 0;


// prototypes
bool keyDown(int);
void init(int, int);
void resize(int, int);
void display(void);
void drawSubmarine(void);
void drawSubBody(void);
void drawSubTower(void);
void drawFin(void);
void keyboard(unsigned char,int,int);
void keyboardUp(unsigned char, int, int);
void special(int, int, int);
void specialUp(int, int, int);
void mainLoop(void);
void drawSubPropeller(void);

bool keyDown(int key)
{
	return (keyMask & key) != 0;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(viewportWidth, viewportHeight);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Assignment 1");

	init(viewportWidth, viewportHeight);

	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	//glutMouseFunc(mouse);
	//glutMotionFunc(mouseMotionHandler);
	
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	

	mainLoop();
	
	glutMainLoop();

	return 0;
}

void mainLoop()
{
	const float deltaTime = 20 / 1000.0f; // TODO accurately calculate time since last tick

	if (keyDown(KEY_MOVEMENT_FORWARD))
	{
		propellerSpeedPct += propellerAcceleration * deltaTime;
		if (propellerSpeedPct > 1)
			propellerSpeedPct = 1;
	}
	else if (keyDown(KEY_MOVEMENT_BACKWARD))
	{
		propellerSpeedPct -= propellerAcceleration * deltaTime;
		if (propellerSpeedPct < -1)
			propellerSpeedPct = -1;
	}
	else if (propellerSpeedPct != 0)
	{
		const float amount = propellerDeceleration * deltaTime;
		if (propellerSpeedPct > 0)
		{
			propellerSpeedPct -= amount;
			if (propellerSpeedPct < 0)
				propellerSpeedPct = 0;
		} else
		{
			propellerSpeedPct += amount;
			if (propellerSpeedPct > 0)
				propellerSpeedPct = 0;
		}
	}
	propellerRotation = (float)fmod(propellerRotation + propellerSpeedPct * 7.0, 360);

	horizontalVelocity += propellerSpeedPct / 50;
	if (horizontalVelocity > terminalVelocity)
		horizontalVelocity = terminalVelocity;
	if (horizontalVelocity < -terminalVelocity)
		horizontalVelocity = -terminalVelocity;

	// apply drag
	if (horizontalVelocity > 0)
	{
		horizontalVelocity -= waterDrag * deltaTime;
		if (horizontalVelocity < 0)
			horizontalVelocity = 0;
	} else if (horizontalVelocity < 0)
	{
		horizontalVelocity += waterDrag * deltaTime;
		if (horizontalVelocity > 0)
			horizontalVelocity = 0;
	}
	
	const float movementDelta = -1 * horizontalVelocity * deltaTime;

	if (movementDelta != 0)
	{
		const double subRotationRadians = submarineRotation * PI / 180.0;
		submarinePosition[0] += movementDelta * cos(subRotationRadians);
		submarinePosition[2] -= movementDelta * sin(subRotationRadians);
	}

	// rotation
	if (keyDown(KEY_ROTATION_RIGHT))
	{
		if (!keyDown(KEY_ROTATION_LEFT))
		{
			rotationalVelocity = (float) fmin(1.5, rotationalVelocity + 0.3);
		}
	} else if (keyDown(KEY_ROTATION_LEFT))
	{
		rotationalVelocity = (float) fmax(-1.5, rotationalVelocity - 0.3);
	}

	if (rotationalVelocity > 0)
		rotationalVelocity = (float) fmax(0, rotationalVelocity - 0.1);
	else if (rotationalVelocity < 0)
		rotationalVelocity = (float) fmin(0, rotationalVelocity + 0.1);
	
	if (rotationalVelocity != 0)
		submarineRotation = fmod(submarineRotation - rotationalVelocity, 360);
	// end rotation

	// vertical movement
	if (keyDown(KEY_MOVEMENT_UP))
	{
		if (!keyDown(KEY_MOVEMENT_DOWN))
		{
			verticalVelocity = (float)fmin(1.5, verticalVelocity + 0.3);
		}
	}
	else if (keyDown(KEY_MOVEMENT_DOWN))
	{
		verticalVelocity = (float)fmax(-1.5, verticalVelocity - 0.3);
	}

	if (verticalVelocity > 0)
		verticalVelocity = (float)fmax(0, verticalVelocity - 0.1);
	else if (verticalVelocity < 0)
		verticalVelocity = (float)fmin(0, verticalVelocity + 0.1);

	if (verticalVelocity != 0)
		submarinePosition[1] += verticalVelocity/70;
	// end vertical movement
	
	glutPostRedisplay();
	
	glutTimerFunc(20, mainLoop, 0);
}

void keyboard(unsigned char key, int x, int y)
{
	double subRotationRadians = submarineRotation * PI / 180.0;
	switch(key)
	{
	case 'w':
		keyMask |= KEY_MOVEMENT_FORWARD; break;
	case 's':
		keyMask |= KEY_MOVEMENT_BACKWARD; break;
	case 'd':
		keyMask |= KEY_ROTATION_RIGHT; break;
	case 'a':
		keyMask |= KEY_ROTATION_LEFT; break;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		keyMask &= ~KEY_MOVEMENT_FORWARD; break;
	case 's':
		keyMask &= ~KEY_MOVEMENT_BACKWARD; break;
	case 'd':
		keyMask &= ~KEY_ROTATION_RIGHT; break;
	case 'a':
		keyMask &= ~KEY_ROTATION_LEFT; break;
	}
}

void special(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		keyMask |= KEY_MOVEMENT_UP; break;
	case GLUT_KEY_DOWN:
		keyMask |= KEY_MOVEMENT_DOWN; break;
	}
}

void specialUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keyMask &= ~KEY_MOVEMENT_UP; break;
	case GLUT_KEY_DOWN:
		keyMask &= ~KEY_MOVEMENT_DOWN; break;
	}
}

// Set up OpenGL. For viewport and projection setup see reshape(). */
void init(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.0F, 0.0F, 0.0F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	// Set up ground quad mesh
	const Vector3D origin = NewVector3D(-50.0f, 0.0f, 16.0f);
	const Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	const Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh,
		meshSize,
		origin,
		100.0,
		100.0,
		dir1v,
		dir2v);

	SetMaterialQM(&groundMesh, 
		NewVector3D(0.0f, 0.0f, 0.9f), // ambient
		NewVector3D(0.4f, 0.4f, 0.4f), // diffuse
		NewVector3D(0.04f, 0.04f, 0.04f), // specular
		0); // shininess

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);
	
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);

	glEnable(GL_POLYGON_SMOOTH);
}

void resize(int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble) width / height, 0.2, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		0, 10, 20,
		0, 0, 0,
		0, 1, 0
	); // CTM = I * V
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set materials
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	
	// stationary cube
	glPushMatrix();
	glTranslatef(6,1.5,0);
	glutSolidCube(3);
	glPopMatrix();
	// end stationary cube
	
	drawSubmarine();

	DrawMeshQM(&groundMesh, meshSize); // draw ground mesh

	glutSwapBuffers(); // using GLUT_DOUBLE
}

void drawSubmarine()
{
	glPushMatrix(); // begin full model

	// POSITION/ROTATION
	// TEMP: p' = CTM * T(...) * p
	glTranslatef(submarinePosition[0], submarinePosition[1], submarinePosition[2]); // move into position
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
		glRotatef(90*i, 1, 0, 0); // rotate
		// TEMP: p' = CTM * T(...) * R(...) * T(4.8,0,0) * R(_,1,0,0) * T(0,0.3,0) * p
		glTranslatef(0, 0.3, 0); // distance from the submarine
		drawFin();
		glPopMatrix();
	}
	glPopMatrix();

	// PROPELLER
	drawSubPropeller();
	
	glPopMatrix(); // end full model
	
}

void drawSubBody()
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

void drawSubTower()
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

void drawFin()
{
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * p
	glScalef(0.8, 0.5, 0.2);
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * p
	glRotatef(-3,0,0,1);

	glPushMatrix();
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * R(-90,1,0,0) * p
	glRotatef(-90, 1, 0, 0); // rotate cylinder to be upright
	gluCylinder(qobj, 0.5, 0.4, 2, 20, 20);
	glPopMatrix();

	glPushMatrix();
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * T(0,2,0) * p
	glTranslatef(0,2,0);
	// TEMP: p' = CTM * S(0.8,0.5,0.2) * R(-3,0,0,1) * T(0,2,0) * S(1,0.2,1) * p
	glScalef(1, 0.2, 1);
	gluSphere(qobj,0.4,20,20);
	glPopMatrix();
}

void drawSubPropeller()
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
	glTranslatef(length/2, 0, 0); // translate to the center of the propeller ring/shell
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