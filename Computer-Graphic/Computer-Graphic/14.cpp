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
	float cx = 0.0f, cy = 0.0f;
	float dx = 0.005f, dy = 0.005f;
	GLuint VAO, VBO[2];
	float size = 0.15f;
	int type = 1; // 1: 다이아몬드형, 2: X자형
	float rotation = 0.0f;
	bool animate = false;
	float angle = 0.0f;  // 궤도 회전각도
	float selfRotation = 0.0f; // 자체 회전각도 추가
	float radius = 0.0f; // 회전 반지름
	float centerX = 0.0f, centerY = 0.0f; // 회전 중심점
	int rotationDirection = 1; // 1: 시계방향, -1: 반시계방향
};

Shape shapes[4];
float x_ndc = 0.0f, y_ndc = 0.0f;
bool animation = true;

GLvoid initBuffer(Shape& shape);

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

void rotatePoint(float& x, float& y, float cx, float cy, float angle)
{
	float cosA = cos(angle);
	float sinA = sin(angle);
	float dx = x - cx;
	float dy = y - cy;

	x = cx + dx * cosA - dy * sinA;
	y = cy + dx * sinA + dy * cosA;
}

void make_triangle(Shape& shape, int index)
{
	float color[3] = { 1.0f, 0.8f, 0.0f };
	shape.colors.clear();
	shape.vertices.clear();

	float size = shape.size;
	std::vector<float> baseVertices;

	if (shape.type == 1) { 
		switch (index) {
		case 1: 
			baseVertices = {
				shape.cx, shape.cy + size, 0.0f,
				shape.cx - size * 0.7f, shape.cy, 0.0f,
				shape.cx + size * 0.7f, shape.cy, 0.0f
			};
			break;
		case 3: 
			baseVertices = {
				shape.cx, shape.cy - size, 0.0f,
				shape.cx - size * 0.7f, shape.cy, 0.0f,
				shape.cx + size * 0.7f, shape.cy, 0.0f
			};
			break;
		case 2: 
			baseVertices = {
				shape.cx - size, shape.cy, 0.0f,
				shape.cx, shape.cy - size * 0.7f, 0.0f,
				shape.cx, shape.cy + size * 0.7f, 0.0f
			};
			break;
		case 0: 
			baseVertices = {
				shape.cx + size, shape.cy, 0.0f,
				shape.cx, shape.cy - size * 0.7f, 0.0f,
				shape.cx, shape.cy + size * 0.7f, 0.0f
			};
			break;
		}
	}
	else if (shape.type == 2) { 
		switch (index) {
		case 3: 
			baseVertices = {
				shape.cx, shape.cy + size, 0.0f,
				shape.cx - size * 0.7f, shape.cy, 0.0f,
				shape.cx + size * 0.7f, shape.cy, 0.0f
			};
			break;
		case 1: 
			baseVertices = {
				shape.cx, shape.cy - size, 0.0f,
				shape.cx - size * 0.7f, shape.cy, 0.0f,
				shape.cx + size * 0.7f, shape.cy, 0.0f
			};
			break;
		case 0: 
			baseVertices = {
				shape.cx - size, shape.cy, 0.0f,
				shape.cx, shape.cy - size * 0.7f, 0.0f,
				shape.cx, shape.cy + size * 0.7f, 0.0f
			};
			break;
		case 2: 
			baseVertices = {
				shape.cx + size, shape.cy, 0.0f,
				shape.cx, shape.cy - size * 0.7f, 0.0f,
				shape.cx, shape.cy + size * 0.7f, 0.0f
			};
			break;
		}
	}
	shape.vertices = baseVertices;
	for (int i = 0; i < baseVertices.size(); i += 3) {
		rotatePoint(shape.vertices[i], shape.vertices[i + 1], shape.cx, shape.cy, shape.selfRotation);
	}

	for (int i = 0; i < 3; i++) {
		shape.colors.push_back(color[0]);
		shape.colors.push_back(color[1]);
		shape.colors.push_back(color[2]);
	}

	initBuffer(shape);
}

void updateShapeVertices(Shape& shape, int index)
{
	make_triangle(shape, index);
}

void move_shape(Shape& shape, int index)
{
	if (!shape.animate) return;

	shape.angle += 0.05f * shape.rotationDirection;

	shape.selfRotation += 0.05f * shape.rotationDirection;

	shape.cx = shape.centerX + shape.radius * cos(shape.angle);
	shape.cy = shape.centerY + shape.radius * sin(shape.angle);

	updateShapeVertices(shape, index);
}

void initshapes()
{
	for (int i = 0; i < 4; i++) {
		shapes[i].centerX = 0.0f;
		shapes[i].centerY = 0.0f;
		shapes[i].radius = 0.2f;
		shapes[i].angle = i * 90.0f * 3.14159f / 180.0f;
		shapes[i].cx = shapes[i].centerX + shapes[i].radius * cos(shapes[i].angle);
		shapes[i].cy = shapes[i].centerY + shapes[i].radius * sin(shapes[i].angle);
		shapes[i].type = 1; 
		shapes[i].animate = false;
		shapes[i].rotationDirection = 1;
		shapes[i].selfRotation = 0.0f;
		make_triangle(shapes[i], i);
	}
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

	initshapes();
	make_shaderProgram();

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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 4개의 삼각형 그리기
	for (int i = 0; i < 4; i++) {
		glBindVertexArray(shapes[i].VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
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
	case 'c': // 중심에 가운데 중심점을 기준으로 시계방향으로 삼각형들이 회전한다
		for (int i = 0; i < 4; i++) {
			shapes[i].animate = true;
			shapes[i].rotationDirection = 1; // 시계방향
		}
		break;
	case 't': // 중심에 가운데 중심점을 기준으로 반시계 방향으로 삼각형들이 회전한다
		for (int i = 0; i < 4; i++) {
			shapes[i].animate = true;
			shapes[i].rotationDirection = -1; // 반시계방향
		}
		break;
	case 's': // 회전 애니메이션을 멈춘다/다시 회전한다
		for (int i = 0; i < 4; i++) {
			shapes[i].animate = !shapes[i].animate;
		}
		break;
	case '1': // 도형 1 (다이아몬드형)으로 변경
		for (int i = 0; i < 4; i++) {
			shapes[i].type = 1;
			make_triangle(shapes[i], i);
		}
		break;
	case '2': // 도형 2 (X자형)으로 변경
		for (int i = 0; i < 4; i++) {
			shapes[i].type = 2;
			make_triangle(shapes[i], i);
		}
		break;
	case 'q': // 종료
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	if (animation) {
		for (int i = 0; i < 4; i++) {
			move_shape(shapes[i], i);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}