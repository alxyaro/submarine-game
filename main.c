#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "QuadMesh.h"

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

// prototypes
void init(int, int);
void resize(int, int);
void display(void);
void drawSubmarine(void);
void drawSubBody(void);
void drawSubTower(void);
void drawFin(void);
void functionKeys(int,int,int);
void TEMP_rotation(void);
void drawSubPropeller(void);

GLUquadricObj *qobj;

float propellerRotation = 0;

int modelRotation = 0;

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
	//glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	TEMP_rotation();
	
	glutMainLoop();

	return 0;
}

void TEMP_rotation()
{
	propellerRotation += 2;
	if (propellerRotation >= 360)
		propellerRotation -= 360;
	glutPostRedisplay();
	glutTimerFunc(20, TEMP_rotation, 0);
}

void functionKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_RIGHT:
		modelRotation += 10;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		modelRotation -= 10;
		glutPostRedisplay();
		break;
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
	const Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	const Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	const Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh,
		meshSize,
		origin,
		16.0,
		16.0,
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
	gluPerspective(60.0, (GLdouble) width / height, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		0, 8, 15,
		0, 0, 0,
		0, 1, 0
	); // CTM = I * V
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSubmarine();

	DrawMeshQM(&groundMesh, meshSize); // draw ground mesh

	glutSwapBuffers(); // using GLUT_DOUBLE
}

void drawSubmarine()
{
	// set materials
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glPushMatrix(); // begin full model

	// POSITION/ROTATION
	// TEMP: p' = CTM * P(0,2,0) * p
	glTranslatef(0.0, 2.0, 0.0); // move into position
	glRotatef(modelRotation, 0, 1, 0); // TODO REMOVE: rotate whole model

	// MAIN BODY
	drawSubBody();

	// TOWER
	drawSubTower();

	// SIDE FINS
	glPushMatrix();
	// TEMP: p' = CTM * P(0,2,0) * T(-3.4,0,0) * p
	glTranslatef(-3.4, 0, 0); // move closer to the front of the sub
	glPushMatrix();
	// TEMP: p' = CTM * P(0,2,0) * T(-3.4,0,0) * R(90,1,0,0) * p
	glRotatef(90, 1, 0, 0); // rotate to the right
	// TEMP: p' = CTM * P(0,2,0) * T(-3.4,0,0) * R(90,1,0,0) * T(0,0.8,0) p
	glTranslatef(0, 0.8, 0); // distance from the submarine
	drawFin(); // right fin
	glPopMatrix();
	glPushMatrix();
	// TEMP: p' = CTM * P(0,2,0) * T(-3.4,0,0) * R(-90,1,0,0) * p
	glRotatef(-90, 1, 0, 0); // rotate to the left
	// TEMP: p' = CTM * P(0,2,0) * T(-3.4,0,0) * R(-90,1,0,0) * T(0,0.8,0) p
	glTranslatef(0, 0.8, 0); // distance from the submarine
	drawFin(); // left fin
	glPopMatrix();
	glPopMatrix();

	// BACK FINS
	glPushMatrix();
	// TEMP: p' = CTM * P(0,2,0) * T(4.8,0,0) * p
	glTranslatef(4.8, 0, 0); // move to the back of the sub
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		// TEMP: p' = CTM * P(0,2,0) * T(4.8,0,0) * R(_,1,0,0) * p
		glRotatef(90*i, 1, 0, 0); // rotate
		// TEMP: p' = CTM * P(0,2,0) * T(4.8,0,0) * R(_,1,0,0) * T(0,0.3,0) * p
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
	glTranslatef(-2, 0.8, 0);
	glRotatef(-25, 0, 0, 1);
	glScalef(1, 1.2, 1);
	glRotatef(20, 0, 0, 1);
	glScalef(1, 1, 0.5);
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	gluCylinder(qobj, 0.5, 0.5, 0.8, 20, 20);
	glPopMatrix();
	glTranslatef(0, 0.8, 0);
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
	
	glTranslatef(5.4, 0, 0); // move to the back of the sub

	glPushMatrix(); // ring
	
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj, radius, radiusEnd, length, 20, 20);
	gluCylinder(qobj, radius - thickness, radiusEnd - thicknessEnd, length, 20, 20);

	glutSolidTorus(thickness / 2, radius - thickness / 2, 20, 20);

	glPushMatrix();
	glTranslatef(0, 0, length);
	glutSolidTorus(thicknessEnd / 2, radiusEnd - thicknessEnd / 2, 20, 20);
	glPopMatrix();

	glPopMatrix(); // end ring

	glPushMatrix(); // propeller

	glPushMatrix(); // propeller rod
	glTranslatef(-0.1, 0, 0);
	glRotatef(90, 0, 1, 0);
	gluCylinder(qobj, 0.06, 0.06, 0.5, 10, 10);
	glPopMatrix();
	
	glTranslatef(length/2, 0, 0);
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		glRotatef(i * 90 - propellerRotation, 1, 0, 0); // rotate the blade
		glRotatef(-30, 0, 1, 0); // angle the blade outwards a bit
		glScalef(0.15, 0.15, 0.15); // scale down the whole thing
		glScalef(1, 1, 0.3f); // make the shapes thin - like a propeller blade
		glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		gluCylinder(qobj, 0.5, 0.5, 1.5, 15, 15);
		glPopMatrix();
		glTranslatef(0, 1.2, 0);
		glScalef(1, 2.5f, 1);
		gluSphere(qobj, 0.5, 15, 15);
		glPopMatrix();
	}
	
	glPopMatrix(); // end propeller

	glPopMatrix();
}