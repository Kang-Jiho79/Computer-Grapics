#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);

GLclampf color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
bool SetTime = false;

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
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

float getRandomFloat()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	return dis(gen);
}

GLvoid drawScene()
{
	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'c': {
		color[0] = 0.0f; color[1] = 1.0f; color[2] = 1.0f;
		break;
	}
	case 'm': {
		color[0] = 1.0f; color[1] = 0.0f; color[2] = 1.0f;
		break;
	}
	case 'y': {
		color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f;
		break;
	}
	case 'a': {
		color[0] = getRandomFloat(); color[1] = getRandomFloat(); color[2] = getRandomFloat();
		break;
	}
	case 'w': {
		color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
		break;
	}
	case 'k': {
		color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.0f;
		break;
	}
	case 't': {
		SetTime = true;
		break;
	}
	case 's': {
		SetTime = false;
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
	if (SetTime) {
		color[0] = getRandomFloat(); color[1] = getRandomFloat(); color[2] = getRandomFloat();
	}
	glutTimerFunc(100, TimerFunction, 1);
}