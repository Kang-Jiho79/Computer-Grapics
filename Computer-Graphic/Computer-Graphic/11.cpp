#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <cmath>

#define MaxShapes 5
#define PI 3.14159265359f

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;


class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;
	float dx = 0.0f;
	float dy = 0.0f;
	float centerX = 0.0f;  // 중심 좌표
	float centerY = 0.0f;
	float centerdX = 0.0f; // 중심 이동 속도
	float centerdY = 0.0f;
	GLuint VAO, VBO[2];
	int type; // 0:생성안됨 1:생성중 2:생성됨
	float angle = 0.0f; // CIRCLESPIRAL용 각도
	float spiralLevel = 0; // CIRCLESPIRAL용 레벨
	bool spiralDirection = false; 
};

Shape shapes[MaxShapes];
float x_ndc = 0.0f, y_ndc = 0.0f;
int existingShapes = 0;
bool line = false;

GLclampf color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

GLvoid initBuffer(Shape& shape);

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

float getRandomfloat(float min = 0.05f, float max = 0.1f)
{
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}


void clear()
{
	for (int i = 0; i < existingShapes; i++)
	{
		Shape& shape = shapes[i];
		if (shape.VAO != 0) {
			glDeleteVertexArrays(1, &shape.VAO);
			glDeleteBuffers(2, shape.VBO);
			shape.VAO = 0;
		}
		// 완전한 초기화 추가
		shape.vertices.clear();
		shape.colors.clear();
		shape.type = 0;
		shape.angle = 0.0f;
		shape.spiralLevel = 0.0f;
		shape.spiralDirection = false;
		shape.centerX = 0.0f;
		shape.centerY = 0.0f;
		shape.centerdX = 0.0f;
		shape.centerdY = 0.0f;
	}
	existingShapes = 0;
}

void main(int argc, char** argv)
{
	width = 500;
	height = 500;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Rotating Triangle Animation");
	glewExperimental = GL_TRUE;
	glewInit();

	clear();

	make_shaderProgram();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, TimerFunction, 1); // 60fps
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

void initBuffer(Shape& shape)
{
	glGenVertexArrays(1, &shape.VAO);
	glBindVertexArray(shape.VAO);
	glGenBuffers(2, shape.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

GLvoid drawScene()
{
	glClearColor(color[0], color[1], color[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 수정: 올바른 루프
	for (int i = 0; i < existingShapes; i++) {
		Shape& shape = shapes[i];
		glBindVertexArray(shape.VAO);
		if (line)
			glDrawArrays(GL_LINE_STRIP, 0, shape.vertices.size()/3);
		else {
			glPointSize(5.0f);
			glDrawArrays(GL_POINTS, 0, shape.vertices.size()/3);
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
	case '1':
		existingShapes = 1;
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].type == 0) {
				Shape& shape = shapes[i];
				shape.angle = 0.0f;
				shape.spiralLevel = 0.05f; // 시작 레벨
				shape.spiralDirection = false; // 바깥쪽으로 시작
				shape.centerX = getRandomfloat(-0.8f, 0.8f);
				shape.centerY = getRandomfloat(-0.8f, 0.8f);
				shape.centerdX = shape.centerX;
				shape.centerdY = shape.centerY;
				shape.vertices.clear();
				shape.type = 1; // 생성중
				color[0] = getRandomcolor(); color[1] = getRandomcolor(); color[2] = getRandomcolor();
			}
		}
		break;
	case '2':
		existingShapes = 2;
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].type == 0) {
				Shape& shape = shapes[i];
				shape.angle = 0.0f;
				shape.spiralLevel = 0.05f; // 시작 레벨
				shape.spiralDirection = false; // 바깥쪽으로 시작
				shape.centerX = getRandomfloat(-0.8f, 0.8f);
				shape.centerY = getRandomfloat(-0.8f, 0.8f);
				shape.centerdX = shape.centerX;
				shape.centerdY = shape.centerY;
				shape.vertices.clear();
				shape.type = 1; // 생성중
				color[0] = getRandomcolor(); color[1] = getRandomcolor(); color[2] = getRandomcolor();
			}
		}
		break;
	case '3':
		existingShapes = 3;
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].type == 0) {
				Shape& shape = shapes[i];
				shape.angle = 0.0f;
				shape.spiralLevel = 0.05f; // 시작 레벨
				shape.spiralDirection = false; // 바깥쪽으로 시작
				shape.centerX = getRandomfloat(-0.8f, 0.8f);
				shape.centerY = getRandomfloat(-0.8f, 0.8f);
				shape.centerdX = shape.centerX;
				shape.centerdY = shape.centerY;
				shape.vertices.clear();
				shape.type = 1; // 생성중
				color[0] = getRandomcolor(); color[1] = getRandomcolor(); color[2] = getRandomcolor();
			}
		}
		break;
	case '4':
		existingShapes = 4;
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].type == 0) {
				Shape& shape = shapes[i];
				shape.angle = 0.0f;
				shape.spiralLevel = 0.05f; // 시작 레벨
				shape.spiralDirection = false; // 바깥쪽으로 시작
				shape.centerX = getRandomfloat(-0.8f, 0.8f);
				shape.centerY = getRandomfloat(-0.8f, 0.8f);
				shape.centerdX = shape.centerX;
				shape.centerdY = shape.centerY;
				shape.vertices.clear();
				shape.type = 1; // 생성중
				color[0] = getRandomcolor(); color[1] = getRandomcolor(); color[2] = getRandomcolor();
			}
		}
		break;
	case '5':
		existingShapes = 5;
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].type == 0) {
				Shape& shape = shapes[i];
				shape.angle = 0.0f;
				shape.spiralLevel = 0.05f; // 시작 레벨
				shape.spiralDirection = false; // 바깥쪽으로 시작
				shape.centerX = getRandomfloat(-0.8f, 0.8f);
				shape.centerY = getRandomfloat(-0.8f, 0.8f);
				shape.centerdX = shape.centerX;
				shape.centerdY = shape.centerY;
				shape.vertices.clear();
				shape.type = 1; // 생성중
				color[0] = getRandomcolor(); color[1] = getRandomcolor(); color[2] = getRandomcolor();
			}
		}
		break;
	case 'p':
		line = false;
		break;
	case 'l':
		line = true;
		break;
	case 'c':
		clear();
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
	static int time = 0;
	time += 1; // 60fps

	for (int i = 0; i < existingShapes; i++) {
		if (shapes[i].type == 1) {
			Shape& shape = shapes[i];
			float x = cos(shape.angle) * shape.spiralLevel + (shape.spiralDirection ? shape.centerdX : shape.centerX);
			float y = sin(shape.angle) * shape.spiralLevel + (shape.spiralDirection ? shape.centerdY : shape.centerY);
			if (shape.spiralDirection) {
				shape.spiralLevel -= 0.001f; // 안쪽으로 이동
				shape.angle -= 0.05f;
				if (shape.angle < 2.0f) {
					shape.type = 2; // 생성 완료
				}
			}
			else {
				shape.spiralLevel += 0.001f; // 바깥쪽으로 이동
				shape.angle += 0.05f;
				if (shape.angle >= 10.0f && shape.centerY - y <= 0.005f) {
					shape.angle += PI;
					shape.spiralDirection = true; // 안쪽으로 전환
					shape.centerdX = 2 * x - shape.centerX;
					shape.centerdY = 2 * y - shape.centerY;
				}
			}
			if (time % 3 == 0 && shape.type == 1) {
				shape.vertices.push_back(x);
				shape.vertices.push_back(y);
				shape.vertices.push_back(0.0f);
				shape.colors.push_back(0.0f);
				shape.colors.push_back(0.0f);
				shape.colors.push_back(0.0f);
				initBuffer(shape);
			}
		}
	}
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}

