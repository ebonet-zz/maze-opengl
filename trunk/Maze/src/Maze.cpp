/* maze generation program */

//#include "glew/glew.h"
#include "GL/glew.h"
#include <SFML/Window.hpp>

#include <stdlib.h>
#include <stdio.h>
#include "ShaderManager.h"

#include <cmath>
#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>

using namespace std;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <unistd.h>
#include <string.h>
#endif

#define RESOLUTION 512

/* some useful constants */
#define TRUE 1
#define FALSE 0

#define RIGHT 1
#define LEFT -1
#define UP 1
#define DOWN -1
#define ROUND_PRECISION 50
#define CIRC 2 * M_PI
#define DTHETA CIRC / ROUND_PRECISION
#define TRACKBALLSIZE (0.8)          // trackball size in percentage of window
#define Z_SENSITIVITY 0.01           // used to scale translations in z
#define XY_SENSITIVITY 0.01          // used to scale translations in x and y
#define TARGET_FPS 30                // controls spin update rate
#define TIME_WINDOW 3                // number of frames motion is valid after release
#define WALL_WIDTH_DELTA 0.05
#define WALL_HEIGHT 0.3
#define MAZE 0
#define TRACKBALL 1

int displayMode = MAZE;
float motionTime = .0f;
int lastPos[2] = { 0, 0 };
int buttonDown[3] = { 0, 0 };
int spin = FALSE;                    // are we spinning?
int xsize, ysize;                  // window size

GLint loc;
GLint tLoc;
float mazeWidth = 4;
float wd = 1.0 / mazeWidth;
float hw = wd * 2.0;

//Shader program
const char* NORMAL_ATTRIBUTE_NAME = "meshNormal";

GLfloat colors[][3] = { { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, {
		1.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } };
GLfloat GRAY[3] = { 0.5, 0.5, 0.5 };
GLfloat YELLOW[3] = { 1.0, 1.0, 0.3 };

GLfloat faceNormals[][3] = { { 1.0, 0.0, 0.0 }, { -1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 0.0, 0.0,
		1.0 }, { 0.0, 0.0, -1.0 } };

/* 2D point structure */
typedef struct {
	float x;
	float y;
} Point2;

/* edge structure */
typedef struct {
	int cell1; /* edge connects cell1 and cell2 */
	int cell2;
	int vertex1; /* endpoints of edge - indexes into vertex array */
	int vertex2;
	int valid; /* edge can be removed */
	int draw; /* edge should be drawn */
} Edge;

struct Wall {
	GLfloat vertices[8][3];
	bool isHorizontal;

	Wall(GLfloat * newVertices) {
		memcpy(vertices, newVertices, 24 * sizeof(GLfloat));
		isHorizontal = false;
	}

	Wall(Point2 * p1, Point2 * p2) {
		if (p1->x == p2->x) {
			isHorizontal = false;
			// Vertical line

			vertices[0][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[0][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[0][2] = 0.0;

			vertices[1][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[1][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[1][2] = WALL_HEIGHT;

			vertices[2][0] = p1->x + WALL_WIDTH_DELTA;
			vertices[2][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[2][2] = WALL_HEIGHT;

			vertices[3][0] = p1->x + WALL_WIDTH_DELTA;
			vertices[3][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[3][2] = 0.0;

			vertices[4][0] = p2->x - WALL_WIDTH_DELTA;
			vertices[4][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[4][2] = 0.0;

			vertices[5][0] = p2->x - WALL_WIDTH_DELTA;
			vertices[5][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[5][2] = WALL_HEIGHT;

			vertices[6][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[6][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[6][2] = WALL_HEIGHT;

			vertices[7][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[7][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[7][2] = 0.0;
		} else {
			// Horizontal line

			isHorizontal = true;

			vertices[0][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[0][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[0][2] = 0.0;

			vertices[1][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[1][1] = p1->y - WALL_WIDTH_DELTA;
			vertices[1][2] = WALL_HEIGHT;

			vertices[2][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[2][1] = p1->y + WALL_WIDTH_DELTA;
			vertices[2][2] = WALL_HEIGHT;

			vertices[3][0] = p1->x - WALL_WIDTH_DELTA;
			vertices[3][1] = p1->y + WALL_WIDTH_DELTA;
			vertices[3][2] = 0.0;

			vertices[4][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[4][1] = p2->y - WALL_WIDTH_DELTA;
			vertices[4][2] = 0.0;

			vertices[5][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[5][1] = p2->y - WALL_WIDTH_DELTA;
			vertices[5][2] = WALL_HEIGHT;

			vertices[6][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[6][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[6][2] = WALL_HEIGHT;

			vertices[7][0] = p2->x + WALL_WIDTH_DELTA;
			vertices[7][1] = p2->y + WALL_WIDTH_DELTA;
			vertices[7][2] = 0.0;
		}
	}

	void draw_vertex(float color[3], float coordinates[3]) {
		glVertexAttrib3fv(loc, color);
		Point2 attribData;
		attribData.x = coordinates[0] * wd + hw;
		attribData.y = coordinates[1] * wd + hw;
		glVertexAttrib2fv(tLoc, &attribData.x);
		glVertex3fv(coordinates);
	}

	void draw_mesh(int a, int b, int c, int d, int normal) {
		glBegin(GL_POLYGON);
		{
			glNormal3fv(faceNormals[normal]);
			draw_vertex(colors[a], vertices[a]);
			draw_vertex(colors[b], vertices[b]);
			draw_vertex(colors[c], vertices[c]);
			draw_vertex(colors[d], vertices[d]);
		}
		glEnd();
	}

	void draw() {
		if (isHorizontal) {
			draw_mesh(1, 0, 3, 2, 0);
			draw_mesh(3, 7, 6, 2, 3);
			draw_mesh(7, 3, 0, 4, 4);
			draw_mesh(2, 6, 5, 1, 5);
			draw_mesh(4, 5, 6, 7, 1);
			draw_mesh(5, 4, 0, 1, 2);
		} else {
			draw_mesh(1, 0, 3, 2, 2);
			draw_mesh(3, 7, 6, 2, 1);
			draw_mesh(7, 3, 0, 4, 4);
			draw_mesh(2, 6, 5, 1, 5);
			draw_mesh(4, 5, 6, 7, 3);
			draw_mesh(5, 4, 0, 1, 0);
		}
	}
};

struct Bound {

	float xMin, xMax, yMin, yMax;
	Bound(Point2 * p1, Point2 * p2) {
		xMin = xMax = yMin = yMax = 0;

		// is vertical
		if (p1->x == p2->x) {
			xMin = p1->x - 3 * WALL_WIDTH_DELTA;
			xMax = p1->x + 3 * WALL_WIDTH_DELTA;
			yMax = p2->y + 3 * WALL_WIDTH_DELTA;
			yMin = p1->y - 3 * WALL_WIDTH_DELTA;
		}
		// is horizontal
		else {
			xMin = p1->x - 3 * WALL_WIDTH_DELTA;
			xMax = p2->x + 3 * WALL_WIDTH_DELTA;
			yMin = p1->y - 3 * WALL_WIDTH_DELTA;
			yMax = p1->y + 3 * WALL_WIDTH_DELTA;
		}

	}

	bool isPointInsideBound(float x, float y) {
		return (x >= xMin) && (x <= xMax) && (y >= yMin) && (y <= yMax);
	}

};

/* global parameters */
int w, h, edges, perimeters, vertices, groups, *group = NULL, redges, done = 0;
Edge *edge = NULL, *perimeter = NULL;
Point2 *vertex = NULL;
vector<Wall> walls;
vector<Bound> bounds;

// Storage space for the various transformations we'll need
float trackballTranslation1[16], trackballTranslation2[16], trackballRotation[16], trackballIncRotation[16];
float mazeTranslation[16], mazeRotation[16];
float currentAngle = 0;
float currentPositionX = 0;
float currentPositionY = 0;

GLfloat verticesTrackBall[][3] = { { 0.0, 0.0, 0.0 }, { w, 0.0, 0.0 }, { w, h, 0.0 }, { 0.0, h, 0.0 }, { 0.0, 0.0,
		WALL_HEIGHT }, { w, 0.0, WALL_HEIGHT }, { w, h, WALL_HEIGHT }, { 0.0, h, WALL_HEIGHT } };

/* init_maze initializes a w1 by h1 maze.  all walls are initially
 included.  the edge and perimeter arrays, vertex array, and group
 array are allocated and filled in.  */

void init_maze(int w1, int h1) {
	int i, j, vedges, hedges;
	float x, y, t, inc, xoff, yoff;

	vedges = (w1 - 1) * h1; /* number of vertical edges */
	hedges = (h1 - 1) * w1; /* number of horizontal edges */
	redges = edges = vedges + hedges; /* number of removable edges */
	perimeters = 2 * w1 + 2 * h1;
	vertices = (w1 + 1) * (h1 + 1);
	groups = w1 * h1;

	/* allocate edge array */
	if (edge != NULL)
		free(edge);
	if ((edge = (Edge*) malloc(edges * sizeof(Edge))) == NULL) {
		fprintf(stderr, "Could not allocate edge table\n");
		exit(1);
	}

		/* fill in the vertical edges */
	for (i = 0; i < vedges; i++) {
		x = i % (w1 - 1); /* convert edge number to column */
		y = i / (w1 - 1); /* and row */
		j = y * w1 + x; /* convert to cell number */
		edge[i].cell1 = j;
		edge[i].cell2 = j + 1;
		edge[i].vertex1 = y * (w1 + 1) + x + 1; /* convert to vertex number */
		edge[i].vertex2 = (y + 1) * (w1 + 1) + x + 1;
		edge[i].valid = TRUE;
		edge[i].draw = TRUE;
	}
	for (i = vedges; i < edges; i++) {
		j = i - vedges; /* convert to cell number */
		x = j % w1; /* convert edge number to column */
		y = j / w1; /* and row*/
		edge[i].cell1 = j;
		edge[i].cell2 = j + w1;
		edge[i].vertex1 = (y + 1) * (w1 + 1) + x; /* convert to vertex number */
		edge[i].vertex2 = (y + 1) * (w1 + 1) + x + 1;
		edge[i].valid = TRUE;
		edge[i].draw = TRUE;
	}

	/* allocate perimeter */
	if (perimeter != NULL)
		free(perimeter);
	if ((perimeter = (Edge*) malloc(perimeters * sizeof(Edge))) == NULL) {
		fprintf(stderr, "Could not allocate perimeter table\n");
		exit(1);
	}

		/* fill in horizontal perimeter */
	for (i = 0; i < w1; i++) {
		perimeter[2 * i].cell1 = i;
		perimeter[2 * i].cell2 = i;
		perimeter[2 * i].vertex1 = i;
		perimeter[2 * i].vertex2 = i + 1;
		perimeter[2 * i].valid = TRUE;
		perimeter[2 * i].draw = TRUE;
		perimeter[2 * i + 1].cell1 = i + h1 * w1;
		perimeter[2 * i + 1].cell2 = i + h1 * w1;
		perimeter[2 * i + 1].vertex1 = i + h1 * (w1 + 1);
		perimeter[2 * i + 1].vertex2 = i + h1 * (w1 + 1) + 1;
		perimeter[2 * i + 1].valid = TRUE;
		perimeter[2 * i + 1].draw = TRUE;
	}
	/* fill in vertical perimeter */
	for (i = w1; i < w1 + h1; i++) {
		j = i - w1;
		perimeter[2 * i].cell1 = j * w1;
		perimeter[2 * i].cell2 = j * w1;
		perimeter[2 * i].vertex1 = j * (w1 + 1);
		perimeter[2 * i].vertex2 = (j + 1) * (w1 + 1);
		perimeter[2 * i].valid = TRUE;
		perimeter[2 * i].draw = TRUE;
		perimeter[2 * i + 1].cell1 = (j + 1) * w1 - 1;
		perimeter[2 * i + 1].cell2 = (j + 1) * w1 - 1;
		perimeter[2 * i + 1].vertex1 = (j + 1) * (w1 + 1) - 1;
		perimeter[2 * i + 1].vertex2 = (j + 2) * (w1 + 1) - 1;
		perimeter[2 * i + 1].valid = TRUE;
		perimeter[2 * i + 1].draw = TRUE;
	}

	/* allocate vertex array */
	if (vertex != NULL)
		free(vertex);
	if ((vertex = (Point2*) malloc(vertices * sizeof(Point2))) == NULL) {
		fprintf(stderr, "Could not allocate vertex table\n");
		exit(1);
	}

		/* figure out the spacing between vertex coordinates.  we want
		 square cells so use the minimum spacing */
	inc = 3.6 / w1;
	t = 3.6 / h1;
	if (t < inc) {
		inc = t;
	}
	/* determine the required offsets to center the maze using the
	 spacing calculated above */
	xoff = (4.0 - w1 * inc) / 2 - 2.0;
	yoff = (4.0 - h1 * inc) / 2 - 2.0;
	/* fill in the vertex array */
	for (i = 0; i < vertices; i++) {
		x = i % (w1 + 1);
		y = i / (w1 + 1);
		vertex[i].x = x * inc + xoff;
		vertex[i].y = y * inc + yoff;
	}

	/* allocate the group table */
	if (group != NULL)
		free(group);
	if ((group = (int*) malloc(groups * sizeof(int))) == NULL) {
		fprintf(stderr, "Could not allocate group table\n");
		exit(1);
	}

		/* set the group table to the identity */
	for (i = 0; i < groups; i++) {
		group[i] = i;
	}

}

/* this function removes one wall from the maze.  if removing this
 wall connects all cells, an entrance and exit are created and a
 done flag is set */
void remove_one_edge(void) {
	int i, j, k, o, n;

	/* randomly select one of the the remaining walls */
	k = rand() % redges;
	/* scan down the edge array till we find the kth removeable edge */
	for (i = 0; i < edges; i++) {
		if (edge[i].valid == TRUE) {
			if (k == 0) {
				edge[i].valid = FALSE;
				n = group[edge[i].cell1];
				o = group[edge[i].cell2];
				/* if the cells are already connected don't remove the wall */
				if (n != o) {
					edge[i].draw = FALSE;
					done = 1;
					/* fix up the group array */
					for (j = 0; j < groups; j++) {
						if (group[j] == o) {
							group[j] = n;
						}
						if (group[j] != n) {
							done = 0; /* if we have more than one
							 group we're not done */
						}
					}
				}
				break;
			} else {
				k--;
			}
		}
	}
	redges--; /* decriment the number of removable edges */
	/* if we're done, create an entrance and exit */
	if (done) {
		for (j = 0; j < 1; j++) {
			/* randomly select a perimeter edge */
			k = rand() % (perimeters - j);
			for (i = 0; i < perimeters; i++) {
				if (k == 0) {
					if (perimeter[i].valid == TRUE) {
						perimeter[i].draw = FALSE;
						break;
					}
				} else {
					k--;
				}
			}
		}
	}

}

void create_walls() {
	walls.clear();

	Point2 * p1, *p2;
	int i;
	for (i = 0; i < edges; i++) {
		if (edge[i].draw == TRUE) {

			p1 = vertex + edge[i].vertex1;
			p2 = vertex + edge[i].vertex2;

			walls.push_back(Wall(p1, p2));
		}
	}
	/* draw the perimeter edges */
	for (i = 0; i < perimeters; i++) {
		if (perimeter[i].draw == TRUE) {

			p1 = vertex + perimeter[i].vertex1;
			p2 = vertex + perimeter[i].vertex2;

			walls.push_back(Wall(p1, p2));
		}
	}
}

void create_bounds() {
	bounds.clear();

	Point2 * p1, *p2;
	int i;
	for (i = 0; i < edges; i++) {
		if (edge[i].draw == TRUE) {

			p1 = vertex + edge[i].vertex1;
			p2 = vertex + edge[i].vertex2;

			bounds.push_back(Bound(p1, p2));
		}
	}
	/* draw the perimeter edges */
	for (i = 0; i < perimeters; i++) {
		if (perimeter[i].draw == TRUE) {

			p1 = vertex + perimeter[i].vertex1;
			p2 = vertex + perimeter[i].vertex2;

			bounds.push_back(Bound(p1, p2));
		}
	}

}

void build_maze() {
	done = 0;
	/* build maze */
	init_maze(w, h);
	while (!done) {
		/* remove one edge */
		remove_one_edge();
	}
	create_walls();
	create_bounds();
}

void draw_maze(void) {
	GLuint i;

	for (i = 0; i < walls.size(); i++) {
		walls[i].draw();
	}
}

void resetModelViewMatrix() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void setLookAt() {
	glLoadIdentity();
	if (displayMode != MAZE) {
		gluLookAt(0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	} else {

		gluLookAt(currentPositionX, currentPositionY, WALL_HEIGHT / 2.0, currentPositionX + cos(currentAngle),
				currentPositionY + sin(currentAngle), WALL_HEIGHT / 2.0, 0.0, 0.0, 1.0);

	}
}

void resetAndApplyAllTransforms() {
	// Clean any mess
	resetModelViewMatrix();

	// Set the viewing to the same value as came in default
	setLookAt();

	glMultMatrixf(trackballTranslation2);
	glMultMatrixf(trackballRotation);
	glMultMatrixf(trackballTranslation1);
}

void incrementRotation() {
	// Clean any mess
	resetModelViewMatrix();

	// Accumulate rotation and increment
	glMultMatrixf(trackballIncRotation);
	glMultMatrixf(trackballRotation);

	// Save for future use
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) trackballRotation);
}

void drawFloor() {

	//glColor3fv(GRAY);
	glVertexAttrib3fv(loc, GRAY);
	glBegin(GL_QUADS);
	{
		glNormal3fv(faceNormals[5]);
		glVertex3f(-1.9, -1.9, -0.001);
		glVertex3f(-1.9, 1.9, -0.001);
		glVertex3f(1.9, 1.9, -0.001);
		glVertex3f(1.9, -1.9, -0.001);

	}
	glEnd();

}

void drawPerson() {

	glVertexAttrib3fv(loc, YELLOW);
	glPushMatrix();
	glTranslatef(currentPositionX, currentPositionY, 0.0);
	//glNormal3fv(faceNormals[4]);
	gluDisk(gluNewQuadric(), 0, WALL_WIDTH_DELTA, 4.0, 4.0);

	gluCylinder(gluNewQuadric(), WALL_WIDTH_DELTA, WALL_WIDTH_DELTA, 0.6, 4.0, 4.0);

	glTranslatef(0.0, 0.0, 0.6);
	//glNormal3fv(faceNormals[5]);
	gluDisk(gluNewQuadric(), 0, WALL_WIDTH_DELTA, 4.0, 4.0);
	glPopMatrix();
}

void configure() {
	glEnable(GL_DEPTH_TEST);

	// initialize the projection stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (displayMode == MAZE) {
		gluPerspective(60, 1.0, 0.1, 100);
	} else {
		gluPerspective(30, 1.0, 0.1, 100);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, xsize, ysize);

	setLookAt();
}

void display(GLint prog) {
	loc = glGetAttribLocation(prog, "attr_color");
	tLoc = glGetAttribLocation(prog, "localAttr");

	configure();

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (displayMode == TRACKBALL) {
		resetAndApplyAllTransforms();
	}
	drawFloor();
	draw_maze();
	if (displayMode == TRACKBALL) {
		drawPerson();
	}
	glFlush();
}

// update the modelview matrix with a new translation in the z direction
void update_z(int x1, int y1, int x2, int y2) {
	// Lets calculate the translation amount
	GLfloat translationOnZ = (y1 - y2) * Z_SENSITIVITY;

	// Apply it!
	// First, lets reset the modelview back to default
	resetModelViewMatrix();

	// Then, lets restore only what we had for translation before
	glMultMatrixf(trackballTranslation2);

	// Now lets apply the new Z translation on top of that
	glTranslatef(0, 0, translationOnZ);

	// We now have the new accumulated translation values, lets save it back so we can use it later
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballTranslation2);
}

// update the modelview matrix with a new translation in the x and/or y direction
void update_trans(int x1, int y1, int x2, int y2) {
	// Lets calculate the translation amount
	GLfloat translationOnX = (x2 - x1) * XY_SENSITIVITY;
	GLfloat translationOnY = (y1 - y2) * XY_SENSITIVITY;

	// Apply it!
	// First, lets reset the modelview back to default
	resetModelViewMatrix();

	// Then, lets restore only what we had for translation before
	glMultMatrixf(trackballTranslation2);

	// Now lets apply the new X and Y translation on top of that
	glTranslatef(translationOnX, translationOnY, 0);

	// We now have the new accumulated translation values, lets save it back so we can use it later
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballTranslation2);
}

// find the z coordinate corresponding to the mouse position
float project_to_sphere(float r, float x, float y) {
	float d, t, z;

	d = sqrt(x * x + y * y);
	t = r / M_SQRT2;
	if (d < t) {       // Inside sphere
		z = sqrt(r * r - d * d);
	} else {             // On hyperbola
		z = t * t / d;
	}

	return z;
}

// update the modelview matrix with a new rotation
void update_rotate(int x1, int y1, int x2, int y2) {
	float axis[3], p1[3], p2[3];
	float phi, t;

	if (x1 == x2 && y1 == y2) { // if there's no movement, no rotation
		axis[0] = 1;
		axis[1] = 0;
		axis[2] = 0;
		phi = 0;
	} else {
		// first vector
		p1[0] = (2.0 * x1) / (float) xsize - 1.0;
		p1[1] = 1.0 - (2.0 * y1) / (float) ysize;
		p1[2] = project_to_sphere((float) TRACKBALLSIZE, p1[0], p1[1]);
		// second vector
		p2[0] = (2.0 * x2) / (float) xsize - 1.0;
		p2[1] = 1.0 - (2.0 * y2) / (float) ysize;
		p2[2] = project_to_sphere((float) TRACKBALLSIZE, p2[0], p2[1]);

		// the axis of rotation is given by the cross product of the first
		// and second vectors
		axis[0] = p1[1] * p2[2] - p1[2] * p2[1];
		axis[1] = p1[2] * p2[0] - p1[0] * p2[2];
		axis[2] = p1[0] * p2[1] - p1[1] * p2[0];
		t = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2];
		axis[0] /= t;
		axis[1] /= t;
		axis[2] /= t;

		// the amount of rotation is proportional to the magnitude of the
		// difference between the vectors
		t = sqrt(
				(p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1])
						+ (p1[2] - p2[2]) * (p1[2] - p2[2])) / (2.0 * TRACKBALLSIZE);

		if (t > 1.0) {
			t = 1.0;
		}
		if (t < -1.0) {
			t = -1.0;
		}
		phi = 360.0 * asin(t) / M_PI;
	}

	// First, lets reset the modelview to the default
	resetModelViewMatrix();

	// Then, lets rotate it by the new values
	glRotatef(phi, axis[0], axis[1], axis[2]);

	// This should give us our incremental rotation factor
	// Lets save it for future use
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) trackballIncRotation);

	// Now, apply the incremental factor on what we already had for rotation
	glMultMatrixf(trackballRotation);

	// Finally, save the accumulated total rotation for future use
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) trackballRotation);
}

// function used to spin the object.
void SpinCube2(int t) {
	if (spin) {  // if we're still spinning, increment the rotation
		incrementRotation();
	}
}

bool isWall(float x, float y) {

	for (unsigned int i = 0; i < bounds.size(); i++) {
		if (bounds[i].isPointInsideBound(x, y))
			return true;
	}

	return false;

}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	xsize = w;
	ysize = h;

	// default aspectRatio was 1.0
	//GLfloat newAspectRatio = ((float) xsize) / ((float) ysize);
}

void gfxinit() {
	int i;
	float x = 0, y = 0, z = 0;
	glEnable(GL_DEPTH_TEST);
	// initialize the projection stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (displayMode == MAZE) {
		gluPerspective(60, 1.0, 0.1, 100);
	} else {
		gluPerspective(30, 1.0, 0.1, 100);
	}
	// initialize the modelview stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballTranslation2);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballRotation);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballIncRotation);
	for (i = 0; i < 8; i++) {
		x += verticesTrackBall[i][0];
		y += verticesTrackBall[i][1];
		z += verticesTrackBall[i][2];
	}
	x /= 8;
	y /= 8;
	z /= 8;
	glTranslatef(-x, -y, -z);
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &trackballTranslation1);
	glLoadIdentity();

	if (displayMode != MAZE) {
		glMultMatrixf(trackballTranslation1);
	}

	glViewport(0, 0, RESOLUTION, RESOLUTION);
	xsize = RESOLUTION;
	ysize = RESOLUTION;

	setLookAt();
	build_maze();

	if (displayMode == MAZE) {
		currentPositionX = -1.6 + rand() % 4;
		currentPositionY = -1.3 + rand() % 3;

		while (isWall(currentPositionX, currentPositionY)) {
			currentPositionX = -1.6 + rand() % 4;
			currentPositionY = -1.3 + rand() % 3;
		}
	}
}

class GLBox {
public:
	GLBox() {
		//create a clock for measuring time elapsed
		Clock = sf::Clock();

		ysize = xsize = RESOLUTION;

		//new SFML window and OpenGL context
		App = new sf::Window(sf::VideoMode(xsize, ysize, 32), "Maze");

		//log file to record any errors that come up

		FILE * logFile;
		logFile = fopen("log.txt", "wb");
		if (logFile == NULL) {
			printf("Unable to open log file. Exiting...\n");
			exit(2);
		}

		//need to detect which version of shaders are supported

		__glewInit(logFile);

		//this class handles shader creation for ARB and 2.0 shaders
		ShaderManager shaders = ShaderManager(logFile);

		//source code for maze shader
//		const char * mazeVertPath = "Shaders/maze.vert";
//		const char * mazeFragPath = "Shaders/maze.frag";
		const char * mazeVertPath = "Shaders/shader.vert";
		const char * mazeFragPath = "Shaders/shader.frag";
		GLint progMaze = shaders.buildShaderProgram(&mazeVertPath, &mazeFragPath, 1, 1);

		//source code for texture shader
		const char * textureVertPath = "Shaders/texture.vert";
		const char * textureFragPath = "Shaders/texture.frag";
		GLint progTex = shaders.buildShaderProgram(&textureVertPath, &textureFragPath, 1, 1);

		//setup render target texture
		//this will eventually hald the rendered scene and be
		//rendered to a quad for post process effects
		int numTex = 1;
		glGenTextures(numTex, &textureTarget);
		setupTargetTexture();

		//build the maze
		gfxinit();

		while (App->IsOpened()) {
			float targetFrameTime = 1.0f / (float) TARGET_FPS;
			float sleepTime = targetFrameTime - App->GetFrameTime();
			if (sleepTime > 0)
				sf::Sleep(sleepTime);

			App->SetActive();
			handleEvents();

			//glUseProgram(progMaze);
			if (displayMode == TRACKBALL)
				SpinCube2(0);

			//setShaderVariables(progMaze);

			//display(progMaze);

			//clear color and depth before rendering
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//render maze to framebuffer
			renderModel(progMaze);

			//render texture pass
			renderTexture(progTex);

			App->Display();
		}

		fclose(logFile);
	}

private:
	sf::Window *App;
	sf::Clock motionClock;
	sf::Clock motionClock2;
	float timeSinceMotion;
	sf::Clock Clock;
	GLint progMaze;
	GLuint textureTarget;

	void handleHorizontalCameraRotate(int direction) {
		currentAngle -= 2 * direction * 0.05;
		setLookAt();
	}

	void handleHorizontalCameraMove(int direction) {
		float newX = currentPositionX + cos(currentAngle) * direction * 0.05;
		float newY = currentPositionY + sin(currentAngle) * direction * 0.05;

		if (newX > 1.7) {
			newX = 1.7;
		}
		if (newX < -1.7) {
			newX = -1.7;
		}

		if (newY > 1.7) {
			newY = 1.7;
		}
		if (newY < -1.7) {
			newY = -1.7;
		}

		if (!isWall(newX, newY)) {
			currentPositionX = newX;
			currentPositionY = newY;
		} else {
			if (!isWall(newX, currentPositionY)) {
				currentPositionX = newX;
			} else {
				if (!isWall(currentPositionX, newY)) {
					currentPositionY = newY;
				}
			}
		}

		cout << "(" << currentPositionX << "," << currentPositionY << ")" << endl;
		setLookAt();
	}

	void handleEvents() {
		const sf::Input& Input = App->GetInput();
		bool shiftDown = Input.IsKeyDown(sf::Key::LShift) || Input.IsKeyDown(sf::Key::RShift);
		sf::Event Event;
		while (App->GetEvent(Event)) {
			// Close window : exit
			if (Event.Type == sf::Event::Closed)
				App->Close();

			// Escape key : exit
			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape))
				App->Close();

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Right)) {
				if (displayMode == MAZE)
					handleHorizontalCameraRotate(RIGHT);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Left)) {
				if (displayMode == MAZE)
					handleHorizontalCameraRotate(LEFT);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Up)) {
				if (displayMode == MAZE)
					handleHorizontalCameraMove(UP);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Down)) {
				if (displayMode == MAZE)
					handleHorizontalCameraMove(DOWN);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::M)) {
				//resetModelViewMatrix();
				glLoadIdentity();
				displayMode = (displayMode + 1) % 2;
				setLookAt();
			}

			if (displayMode == TRACKBALL) {
				if (Event.Type == sf::Event::MouseButtonPressed) {
					lastPos[0] = Event.MouseButton.X;
					lastPos[1] = Event.MouseButton.Y;

					if (Event.MouseButton.Button == sf::Mouse::Left && !shiftDown) {
						buttonDown[0] = 1;
						spin = FALSE;
					}
					if (Event.MouseButton.Button == sf::Mouse::Right)
						buttonDown[1] = 1;
					if (Event.MouseButton.Button == sf::Mouse::Middle)
						buttonDown[2] = 1;
					if (Event.MouseButton.Button == sf::Mouse::Left && shiftDown)
						buttonDown[2] = 1;
				}

				if (Event.Type == sf::Event::MouseButtonReleased) {
					if (Event.MouseButton.Button == sf::Mouse::Left && !shiftDown)
						buttonDown[0] = 0;
					if (Event.MouseButton.Button == sf::Mouse::Right)
						buttonDown[1] = 0;
					if (Event.MouseButton.Button == sf::Mouse::Middle)
						buttonDown[2] = 0;
					if (Event.MouseButton.Button == sf::Mouse::Left && shiftDown)
						buttonDown[2] = 0;

					timeSinceMotion = motionClock.GetElapsedTime();
					float maxTime = 1.0f / (float) TARGET_FPS * TIME_WINDOW;
					if (timeSinceMotion < maxTime)
						spin = TRUE;
				}

				if (Event.Type == sf::Event::MouseMoved && (buttonDown[0] || buttonDown[1] || buttonDown[2])) {
					int x = Event.MouseMove.X;
					int y = Event.MouseMove.Y;

					timeSinceMotion = motionClock.GetElapsedTime();
					motionClock.Reset();

					if (buttonDown[0])
						update_rotate(lastPos[0], lastPos[1], x, y);
					if (buttonDown[1])
						update_trans(lastPos[0], lastPos[1], x, y);
					if (buttonDown[2])
						update_z(lastPos[0], lastPos[1], x, y);

					lastPos[0] = x;
					lastPos[1] = y;
				}
			}
			if (Event.Type == sf::Event::Resized) {
				reshape(Event.Size.Width, Event.Size.Height);
				setupTargetTexture();
			}
		}
	}

	void setShaderVariables(GLuint shaderProg) {
		if (__GLEW_VERSION_2_0) {
			glUniform1f(glGetUniformLocation(shaderProg, "elapsedTime"), motionClock2.GetElapsedTime());
			glUniform1f(glGetUniformLocation(shaderProg, "cameraX"), currentPositionX);
			glUniform1f(glGetUniformLocation(shaderProg, "cameraY"), currentPositionY);

			glUniform1f(glGetUniformLocation(shaderProg, "elapsedTime"), Clock.GetElapsedTime());
			glUniform2f(glGetUniformLocation(shaderProg, "resolution"), xsize, ysize);
		} else {
			glUniform1fARB(glGetUniformLocationARB(shaderProg, "elapsedTime"), motionClock2.GetElapsedTime());
			glUniform1fARB(glGetUniformLocationARB(shaderProg, "cameraX"), currentPositionX);
			glUniform1fARB(glGetUniformLocationARB(shaderProg, "cameraY"), currentPositionY);

			glUniform1fARB(glGetUniformLocationARB(shaderProg, "elapsedTime"), Clock.GetElapsedTime());
			glUniform2fARB(glGetUniformLocationARB(shaderProg, "resolution"), xsize, ysize);
		}
	}

	void setupTargetTexture() {
		glBindTexture(GL_TEXTURE_2D, textureTarget);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xsize, ysize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	void renderModel(GLint program) {
		if (__GLEW_VERSION_2_0)
			glUseProgram(program);
		else
			glUseProgramObjectARB(program);
		setShaderVariables(program);
		display(program);
	}

	void renderTexture(GLint program) {
		//copy frame buffer to texture
		glBindTexture(GL_TEXTURE_2D, textureTarget);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, xsize, ysize, 0);

		//activate texture shader program
		if (__GLEW_VERSION_2_0)
			glUseProgram(program);
		else
			glUseProgramObjectARB(program);
		setShaderVariables(program);

		//prepare to render texture quad
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);

		//set shader variables to help with texture render
		int textureUnit = 0;
		if (__GLEW_VERSION_2_0) {
			glUniform1i(glGetUniformLocation(program, "texId"), textureUnit);
			glUniform2f(glGetUniformLocation(program, "resolution"), xsize, ysize);
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		} else {
			glUniform1iARB(glGetUniformLocationARB(program, "texId"), textureUnit);
			glUniform2fARB(glGetUniformLocationARB(program, "resolution"), xsize, ysize);
			glActiveTexture(GL_TEXTURE0 + textureUnit);
		}

		//bind texture
		glBindTexture(GL_TEXTURE_2D, textureTarget);

		//set viewport to entire window
		glViewport(0, 0, xsize, ysize);

		//matrices for full view quad render
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//render quad with texture vertices
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(0, 1);
		glVertex2f(0, 1);
		glEnd();
	}

	void __glewInit(FILE * logFile = NULL) const {
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			//Problem: glewInit failed, something is seriously wrong.
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			if(logFile!=NULL) fprintf(logFile, "Error: %s\n", glewGetErrorString(err));
		}
		else
		{
			printf("GLEW init finished...\n");
			if(logFile!=NULL) fprintf(logFile, "GLEW init finished...\n");
			if( __GLEW_VERSION_2_0 )
			{
				printf("OpenGL 2.0 is supported. Shaders should run correctly.\n");
				if(logFile!=NULL) fprintf(logFile, "OpenGL 2.0 is supported. Shaders should run correctly.\n");
			}
			else
			{
				printf("OpenGL 2.0 is NOT enabled. The program may not work correctly.\n");
				if(logFile!=NULL) fprintf(logFile,"OpenGL 2.0 is NOT enabled. The program may not work correctly.\n");
		}

		if( GLEW_ARB_vertex_program )
		{
			printf("ARB vertex programs supported.\n");
			if(logFile!=NULL) fprintf(logFile, "ARB vertex programs supported.\n");
		}
		else
		{
			printf("ARB vertex programs NOT supported. The program may not work correctly.\n");
			if(logFile!=NULL) fprintf(logFile,"ARB vertex programs NOT supported. The program may not work correctly.\n");
	            }
	            if( GLEW_ARB_fragment_program )
	            {
	                printf("ARB fragment programs supported.\n");
	                if(logFile!=NULL) fprintf(logFile, "ARB fragment programs supported.\n");
	            }
	            else
	            {
	                printf("ARB fragment programs NOT supported. The program may not work correctly.\n");
	                if(logFile!=NULL) fprintf(logFile, "ARB fragment programs NOT supported. The program may not work correctly.\n");
	            }
	        }
		}
	};

int main(int argc, char **argv) {
#ifdef __APPLE__
#define pathSize 5000
	char path[pathSize];
	uint32_t size = pathSize;
	_NSGetExecutablePath(path, &size);
	char *slashPos = strrchr(path, '/');
	slashPos[0] = '\0';
	chdir(path);
	chdir("../../../");
#endif

	/* check that there are sufficient arguments */
	if (argc < 3) {
		w = 5;
		h = 6;
		fprintf(stderr,"The width and height can be specified as command line arguments. Defaulting to %i %i\n", w, h);
	}
	else {
		w = atoi(argv[1]);
		h = atof(argv[2]);
	}

	GLBox prog;
	return EXIT_SUCCESS;
}
