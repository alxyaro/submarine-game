#include <cstdio>
#include <cstring>
#include <GL/glew.h>
#include <GL/glut.h>
#include "BoundingBox.h"
#include <vector>

extern "C" {
#include "Vector3D.h"
#include "QuadMesh.h"
#include "perlin.h"
}
#include "Submarine.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI acos(-1.0)

#define KEY_MOVEMENT_FORWARD	0b000000000001
#define KEY_MOVEMENT_BACKWARD	0b000000000010
#define KEY_ROTATION_LEFT		0b000000000100
#define KEY_ROTATION_RIGHT		0b000000001000
#define KEY_MOVEMENT_UP			0b000000010000
#define KEY_MOVEMENT_DOWN		0b000000100000
#define KEY_PERISCOPE_LEFT		0b000001000000
#define KEY_PERISCOPE_RIGHT		0b000010000000
#define KEY_ZOOM_IN				0b000100000000
#define KEY_ZOOM_OUT			0b001000000000

int keyMask = 0;

bool debugMode = false;

const int viewportWidth = 650;
const int viewportHeight = 500;

// Light properties
static GLfloat light_position0[] = { -6.0F, 50.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 0.4, 0.4, 0.4, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat mat_ambient[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_specular[] = { 0.1F, 0.1F, 0.1F, 1.0F };
static GLfloat mat_diffuse[] = { 0.8F, 0.8F, 0.8F, 1.0F };
static GLfloat mat_shininess[] = { 0.1F };

const int meshResolution = 100;
const float meshSize = 200;
Vector3D meshOrigin;
QuadMesh groundMesh;

Submarine* submarine;
bool periscopeView;
float zoomPct;
std::vector<AISubmarine*> enemySubs;

// prototypes
bool keyDown(int);
void reset(void);

void init(int, int);
void resize();
void resize(int, int);
void updateCamera(void);
void mainLoop(int);
short determineRequiredAiSubTurnDirection(Vector3, Vector3);
Vector3 getHighestGroundPosition(BoundingBox, bool);
bool aboveGroundMesh(BoundingBox);
bool withinGroundMeshBounds(BoundingBox);
void display(void);

void keyboard(unsigned char,int,int);
void keyboardUp(unsigned char, int, int);
void special(int, int, int);
void specialUp(int, int, int);

bool keyDown(int key)
{
	return (keyMask & key) != 0;
}

void reset(void)
{
	submarine->reset();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(viewportWidth, viewportHeight);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Assignment 1");

	glewExperimental = TRUE;
	glewInit();
	
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


	glEnable(GL_TEXTURE_2D); // enable texture mapping

	unsigned int meshTexture;
	glGenTextures(1, &meshTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, meshTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // GL_LINEAR_MIPMAP_NEAREST
	int width1, height1, nrChannels1;
	unsigned char* data = stbi_load("ocean-floor.png", &width1, &height1, &nrChannels1, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set up ground quad mesh
	meshOrigin = NewVector3D(-meshSize / 2.0f, 0.0f, meshSize / 2.0f);
	const Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	const Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshResolution);
	InitMeshQM(&groundMesh,
		meshResolution,
		meshOrigin,
		meshSize,
		meshSize,
		dir1v,
		dir2v);
	groundMesh.texture = meshTexture; // assign texture

	SetMaterialQM(&groundMesh,
		NewVector3D(0.05f, 0.05f, 0.05f), // ambient
		NewVector3D(1.0f, 1.0f, 1.0f), // diffuse
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
	for (int i = 0; i < meshResolution + 1; i++)
	{
		for (int j = 0; j < meshResolution + 1; j++)
		{
			Vector3D* position = &groundMesh.vertices[currentVertex].position;
			float x = position->x + 10000;
			float z = position->z + 10000;
			position->y = perlin2d(x, z, 0.5f, 4) + perlin2d(x / 20.0f, z / 20.0f, 0.8f, 4) * 10;

			for (unsigned int i = 0; i < sizeof(blobsList) / sizeof(blob); i++)
			{
				blob* blob = &blobsList[i];
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


	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	int width, height, nrChannels;
	unsigned char* subTextureData = stbi_load("sub-metal.png", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, subTextureData);
	glGenerateMipmap(GL_TEXTURE_2D);

	// TODO change the following to a cylindrical or cube texture mapping
	GLfloat planes[] = { 1.0, 0.0, 0.0, 0.0 };
	GLfloat planet[] = { 0.0, 0.0, 1.0, 0.0 };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, planes);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, planet);

	GLUquadric* qobj = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	gluQuadricDrawStyle(qobj, GLU_FILL);

	submarine = new Submarine(texture, qobj);
	submarine->reset();
	submarine->setFast();

	enemySubs.push_back(new AISubmarine(texture, qobj));
	enemySubs.at(0)->reset();
	enemySubs.push_back(new AISubmarine(texture, qobj));
	enemySubs.at(1)->reset();
	srand(234);
}

void resize()
{
	resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

void resize(int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(periscopeView ? 100-(zoomPct*80) : 60.0, (GLdouble)width / height, 0.2, 300.0);
	glMatrixMode(GL_MODELVIEW);

	updateCamera();
}

void updateCamera()
{
	glLoadIdentity();
	Vector3 position = submarine->getPosition();
	if (periscopeView)
	{
		position.y += 2.25 + 0.05;

		position.x -= cosf(submarine->getRotation()) * 2;
		position.z += sinf(submarine->getRotation()) * 2;

		const float periscopeAngleRad = submarine->getPeriscopeRotation();
		
		float dirX = -cosf(submarine->getRotation() + periscopeAngleRad);
		float dirZ = sinf(submarine->getRotation() + periscopeAngleRad);
		
		position.x += dirX * 0.18;
		position.z += dirZ * 0.18;
		
		gluLookAt(
			position.x, position.y, position.z,
			position.x + dirX, position.y, position.z + dirZ,
			0, 1, 0
		); // CTM = I * V
	}
	else
	{
		gluLookAt(
			position.x, position.y + 40, position.z + 40,
			position.x, position.y, position.z,
			0, 1, 0
		); // CTM = I * V
	}
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

	submarine->tick(power, rotation, vertical, deltaTime);
	if (keyDown(KEY_PERISCOPE_LEFT))
		submarine->rotatePeriscope(1.5, true);
	if (keyDown(KEY_PERISCOPE_RIGHT))
		submarine->rotatePeriscope(-1.5, true);
	if (!withinGroundMeshBounds(submarine->getBoundingBox()) || !aboveGroundMesh(submarine->getBoundingBox()))
		submarine->reset();
	
	for (int i = 0; i < enemySubs.size(); i++)
	{
		AISubmarine* enemySub = enemySubs[i];
		if (!enemySub->initialized)
		{
			float x, z;
			do
			{
				x = rand() % 90;
				z = rand() % 90;
				if (rand() % 2 == 0) x *= -1;
				if (rand() % 2 == 0) z *= -1;
			} while (fabs(x - submarine->getPosition().x) < 25 || fabs(z - submarine->getPosition().z) < 25);
			enemySub->getBoundingBox().setPosition(x, 0, z);
			enemySub->rotate(rand() % 360);
			enemySub->setPosition(x, getHighestGroundPosition(enemySub->getBoundingBox(), true).y+10, z);
			enemySub->initialized = true;
		}
		if (enemySub->powerCalcCooldown > 0) enemySub->powerCalcCooldown--;
		if (enemySub->rotationCalcCooldown > 0) enemySub->rotationCalcCooldown--;
		if (enemySub->aiCooldown > 0) enemySub->aiCooldown--;
		else
		{
			// power
			if (enemySub->powerCalcCooldown == 0)
			{
				if (enemySub->powerDirection != 0 && rand() % 200 == 0)
				{
					enemySub->powerDirection = 0;
					enemySub->powerCalcCooldown = 25 + rand() % 50; // 500-1500ms
				}
				else
				{
					enemySub->powerDirection = 1;
				}
			}

			// rotation
			float requiredRotationDir = determineRequiredAiSubTurnDirection(enemySub->getPosition(), enemySub->getVelocity(1));
			if (requiredRotationDir != 0)
				enemySub->rotationDirection = requiredRotationDir;
			else if (enemySub->rotationCalcCooldown == 0)
			{
				if (rand() % 100 <= 25) // 25% chance we start turning
				{
					enemySub->rotationDirection = 0.29f+(static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*0.7f;
					enemySub->rotationDirection *= rand() % 2 == 0 ? -1 : 1;
					enemySub->rotationCalcCooldown = 10 + rand() % 20;
				}
				else
				{
					enemySub->rotationDirection = 0;
					enemySub->rotationCalcCooldown = 50;
				}
			}

			// elevation
			BoundingBox bbox = enemySub->getBoundingBox();
			Vector3 belowSurface = getHighestGroundPosition(bbox, true);
			Vector3 aheadSurface = getHighestGroundPosition(*enemySub->forwardViewBb, true);
			float idealHeight = fmax(belowSurface.y, aheadSurface.y) + 10;

			float idealDiff = idealHeight - bbox.getLowerY();
			if (idealDiff > 0.5)
			{
				enemySub->verticalDirection = 1;
			}
			else if (idealDiff < -2)
			{
				enemySub->verticalDirection = -1;
			}
			else
			{
				enemySub->verticalDirection = 0;
			}
			
			enemySub->aiCooldown = 25 + rand() % 25;
		}
		enemySub->tick(enemySub->powerDirection, enemySub->rotationDirection, enemySub->verticalDirection, deltaTime);
		if (!withinGroundMeshBounds(enemySub->getBoundingBox()) || !aboveGroundMesh(enemySub->getBoundingBox()))
			enemySub->reset(); // TODO explosion animation?
		else if (enemySub->getBoundingBox().collidesWith(submarine->getBoundingBox()))
		{
			enemySub->reset();
			submarine->reset();
			// TODO explosion animation?
		}
		else
		{
			for (int j = i+1; j < enemySubs.size(); j++)
			{
				if (enemySub->getBoundingBox().collidesWith(enemySubs[j]->getBoundingBox()))
				{
					enemySub->reset();
					enemySubs[j]->reset();
					// TODO explosion animation?
					break;
				}
			}
		}
	}
	
	//printf("below mesh = %i\n", belowMesh(submarine->getBoundingBox()));

	bool doResize = false;
	if (keyDown(KEY_ZOOM_IN))
	{
		doResize = true;
		zoomPct = fmin(zoomPct + 0.02, 1);
	}
	if (keyDown(KEY_ZOOM_OUT))
	{
		doResize = true;
		zoomPct = fmax(zoomPct - 0.02, 0);
	}

	if (doResize)
		resize();
	else
		updateCamera();

	glutPostRedisplay();

	glutTimerFunc(20, mainLoop, 0);
}

// this is for avoiding the ground boundary TODO & main submarine
short determineRequiredAiSubTurnDirection(Vector3 position, Vector3 velocity)
{
	velocity.y = 0;
	velocity.normalize();
	
	float padding = 15;
	float safeZone = meshSize / 2 - padding;

	Vector3 targetDirection = { 0,0,0 };

	if (position.x < -safeZone)
		targetDirection.x = 1;
	else if (position.x > safeZone)
		targetDirection.x = -1;
	
	if (position.z < -safeZone)
		targetDirection.z = 1;
	else if (position.z > safeZone)
		targetDirection.z = -1;

	if (!targetDirection.isZero())
	{
		targetDirection.normalize();
		float dotProduct = targetDirection.dotProduct(velocity);
		// dot product:
		// 1 if both vectors same dir
		// 0 if vector perpendicular
		// -1 if vectors opposite dirs
		if (dotProduct < 0.25) // not going in the direction we want
		{
			Vector3 turnLeft = velocity.copy();
			turnLeft.rotateAboutYAxis(0.17); // around 10 deg
			Vector3 turnRight = velocity.copy();
			turnRight.rotateAboutYAxis(-0.17);

			//printf("(%f,%f) < %f > (%f,%f) -> turning %s\n", velocity.x, velocity.z, dotProduct, targetDirection.x, targetDirection.z, turnLeft.angleXZ(targetDirection) < turnRight.angleXZ(targetDirection) ? "left" : "right");
			return turnLeft.angleXZ(targetDirection) < turnRight.angleXZ(targetDirection) ? -1 : 1;
		}
	}
	

	return 0;
}
Vector3D getMeshVertex(int meshX, int meshZ)
{
	int index = (-meshZ) * (meshResolution + 1) + meshX;
	return groundMesh.vertices[index].position;
}
Vector3 getHighestGroundPosition(BoundingBox bbox, bool considerChildren)
{
	bounds bbounds = bbox.getBounds();

	bbounds.x1 -= meshOrigin.x;
	bbounds.z1 -= meshOrigin.z;
	bbounds.x2 -= meshOrigin.x;
	bbounds.z2 -= meshOrigin.z;

	float pct = meshResolution / meshSize;

	bbounds.x1 *= pct;
	bbounds.z1 *= pct;
	bbounds.x2 *= pct;
	bbounds.z2 *= pct;

	int lowerX = ceil(bbounds.x1);
	int upperX = floor(bbounds.x2);
	int lowerZ = ceil(bbounds.z1);
	int upperZ = floor(bbounds.z2);

	Vector3 result(0, INT_MIN, 0);
	bool averageOut = false;
	int averageCount = 0;
	
	if (lowerX > upperX)
	{
		lowerX -= 1;
		upperX += 1;
		averageOut = true;
	}

	if (lowerZ > upperZ)
	{
		lowerZ -= 1;
		upperZ += 1;
		averageOut = true;
	}

	for (int x = lowerX; x <= upperX; x++)
	{
		for (int z = lowerZ; z <= upperZ; z++)
		{
			if (x < 0 || x > meshResolution || z < -meshResolution || z > 0)
				continue;

			Vector3D pos = getMeshVertex(x, z);

			if (averageOut)
			{
				if (averageCount == 0)
					result.y = 0;
				result.x += pos.x;
				result.y += pos.y;
				result.z += pos.z;
				averageCount++;
				continue;
			}

			if (pos.y > result.y)
			{
				result.x = pos.x;
				result.y = pos.y;
				result.z = pos.z;
			}
		}
	}

	if (averageOut && averageCount > 0)
	{
		result.x /= averageCount;
		result.y /= averageCount;
		result.z /= averageCount;
	}

	if (considerChildren && bbox.child != nullptr)
	{
		Vector3 other = getHighestGroundPosition(*bbox.child, true);
		return other.y > result.y ? other : result;
	}
	return result;
}

bool withinGroundMeshBounds(BoundingBox bbox)
{
	bounds bbounds = bbox.getBounds();

	Vector3 meshLowerCorner(meshOrigin.x, 0, meshOrigin.z - meshSize);
	Vector3 meshUpperCorner(meshOrigin.x + meshSize, 0, meshOrigin.z);
	
	if (bbounds.x1 < meshLowerCorner.x || bbounds.x2 > meshUpperCorner.x || bbounds.z1 < meshLowerCorner.z || bbounds.z2 > meshUpperCorner.z)
		return false;
	if (bbox.child != nullptr)
		return withinGroundMeshBounds(*bbox.child);
	return true;
}

bool aboveGroundMesh(BoundingBox bbox)
{
	return bbox.getLowerY() > getHighestGroundPosition(bbox, true).y;
}

void visualizeGroundBoundary(BoundingBox bbox)
{
	Vector3 pos = getHighestGroundPosition(bbox, false);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	if (bbox.getLowerY() <= pos.y)
		glColor3f(1, 1, 0);
	else
		glColor3f(0, 1, 0);
	glVertex3f(pos.x, pos.y, pos.z);
	glVertex3f(pos.x, bbox.getLowerY(), pos.z);

	glEnd();
	glEnable(GL_LIGHTING);
	if (bbox.child != nullptr)
		visualizeGroundBoundary(*bbox.child);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set materials
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	
	submarine->draw();
	if (debugMode)
	{
		submarine->getBoundingBox().debugDraw();
		visualizeGroundBoundary(submarine->getBoundingBox());
	}

	for (int i = 0; i < enemySubs.size(); i++)
	{
		AISubmarine* enemySub = enemySubs[i];
		if (!enemySub->initialized)
			continue;
		enemySub->draw();
		if (debugMode)
		{
			enemySub->getBoundingBox().debugDraw();
			enemySub->forwardViewBb->debugDraw();
			visualizeGroundBoundary(enemySub->getBoundingBox());
		}
	}

	DrawMeshQM(&groundMesh, meshResolution, debugMode); // draw ground mesh
	// TODO extra ground objects (cubes)

	glutSwapBuffers(); // using GLUT_DOUBLE
}


// KEYBOARD INPUT

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		keyMask |= KEY_MOVEMENT_FORWARD; break;
	case 's':
		keyMask |= KEY_MOVEMENT_BACKWARD; break;
	case 'd':
		keyMask |= KEY_ROTATION_RIGHT; break;
	case 'a':
		keyMask |= KEY_ROTATION_LEFT; break;
	case 'm':
		keyMask |= KEY_ZOOM_IN; break;
	case 'n':
		keyMask |= KEY_ZOOM_OUT; break;
	case 'r':
		reset(); break;
	case 'x':
		submarine->alignWithPeriscope();
		break;
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
	case 'm':
		keyMask &= ~KEY_ZOOM_IN; break;
	case 'n':
		keyMask &= ~KEY_ZOOM_OUT; break;
	}
}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		keyMask |= KEY_MOVEMENT_UP; break;
	case GLUT_KEY_DOWN:
		keyMask |= KEY_MOVEMENT_DOWN; break;
	case GLUT_KEY_LEFT:
		keyMask |= KEY_PERISCOPE_LEFT; break;
	case GLUT_KEY_RIGHT:
		keyMask |= KEY_PERISCOPE_RIGHT; break;
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
	case GLUT_KEY_LEFT:
		keyMask &= ~KEY_PERISCOPE_LEFT; break;
	case GLUT_KEY_RIGHT:
		keyMask &= ~KEY_PERISCOPE_RIGHT; break;
	case GLUT_KEY_F2:
		periscopeView = !periscopeView;
		resize();
		break;
	case GLUT_KEY_F3:
		debugMode = !debugMode; break;
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