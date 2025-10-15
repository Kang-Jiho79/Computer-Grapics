#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
GLvoid Motion(int x, int y);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;
	std::vector<int> index;
	float cx = 0.0f, cy = 0.0f, cz = 0.0f;
	float size = 0.5f;
	GLuint VAO, VBO[2], EBO;
};

Shape axis;
Shape cube;
Shape pyramid;

GLvoid initBuffer(Shape& shape);

void createAxis(Shape& shape)
{
	shape.vertices = {
		1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f
	};
	shape.colors = {
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};
	shape.index = {
		0, 1,
		2, 3,
		4, 5
	};
	initBuffer(shape);
}

void createCube(Shape& shape)
{
	shape.vertices = {
		// 앞면
		-0.5f, -0.5f, 0.5f,   // 0
		 0.5f, -0.5f, 0.5f,   // 1
		 0.5f,  0.5f, 0.5f,   // 2
		-0.5f,  0.5f, 0.5f,   // 3
		// 뒷면
		-0.5f, -0.5f, -0.5f,  // 4
		 0.5f, -0.5f, -0.5f,  // 5
		 0.5f,  0.5f, -0.5f,  // 6
		-0.5f,  0.5f, -0.5f   // 7
	};
	shape.colors = {
		1.0f, 0.0f, 0.0f,  
		0.0f, 1.0f, 0.0f,   
		0.0f, 0.0f, 1.0f,  
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 
		0.0f, 1.0f, 1.0f,  
		1.0f, 1.0f, 1.0f, 
		0.5f, 0.5f, 0.5f
	};
	shape.index = {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		2, 6, 7, 7, 3, 2,
		0, 3, 7, 7, 4, 0,
		0, 4, 5, 5, 1, 0,
		4, 7, 6, 6, 5, 4
	};
}

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

void main(int argc, char** argv)
{
	width = 500;
	height = 500;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Triangle Animation");
	glewExperimental = GL_TRUE;
	glewInit();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, Timer, 0);
	glutMainLoop();
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("fragment.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_shaderProgram()
{
	make_vertexShaders();
	make_fragmentShaders();

	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgramID);
}

GLvoid initBuffer(Shape& shape)
{
	glGenVertexArrays(1, &shape.VAO);
	glBindVertexArray(shape.VAO);
	glGenBuffers(2, shape.VBO);
	glGenBuffers(1, &shape.EBO);

	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(int), shape.index.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q': // 종료
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}	