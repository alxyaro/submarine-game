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
void drawFin(void);
void TEMP_rotation(void);

GLUquadricObj *qobj;

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
	//glutSpecialFunc(functionKeys);

	TEMP_rotation();
	
	glutMainLoop();

	return 0;
}

void TEMP_rotation()
{
	modelRotation += 1;
	if (modelRotation >= 360)
		modelRotation -= 360;
	glutPostRedisplay();
	glutTimerFunc(50, TEMP_rotation, 0);
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
		NewVector3D(0.0f, 0.0f, 0.9f),
		NewVector3D(0.4f, 0.4f, 0.4f),
		NewVector3D(0.04f, 0.04f, 0.04f),
		0);

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
	);
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

	glPushMatrix(); // full model
	glTranslatef(0.0, 2.0, 0.0); // move into position
	glRotatef(modelRotation, 0, 1, 0); // TEMP: rotate whole model

	double baseLength = 8;
	glPushMatrix(); // main body
	glTranslatef(-baseLength / 2, 0, 0);
	glRotatef(90,0,1,0);
	gluCylinder(qobj,
		1,
		1,
		baseLength-2,
		20,
		20);
	glPopMatrix(); // end main body

	glPushMatrix();
	glTranslatef(-baseLength / 2,  0, 0);
	glScalef(1.5f,1,1);
	gluSphere(qobj,1,20,20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2, 0, 0);
	glScalef(3.5f, 1, 1);
	gluSphere(qobj, 1, 30, 30);
	glPopMatrix();

	// SIDE FINS
	glPushMatrix();
	// TODO
	glPopMatrix();

	
	// BACK FINS
	glPushMatrix();
	glTranslatef(4.8, 0, 0); // move to the back of the sub
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		glRotatef(90*i, 1, 0, 0); // rotate
		glTranslatef(0, 0.3, 0); // distance from the submarine
		drawFin();
		glPopMatrix();
	}
	glPopMatrix();
	
	glPopMatrix(); // end full model
	
}

void drawFin()
{
	glScalef(0.8, 0.5, 0.2);
	glRotatef(-3,0,0,1);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	gluCylinder(qobj, 0.5, 0.4, 2, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0,2,0);
	glScalef(1, 0.2, 1);
	gluSphere(qobj,0.4,20,20);
	glPopMatrix();
}