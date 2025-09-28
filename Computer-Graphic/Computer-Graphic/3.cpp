#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

#define maxrectcount 100

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLclampf backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

bool SetTime = false;

float getRandomcolor()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

float getRandomfloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(-0.8f, 0.8f);
	return dis(gen);
}

class rect {
public:
	GLclampf color[4];
	float x, y, w, h;
	bool exist = false;
	int combinecount = 0;

	void makeRectangles() {
		color[0] = getRandomcolor();
		color[1] = getRandomcolor();
		color[2] = getRandomcolor();
		color[3] = 1.0f;
		x = getRandomfloat();
		y = getRandomfloat();
		w = fabs(getRandomfloat() / 2) + 0.1;
		h = fabs(getRandomfloat() / 2) + 0.1;
		exist = true;
		combinecount = 0;
	}

	void init() {
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 1.0f;
		x = 0.0f;
		y = 0.0f;
		w = 0.0f;
		h = 0.0f;
		exist = false;
		combinecount = 0;
	}
};
rect rectangles[maxrectcount];
int rectCount = 0;
bool isDragging = false;
int draggingRect = -1;
float lastX, lastY;

void crashrect(int dragingrect)
{
	for (int i =0 ; i < maxrectcount; i++) {
		if (i != dragingrect && rectangles[i].exist) {
			if (rectangles[dragingrect].x < rectangles[i].x + rectangles[i].w &&
				rectangles[dragingrect].x + rectangles[dragingrect].w > rectangles[i].x &&
				rectangles[dragingrect].y < rectangles[i].y + rectangles[i].h &&
				rectangles[dragingrect].y + rectangles[dragingrect].h > rectangles[i].y) {
				rectangles[i].color[0] = getRandomcolor();
				rectangles[i].color[1] = getRandomcolor();
				rectangles[i].color[2] = getRandomcolor();
				if (rectangles[i].x > rectangles[dragingrect].x) {
					if (rectangles[i].x + rectangles[i].w > rectangles[dragingrect].x + rectangles[dragingrect].w) {
						rectangles[i].w = (rectangles[i].x + rectangles[i].w) - rectangles[dragingrect].x;
					}
					else {
						rectangles[i].w = rectangles[dragingrect].w;
					}
					rectangles[i].x = rectangles[dragingrect].x;
				}
				else {
					if (rectangles[i].x + rectangles[i].w > rectangles[dragingrect].x + rectangles[dragingrect].w) {
						rectangles[i].w = rectangles[i].w;
					}
					else {
						rectangles[i].w = (rectangles[dragingrect].x + rectangles[dragingrect].w) - rectangles[i].x;
					}
				}
				if (rectangles[i].y > rectangles[dragingrect].y) {
					if (rectangles[i].y + rectangles[i].h > rectangles[dragingrect].y + rectangles[dragingrect].h) {
						rectangles[i].h = (rectangles[i].y + rectangles[i].h) - rectangles[dragingrect].y;
					}
					else
						rectangles[i].h = rectangles[dragingrect].h;
					rectangles[i].y = rectangles[dragingrect].y;
				}
				else {
					if (rectangles[i].y + rectangles[i].h > rectangles[dragingrect].y + rectangles[dragingrect].h) {
						rectangles[i].h = rectangles[i].h;
					}
					else
						rectangles[i].h = (rectangles[dragingrect].y + rectangles[dragingrect].h) - rectangles[i].y;
				}
				
				rectangles[i].combinecount += ++rectangles[dragingrect].combinecount;
				rectCount--;
				rectangles[dragingrect].init();
				isDragging = false;
				draggingRect = -1;
				break;
			}
		}
	}
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Example1");
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	for (int i = 0; i < maxrectcount; i++) {
		rectangles[i].init();
	}
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, TimerFunction, 1);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 0; i < maxrectcount; i++) {
		if (rectangles[i].exist) {
			glColor3f(rectangles[i].color[0], rectangles[i].color[1], rectangles[i].color[2]);
			glRectf(rectangles[i].x, rectangles[i].y, rectangles[i].x + rectangles[i].w, rectangles[i].y + rectangles[i].h);
		}
	}
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a': {
		if (rectCount < 10) {
			for (int i = 0; i < maxrectcount; i++) {
				if (!rectangles[i].exist) {
					rectangles[i].makeRectangles();
					rectCount++;
					break;
				}
			}
		}
		break;
	}
	case 's': {
		printf("draggingRect: %d\n", draggingRect);
		for (int i = 0; i < maxrectcount; i++) {
			if (rectangles[i].exist) {
				printf("rect %d: color(%f, %f, %f), position(%f, %f), size(%f, %f), combinecount(%d)\n",
					i,
					rectangles[i].color[0], rectangles[i].color[1], rectangles[i].color[2],
					rectangles[i].x, rectangles[i].y,
					rectangles[i].w, rectangles[i].h,
					rectangles[i].combinecount);
			}
		}
		break;
	}

	case 'q': {
		glutLeaveMainLoop();
		break;
	}
	}
	glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
	glutPostRedisplay();
	glutTimerFunc(100, TimerFunction, 1);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float x_ndc = (1.0f * x / 400 - 1.0f);
	float y_ndc = -(1.0f * y / 300 - 1.0f);
	printf("mouse down at %f %f\n", x_ndc, y_ndc);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		draggingRect = -1;
		for (int i = 0; i < maxrectcount; i++) {
			if (rectangles[i].exist &&
				x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
				y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
				printf("dragging rect %d\n", i);
				draggingRect = i;
				isDragging = true;
				lastX = x_ndc;
				lastY = y_ndc;
				break;
			}
		}
		glutPostRedisplay();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isDragging = false;
		draggingRect = -1;
		glutPostRedisplay();
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < maxrectcount; i++) {
			if (rectangles[i].exist &&
				x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
				y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
				if (rectangles[i].combinecount > 0) {
					rectangles[i].color[0] = getRandomcolor();
					rectangles[i].color[1] = getRandomcolor();
					rectangles[i].color[2] = getRandomcolor();
					rectangles[i].w = fabs(getRandomfloat() / 2) + 0.1;
					rectangles[i].h = fabs(getRandomfloat() / 2) + 0.1;
					rectangles[i].combinecount--;
					for (int j = 0; j < maxrectcount; j++) {
						if (!rectangles[j].exist) {
							rectangles[j].makeRectangles();
							rectangles[j].x = rectangles[i].x + rectangles[i].w + 0.02;
							rectangles[j].y = rectangles[i].y;
							break;
						}
					}
				}
			}
		}
		glutPostRedisplay();
	}
}
GLvoid Motion(int x, int y)
{
	if (isDragging && draggingRect != -1) {
		float x_ndc = (1.0f * x / 400 - 1.0f);
		float y_ndc = -(1.0f * y / 300 - 1.0f);
		rectangles[draggingRect].x += (x_ndc - lastX);
		rectangles[draggingRect].y += (y_ndc - lastY);
		printf("dragging rect %d to %f %f\n", draggingRect, rectangles[draggingRect].x, rectangles[draggingRect].y);

		
		crashrect(draggingRect);
		
		lastX = x_ndc;
		lastY = y_ndc;
		glutPostRedisplay();
	}
}