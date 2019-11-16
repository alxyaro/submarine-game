extern "C"
{
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include "perlin.h"
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI acos(-1.0)

class Submarine {
public:
	unsigned int textureId;
	GLUquadricObj* qobj;

	Submarine()
	{
		reset();
	}
	Vector3D getPosition()
	{
		return NewVector3D(submarinePosition[0], submarinePosition[1], submarinePosition[2]);
	}
	void reset()
	{
		submarinePosition[0] = 0;
		submarinePosition[1] = 15;
		submarinePosition[2] = 0;

		submarineRotation = 0;

		propellerSpeedPct = 0;
		horizontalVelocity = 0;
		rotationalVelocity = 0;
		verticalVelocity = 0;

	}
	void tick(int powerDirection, int rotationDirection, int verticalDirection, const float deltaTime)
	{
		if (powerDirection == 1)
		{
			propellerSpeedPct += propellerAcceleration * deltaTime;
			if (propellerSpeedPct > 1)
				propellerSpeedPct = 1;
		}
		else if (powerDirection == -1)
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
			}
			else
			{
				propellerSpeedPct += amount;
				if (propellerSpeedPct > 0)
					propellerSpeedPct = 0;
			}
		}

		propellerRotation = (float)fmod(propellerRotation + propellerSpeedPct * 7.0, 360);

		horizontalVelocity += propellerSpeedPct;
		if (horizontalVelocity > terminalVelocity)
			horizontalVelocity = terminalVelocity;
		if (horizontalVelocity < -terminalVelocity)
			horizontalVelocity = -terminalVelocity;

		// apply drag
		if (horizontalVelocity > 0)
			horizontalVelocity = (float)fmax(0, horizontalVelocity - waterDrag * deltaTime);
		else if (horizontalVelocity < 0)
			horizontalVelocity = (float)fmin(0, horizontalVelocity + waterDrag * deltaTime);

		const float movementDelta = -1 * horizontalVelocity * deltaTime;

		if (movementDelta != 0)
		{
			const double subRotationRadians = submarineRotation * PI / 180.0;
			submarinePosition[0] += movementDelta * cos(subRotationRadians);
			submarinePosition[2] -= movementDelta * sin(subRotationRadians);
		}

		// rotation
		if (rotationDirection == 1)
		{
			if (rotationDirection != -1)
			{
				rotationalVelocity = (float)fmin(1.5, rotationalVelocity + 0.3);
			}
		}
		else if (rotationDirection == -1)
		{
			rotationalVelocity = (float)fmax(-1.5, rotationalVelocity - 0.3);
		}

		if (rotationalVelocity > 0)
			rotationalVelocity = (float)fmax(0, rotationalVelocity - 0.1);
		else if (rotationalVelocity < 0)
			rotationalVelocity = (float)fmin(0, rotationalVelocity + 0.1);

		if (rotationalVelocity != 0)
			submarineRotation = fmod(submarineRotation - rotationalVelocity, 360);
		// end rotation

		// vertical movement
		if (verticalDirection == 1)
		{
			if (verticalDirection != -1)
			{
				verticalVelocity = (float)fmin(1.5, verticalVelocity + 0.3);
			}
		}
		else if (verticalDirection == -1)
		{
			verticalVelocity = (float)fmax(-1.5, verticalVelocity - 0.3);
		}

		if (verticalVelocity > 0)
			verticalVelocity = (float)fmax(0, verticalVelocity - 0.1);
		else if (verticalVelocity < 0)
			verticalVelocity = (float)fmin(0, verticalVelocity + 0.1);

		if (verticalVelocity != 0)
			submarinePosition[1] += verticalVelocity / 70;
		// end vertical movement
	}
	void draw()
	{
		drawSubmarine();
	}
private:
	float propellerRotation = 0;

	double submarinePosition[3];
	double submarineRotation;


	// some rough physics

	const float propellerAcceleration = 0.8f;
	const float propellerDeceleration = 0.8f;
	float propellerSpeedPct = 0;

	const float terminalVelocity = 10;
	const float waterDrag = 1; // aka deceleration
	float horizontalVelocity = 0;

	float rotationalVelocity = 0;
	float verticalVelocity = 0;


	void drawSubmarine()
	{
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		//glBindTexture(GL_TEXTURE_2D, textureId);

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
};

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
static GLfloat light_position0[] = { -6.0F, 300.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat mat_ambient[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_specular[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_diffuse[] = { 0.8F, 0.8F, 0.8F, 1.0F };
static GLfloat mat_shininess[] = { 0.1F };

const int meshSize = 100;
static QuadMesh groundMesh;
Vector3D meshOrigin;

Submarine submarine;

// prototypes
bool keyDown(int);
void reset(void);
void init(int, int);
void resize(int, int);
void updateCamera(void);
void display(void);
void keyboard(unsigned char,int,int);
void keyboardUp(unsigned char, int, int);
void special(int, int, int);
void specialUp(int, int, int);
void mainLoop(int);

bool keyDown(int key)
{
	return (keyMask & key) != 0;
}

void reset(void)
{
	submarine.reset();
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

	reset();

	mainLoop(0);
	
	glutMainLoop();

	return 0;
}

void mainLoop(int x)
{
	const float deltaTime = 20 / 1000.0f; // TODO accurately calculate time since last tick

	int power = keyDown(KEY_MOVEMENT_FORWARD) ? 1 : 0;
	if (keyDown(KEY_MOVEMENT_BACKWARD))
		power -= 1;

	int rotation = keyDown(KEY_ROTATION_RIGHT) ? 1 : 0;
	if (keyDown(KEY_ROTATION_LEFT))
		rotation -= 1;

	int vertical = keyDown(KEY_MOVEMENT_UP) ? 1 : 0;
	if (keyDown(KEY_MOVEMENT_DOWN))
		vertical -= 1;

	submarine.tick(power, rotation, vertical, deltaTime);

	updateCamera();
	
	glutPostRedisplay();
	
	glutTimerFunc(20, mainLoop, 0);
}

void keyboard(unsigned char key, int x, int y)
{
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
	case 'r':
		reset(); break;
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
	case GLUT_KEY_F1:
		printf("=============== Tutorial ===============\n");
		printf("Submarine Controls:\n");
		printf(" - hold w to move forward (start propeller)\n");
		printf(" - hold s to move backward (reverse propeller)\n");
		printf(" - hold d to rotate clockwise\n");
		printf(" - hold a to rotate counter-clockwise\n");
		printf(" - hold [up arrow] to move upwards\n");
		printf(" - hold [down arrow] to move downwards\n");
		printf(" - press r to reset submarine/movement\n");
		printf("========================================\n");
		break;
	}
}

unsigned int tempTexture;
// Set up OpenGL. For viewport and projection setup see reshape(). */
void init(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	//glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0F);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidden surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.0F, 0.0F, 0.0F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glEnable(GL_POLYGON_SMOOTH);

	// Set up ground quad mesh
	meshOrigin = NewVector3D(-100.0f, 0.0f, 100.0f);
	const Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	const Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh,
		meshSize,
		meshOrigin,
		200.0,
		200.0,
		dir1v,
		dir2v);

	SetMaterialQM(&groundMesh,
		NewVector3D(0.05f, 0.05f, 0.05f), // ambient
		NewVector3D(0.1f, 0.6f, 0.1f), // diffuse
		NewVector3D(0.04f, 0.04f, 0.04f), // specular
		0.5f); // shininess


	// add terrain variation

	typedef struct blob {
		float x;
		float z;
		float height;
		float radius;
	} blob;

	blob blobsList[40];

	srand(4169671111);

	for (int i = 0; i < 40; i++)
	{
		blob newBlob = { (rand() % 200) - 100, (rand() % 200) - 100, (rand() % 15) - 5, (rand() % 30) + 5 };
		blobsList[i] = newBlob;
	}
	
	int currentVertex = 0;
	for (int i = 0; i < meshSize + 1; i++)
	{
		for (int j = 0; j < meshSize + 1; j++)
		{
			Vector3D* position = &groundMesh.vertices[currentVertex].position;
			float x = position->x+10000;
			float z = position->z+10000;
			position->y = perlin2d(x, z, 0.5f, 4) + perlin2d(x / 20.0f, z / 20.0f, 0.8f, 4) * 10;

			for (unsigned int i = 0; i < sizeof(blobsList)/sizeof(blob); i++)
			{
				blob *blob = &blobsList[i];
				const float distanceToBlob = sqrtf(powf(blob->x - position->x, 2) + powf(blob->z - position->z, 2));
				if (distanceToBlob <= blob->radius)
				{
					const float heightChange = (cosf(PI * distanceToBlob / blob->radius) / 2 + 0.5) * blob->height;
					position->y += heightChange;
				}
			}

			currentVertex++;
		}
	}
	ComputeNormalsQM(&groundMesh);
	

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("sub-metal.png", &width, &height, &nrChannels, 0);

	glEnable(GL_TEXTURE_2D); // enable cube texture mapping

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	GLfloat planes[] = { 0.0, 0.0, 1.0, 0.0 };
	GLfloat planet[] = { 0.0, 1.0, 0.0, 0.0 };

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, planes);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, planet);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	GLUquadric *qobj = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	gluQuadricDrawStyle(qobj, GLU_FILL);

	submarine.qobj = qobj;
	submarine.textureId = texture;
}

void resize(int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble) width / height, 0.2, 300.0);

	glMatrixMode(GL_MODELVIEW);

	updateCamera();
}

void updateCamera()
{
	// TODO modes
	glLoadIdentity();
	Vector3D position = submarine.getPosition();
	gluLookAt(
		position.x, position.y+50, position.z+50,
		position.x, position.y, position.z,
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

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	
	submarine.draw();

	DrawMeshQM(&groundMesh, meshSize); // draw ground mesh
	// TODO extra ground objects (cubes)

	glutSwapBuffers(); // using GLUT_DOUBLE
}