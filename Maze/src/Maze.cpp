#include <stdio.h>
#include <stdlib.h>
#include <SFML/Window.hpp>
#include <cmath>
#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
using namespace std;

#define RESOLUTION 512

#define TRUE 1
#define FALSE 0

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

float lookat[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

GLfloat colors[][3] = { { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, {
		1.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 } };
GLfloat GRAY[3] = { 0.5, 0.5, 0.5 };
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

	Wall(GLfloat * newVertices) {
		memcpy(vertices, newVertices, 24 * sizeof(GLfloat));
	}

	Wall(Point2 * p1, Point2 * p2) {
		if (p1->x == p2->x) {
			// Horizontal line

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
			// Vertical line
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

	void draw_mesh(int a, int b, int c, int d) {
		glBegin(GL_POLYGON);
		{
			glColor3fv(colors[a]);
			glVertex3fv(vertices[a]);
			glColor3fv(colors[b]);
			glVertex3fv(vertices[b]);
			glColor3fv(colors[c]);
			glVertex3fv(vertices[c]);
			glColor3fv(colors[d]);
			glVertex3fv(vertices[d]);
		}
		glEnd();
	}

	void draw() {
		draw_mesh(1, 0, 3, 2);
		draw_mesh(3, 7, 6, 2);
		draw_mesh(7, 3, 0, 4);
		draw_mesh(2, 6, 5, 1);
		draw_mesh(4, 5, 6, 7);
		draw_mesh(5, 4, 0, 1);
	}

};

/* global parameters */
int w, h, edges, perimeters, vertices, groups, *group = NULL, redges, done = 0;
Edge *edge = NULL, *perimeter = NULL;
Point2 *vertex = NULL;
vector<Wall> walls;

// Storage space for the various transformations we'll need
float translation1[16], translation2[16], rotation[16], inc_rotation[16];

#if 0
GLfloat verticesTrackBall[][3] = { {-1.0, -1.0, -1.0},
	{	1.0, -1.0, -1.0},
	{	1.0, 1.0, -1.0},
	{	-1.0, 1.0, -1.0},
	{	-1.0, -1.0, 1.0},
	{	1.0, -1.0, 1.0},
	{	1.0, 1.0, 1.0},
	{	-1.0, 1.0, 1.0}};
#else
//GLfloat verticesTrackBall[][3] = { { 0.0, 0.0, 0.0 }, { 2.0, 0.0, 0.0 }, { 2.0, 2.0, 0.0 }, { 0.0, 2.0, 0.0 }, { 0.0,
//		0.0, 2.0 }, { 2.0, 0.0, 2.0 }, { 2.0, 2.0, 2.0 }, { 0.0, 2.0, 2.0 } };

GLfloat verticesTrackBall[][3] = { { 0.0, 0.0, 0.0 }, { w, 0.0, 0.0 }, { w, h, 0.0 }, { 0.0, h, 0.0 }, { 0.0, 0.0,
		WALL_HEIGHT }, { w, 0.0, WALL_HEIGHT }, { w, h, WALL_HEIGHT }, { 0.0, h, WALL_HEIGHT } };
#endif

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
		for (j = 0; j < 2; j++) {
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

void build_maze() {
	done = 0;
	/* build maze */
	init_maze(w, h);
	while (!done) {
		/* remove one edge */
		remove_one_edge();
	}
	create_walls();
}

void draw_maze(void) {
	GLuint i;

	for (i = 0; i < walls.size(); i++) {
		walls[i].draw();
	}
}

//// draw a rectangle using the index of each vertex and color
//void draw_polygon(int a, int b, int c, int d, int face) {
//	glBegin(GL_POLYGON);
//	{
//		glColor3fv(colors[a]);
//		glVertex3fv(verticesTrackBall[a]);
//		glColor3fv(colors[b]);
//		glVertex3fv(verticesTrackBall[b]);
//		glColor3fv(colors[c]);
//		glVertex3fv(verticesTrackBall[c]);
//		glColor3fv(colors[d]);
//		glVertex3fv(verticesTrackBall[d]);
//	}
//	glEnd();
//}
//
//// draw a cube
//void draw_color_cube(void) {
//	draw_polygon(1, 0, 3, 2, 0);
//	draw_polygon(3, 7, 6, 2, 1);
//	draw_polygon(7, 3, 0, 4, 2);
//	draw_polygon(2, 6, 5, 1, 3);
//	draw_polygon(4, 5, 6, 7, 4);
//	draw_polygon(5, 4, 0, 1, 5);
//}

void resetModelViewMatrix() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void setLookAt() {
	if (displayMode != MAZE) {
		gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 9.0, 0.0, 1.0, 0.0);
	} else {
		gluLookAt(lookat[0], lookat[1], lookat[2], lookat[3], lookat[4], lookat[5], lookat[6], lookat[7], lookat[8]);
	}
}

void resetAndApplyAllTransforms() {
	// Clean any mess
	resetModelViewMatrix();

	// Set the viewing to the same value as came in default
	setLookAt();

	glMultMatrixf(translation2);
	glMultMatrixf(rotation);
	glMultMatrixf(translation1);
}

void incrementRotation() {
	// Clean any mess
	resetModelViewMatrix();

	// Accumulate rotation and increment
	glMultMatrixf(inc_rotation);
	glMultMatrixf(rotation);

	// Save for future use
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) rotation);
}

void drawFloor() {
	glColor3fv(GRAY);
	glBegin(GL_QUADS);
	{
		glVertex3f(-2.0, -2.0, -0.001);
		glVertex3f(-2.0, 2.0, -0.001);
		glVertex3f(2.0, 2.0, -0.001);
		glVertex3f(2.0, -2.0, -0.001);
	}
	glEnd();

}
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	resetAndApplyAllTransforms();
	draw_maze();
	drawFloor();
}

// update the modelview matrix with a new translation in the z direction
void update_z(int x1, int y1, int x2, int y2) {
	// Lets calculate the translation amount
	GLfloat translationOnZ = (y1 - y2) * Z_SENSITIVITY;

	// Apply it!
	// First, lets reset the modelview back to default
	resetModelViewMatrix();

	// Then, lets restore only what we had for translation before
	glMultMatrixf(translation2);

	// Now lets apply the new Z translation on top of that
	glTranslatef(0, 0, translationOnZ);

	// We now have the new accumulated translation values, lets save it back so we can use it later
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation2);
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
	glMultMatrixf(translation2);

	// Now lets apply the new X and Y translation on top of that
	glTranslatef(translationOnX, translationOnY, 0);

	// We now have the new accumulated translation values, lets save it back so we can use it later
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation2);
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
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) inc_rotation);

	// Now, apply the incremental factor on what we already had for rotation
	glMultMatrixf(rotation);

	// Finally, save the accumulated total rotation for future use
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*) rotation);
}

// function used to spin the object.
void SpinCube2(int t) {
	if (spin) {  // if we're still spinning, increment the rotation
		incrementRotation();
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	xsize = w;
	ysize = h;

	// default aspectRatio was 1.0
	GLfloat newAspectRatio = ((float) xsize) / ((float) ysize);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, newAspectRatio, 0.1, 100);
}

void gfxinit() {
	if (displayMode == MAZE) {
		int i;
		float x = 0, y = 0, z = 0;

		glEnable(GL_DEPTH_TEST);

		// initialize the projection stack
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, 1.0, 0.1, 100);

		// initialize the modelview stack
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation2);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &rotation);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &inc_rotation);
		for (i = 0; i < 8; i++) {
			x += verticesTrackBall[i][0];
			y += verticesTrackBall[i][1];
			z += verticesTrackBall[i][2];
		}
		x /= 8;
		y /= 8;
		z /= 8;
		glTranslatef(-x, -y, -z);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation1);
		glLoadIdentity();

		lookat[0] = -1.8 + (1.8 / w);
		lookat[1] = -1.8 + (1.8 / h);
		lookat[2] = WALL_HEIGHT / 2.0;

		lookat[3] = 1.8;
		lookat[4] = -1.8 + (1.8 / h);
		lookat[5] = WALL_HEIGHT / 2.0;

		lookat[6] = 0.0;
		lookat[7] = 0.0;
		lookat[8] = 1.0;
		setLookAt();

		glMultMatrixf(translation1);

		glViewport(0, 0, RESOLUTION, RESOLUTION);
		xsize = RESOLUTION;
		ysize = RESOLUTION;

		build_maze();
	} else {
		int i;
		float x = 0, y = 0, z = 0;

		glEnable(GL_DEPTH_TEST);

		// initialize the projection stack
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(30, 1.0, 0.1, 100);

		// initialize the modelview stack
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation2);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &rotation);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &inc_rotation);
		for (i = 0; i < 8; i++) {
			x += verticesTrackBall[i][0];
			y += verticesTrackBall[i][1];
			z += verticesTrackBall[i][2];
		}
		x /= 8;
		y /= 8;
		z /= 8;
		glTranslatef(-x, -y, -z);
		glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &translation1);
		glLoadIdentity();
		setLookAt();
		glMultMatrixf(translation1);

		glViewport(0, 0, RESOLUTION, RESOLUTION);
		xsize = RESOLUTION;
		ysize = RESOLUTION;

		build_maze();
	}
}

class GLBox {
public:
	GLBox() {
		App = new sf::Window(sf::VideoMode(RESOLUTION, RESOLUTION, 32), "Trackball");

		gfxinit();

		while (App->IsOpened()) {
			float targetFrameTime = 1.0f / (float) TARGET_FPS;
			float sleepTime = targetFrameTime - App->GetFrameTime();
			if (sleepTime > 0)
				sf::Sleep(sleepTime);

			App->SetActive();

			handleEvents();
			SpinCube2(0);
			display();

			App->Display();
		}
	}

private:
	sf::Window *App;
	sf::Clock motionClock;
	float timeSinceMotion;

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
//				handleHorizontalCameraRotate(RIGHT);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Left)) {
//				handleHorizontalCameraRotate(LEFT);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Up)) {
//				handleHorizontalCameraMove(UP);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Down)) {
//				handleHorizontalCameraMove(DOWN);
			}

			if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::M)) {
				displayMode = (displayMode + 1) % 2;
			}

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

			if (Event.Type == sf::Event::Resized) {
				reshape(Event.Size.Width, Event.Size.Height);
			}
		}
	}
};

int main(int argc, char **argv) {
	/* check that there are sufficient arguments */
	if (argc < 3) {
		w = 6;
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
