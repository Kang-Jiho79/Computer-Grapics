#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);

GLclampf backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

bool SetTime = false;

struct rect {
	GLclampf color[4];
	float x, y, w, h;
};
rect rectangles[4];

float getRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

void initRectangles() {
	for (int i = 0; i < 4; i++) {
		rectangles[i].color[0] = getRandomFloat();
		rectangles[i].color[1] = getRandomFloat();
		rectangles[i].color[2] = getRandomFloat();
		rectangles[i].color[3] = 1.0f;
		rectangles[i].x = i % 2 - 1.0f;
		rectangles[i].y = i < 2 ? 0.0f : -1.0f;
		rectangles[i].w = 1.0f;
		rectangles[i].h = 1.0f;
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
	initRectangles();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, TimerFunction, 1);
	glutMouseFunc(Mouse);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 0; i < 4; i++) {
		glColor3f(rectangles[i].color[0], rectangles[i].color[1], rectangles[i].color[2]);
		glRectf(rectangles[i].x, rectangles[i].y, rectangles[i].x + rectangles[i].w, rectangles[i].y + rectangles[i].h);
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		for (int i = 0; i < 4; i++) {
			if (x_ndc >= i % 2 - 1.0f && x_ndc <= i % 2 &&
				y_ndc >= (i < 2 ? 0.0f : -1.0f) && y_ndc <= (i < 2 ? 1.0f : 0.0f)) {
				if (x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
					y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
					rectangles[i].color[0] = getRandomFloat();
					rectangles[i].color[1] = getRandomFloat();
					rectangles[i].color[2] = getRandomFloat();
					break;
				}
				else {
					backgroundColor[0] = getRandomFloat();
					backgroundColor[1] = getRandomFloat();
					backgroundColor[2] = getRandomFloat();
					break;
				}
			}
		}
		glutPostRedisplay();
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

		for (int i = 0; i < 4; i++) {
			if (x_ndc >= i % 2 - 1.0f && x_ndc <= i % 2 &&
				y_ndc >= (i < 2 ? 0.0f : -1.0f) && y_ndc <= (i < 2 ? 1.0f : 0.0f)) {
				if (x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
					y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
					if (rectangles[i].w > 0.2f && rectangles[i].h > 0.2f) {
						rectangles[i].x += 0.05f;
						rectangles[i].y += 0.05f;
						rectangles[i].w -= 0.1f;
						rectangles[i].h -= 0.1f;
						printf("Rectangle %d resized to (%f, %f, %f, %f)\n", i, rectangles[i].x, rectangles[i].y, rectangles[i].w, rectangles[i].h);
						break;
					}
				}
				else {
					if (rectangles[i].w < 1.0f && rectangles[i].h < 1.0f) {
						rectangles[i].x -= 0.05f;
						rectangles[i].y -= 0.05f;
						rectangles[i].w += 0.1f;
						rectangles[i].h += 0.1f;
						break;
					}
				}
			}
		}
		glutPostRedisplay();
	}
}