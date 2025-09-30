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

#define MaxShapes 10
#define PI 3.14159265359f

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid TimerFunction(int value);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

enum AnimationType { NONE, DIAGONAL, ZIGZAG, RECTSPIRAL, CIRCLESPIRAL };

class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;
	float size;
	float dx = 0.0f;
	float dy = 0.0f;
	float centerX = 0.0f;  // 중심 좌표
	float centerY = 0.0f;
	float rotation = 0.0f; // 현재 회전각 (라디안)
	int type;
	GLuint VAO, VBO[2];

	// 스파이럴 전용 상태 변수 추가
	int spiralState = 0; // 0:왼쪽, 1:아래, 2:오른쪽, 3:위
    float spiralLevel = 0; // 현재 스파이럴 레벨
    bool spiralDirection = true; // true: 안쪽으로, false: 바깥쪽으로

	float angle = 0.0f; // CIRCLESPIRAL용 각도
};

Shape shapes[MaxShapes];
float x_ndc = 0.0f, y_ndc = 0.0f;
int existingShapes = 0;
int animType = NONE;

int rectCount = 0;

bool back = false;

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

void updateTriangleVertices(Shape& shape, float centerX, float centerY, float rotation)
{
	shape.centerX = centerX;
	shape.centerY = centerY;
	shape.rotation = rotation;

	float localVertices[9] = {
		0.0f, shape.size, 0.0f,        // 위쪽 정점
		-shape.size, -shape.size, 0.0f, // 왼쪽 아래
		shape.size, -shape.size, 0.0f   // 오른쪽 아래
	};

	for (int i = 0; i < 3; i++) {
		float x = localVertices[i * 3];
		float y = localVertices[i * 3 + 1];
		
		float rotatedX = x * cosf(rotation) - y * sinf(rotation);
		float rotatedY = x * sinf(rotation) + y * cosf(rotation);
		
		shape.vertices[i * 3] = rotatedX + centerX;
		shape.vertices[i * 3 + 1] = rotatedY + centerY;
		shape.vertices[i * 3 + 2] = 0.0f;
	}
	initBuffer(shape);
}

float calculateRotationAngle(float dx, float dy)
{
	return atan2f(dy, dx) - PI/2;
}

void make_triangle(Shape& shape, float x, float y)
{
	shape.size = getRandomfloat();
	shape.type = 2;
	shape.centerX = x;
	shape.centerY = y;
	shape.rotation = 0.0f;
	shape.vertices.resize(9);
	updateTriangleVertices(shape, x, y, 0.0f);
	
	shape.colors = {
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor()
	};
	initBuffer(shape);
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
	}
	existingShapes = 0;
}

void findCenter(float& cx, float& cy, Shape& shape)
{
	cx = shape.centerX;
	cy = shape.centerY;
}

void makediagonal()
{
	for (int i = 0; i < existingShapes; i++) 
	{
		Shape& shape = shapes[i];
		shape.dx = 0.05f;
		shape.dy = 0.05f;
	}
}

void makezigzag()
{
	for (int i = 0; i < existingShapes; i++) 
	{
		Shape& shape = shapes[i];
		shape.dx = 0.05f;
		shape.dy = 0.0f;
	}
}

void makerectspiral()
{
    for (int i = 0; i < existingShapes; i++) 
    {
        Shape& shape = shapes[i];
		shape.centerY = 1.0f - shape.size; // 시작 위치를 위쪽 중앙으로 설정
		shape.dx = -0.05f;          // 왼쪽으로 이동
		shape.dy = 0.0f;
        shape.spiralLevel = 1.0f;            // 큰 사각형에서 시작
        shape.spiralDirection = false;        // 안쪽으로 시작
    }
}

void makecirclespiral()
{
	for (int i = 0; i < existingShapes; i++) 
	{
		Shape& shape = shapes[i];
		float startAngle = i * (2.0f * PI / existingShapes); 
		shape.centerX = 0.5f * cosf(startAngle);  // 중간 반경에서 시작
		shape.centerY = 0.5f * sinf(startAngle);
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
	glutCreateWindow("Rotating Triangle Animation");
	glewExperimental = GL_TRUE;
	glewInit();
	
	make_shaderProgram();
	
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	
	// 수정: 올바른 루프
	for (int i = 0; i < existingShapes; i++) {
		Shape& shape = shapes[i];
		glBindVertexArray(shape.VAO);
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
	case '1':
		animType = DIAGONAL;
		makediagonal();
		printf("대각선 이동 모드\n");
		break;
	case '2':
		animType = ZIGZAG;
		makezigzag();
		printf("지그재그 이동 모드\n");
		break;
	case '3':
		animType = RECTSPIRAL;
		makerectspiral();
		printf("사각 스파이럴 이동 모드\n");
		break;
	case '4':
		animType = CIRCLESPIRAL;
		makecirclespiral();
		printf("원 스파이럴 이동 모드\n");
		break;
	case 'c':
		clear();
		animType = NONE;
		printf("모든 도형 삭제\n");
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_ndc = (2.0f * x / width - 1.0f);
		y_ndc = -(2.0f * y / height - 1.0f);

		if (existingShapes < MaxShapes) {
			make_triangle(shapes[existingShapes], x_ndc, y_ndc);
			existingShapes++;
		}
		else {
			std::cout << "더이상 도형을 추가할 수 없습니다." << std::endl;
		}

		glutPostRedisplay();
	}
}

GLvoid TimerFunction(int value) {
	static float time = 0.0f;
	time += 0.016f; // 60fps

	if (animType == DIAGONAL) {
		for (int i = 0; i < existingShapes; i++) {
			Shape& shape = shapes[i];
			
			shape.centerX += shape.dx;
			shape.centerY += shape.dy;
			
			if (shape.centerX - shape.size < -1.0f || shape.centerX + shape.size > 1.0f)
				shape.dx *= -1;
			if (shape.centerY - shape.size < -1.0f || shape.centerY + shape.size > 1.0f)
				shape.dy *= -1;
			
			float rotation = calculateRotationAngle(shape.dx, shape.dy);
			
			updateTriangleVertices(shape, shape.centerX, shape.centerY, rotation);
		}
	}
	else if (animType == ZIGZAG) {
		for (int i = 0; i < existingShapes; i++) {
			Shape& shape = shapes[i];
			
			shape.centerX += shape.dx;
			
			if (shape.centerX - shape.size < -1.0f || shape.centerX + shape.size > 1.0f) {
				shape.dx *= -1;
				if (shape.centerY - shape.size < -1.0f || shape.centerY + shape.size > 1.0f) {
					shape.dy *= -1;
				}
				shape.centerY += shape.dy;
			}
			float rotation = calculateRotationAngle(shape.dx, 0);
			
			updateTriangleVertices(shape, shape.centerX, shape.centerY, rotation);
		}
	}
	else if (animType == RECTSPIRAL) {
		for (int i = 0; i < existingShapes; i++) {
			Shape& shape = shapes[i];
			shape.centerX += shape.dx;
			shape.centerY += shape.dy;
			if (!shape.spiralDirection) {
				if (shape.spiralState == 0 && shape.centerX <= -shape.spiralLevel + shape.size) { // 왼쪽
					shape.centerX = -shape.spiralLevel + shape.size;
					shape.spiralState = 1; // 아래로 전환
					shape.dx = 0.0f;
					shape.dy = -0.05f;
					shape.spiralLevel -= 0.1f;
				}
				else if (shape.spiralState == 1 && shape.centerY <= -shape.spiralLevel + shape.size) { // 아래
					shape.centerY = -shape.spiralLevel + shape.size;
					shape.spiralState = 2; // 오른쪽으로 전환
					shape.dx = 0.05f;
					shape.dy = 0.0f;
					shape.spiralLevel -= 0.1f;
				}
				else if (shape.spiralState == 2 && shape.centerX >= shape.spiralLevel - shape.size) { // 오른쪽
					shape.centerX = shape.spiralLevel - shape.size;
					shape.spiralState = 3; // 위로 전환
					shape.dx = 0.0f;
					shape.dy = 0.05f;
					shape.spiralLevel -= 0.1f;
				}
				else if (shape.spiralState == 3 && shape.centerY >= shape.spiralLevel - shape.size) { // 위
					shape.centerY = shape.spiralLevel - shape.size;
					shape.spiralState = 0; // 왼쪽으로 전환
					shape.dx = -0.05f;
					shape.dy = 0.0f;
					shape.spiralLevel -= 0.1f;
				}
				if (shape.spiralLevel < 0.1f) {
					shape.spiralLevel = 0.1f;
					shape.spiralDirection = true; // 안쪽으로 전환
				}
			}
			else {
				if (shape.spiralState == 0 && shape.centerX >= -shape.spiralLevel + shape.size) { // 왼쪽
					shape.centerX = -shape.spiralLevel + shape.size;
					shape.spiralState = 3; // 위로 전환
					shape.dx = 0.0f;
					shape.dy = 0.05f;
					shape.spiralLevel += 0.1f;
				}
				else if (shape.spiralState == 3 && shape.centerY >= shape.spiralLevel - shape.size) { // 위
					shape.centerY = shape.spiralLevel - shape.size;
					shape.spiralState = 2; // 오른쪽으로 전환
					shape.dx = 0.05f;
					shape.dy = 0.0f;
					shape.spiralLevel += 0.1f;
				}
				else if (shape.spiralState == 2 && shape.centerX <= shape.spiralLevel - shape.size) { // 오른쪽
					shape.centerX = shape.spiralLevel - shape.size;
					shape.spiralState = 1; // 아래로 전환
					shape.dx = 0.0f;
					shape.dy = -0.05f;
					shape.spiralLevel += 0.1f;
				}
				else if (shape.spiralState == 1 && shape.centerY <= -shape.spiralLevel + shape.size) { // 아래
					shape.centerY = -shape.spiralLevel + shape.size;
					shape.spiralState = 0; // 왼쪽으로 전환
					shape.dx = -0.05f;
					shape.dy = 0.0f;
					shape.spiralLevel += 0.1f;
				}
				if (shape.spiralLevel > 1.0f) {
					shape.spiralLevel = 1.0f;
					shape.spiralDirection = false; // 바깥쪽으로 전환
				}
			}
			float rotation = calculateRotationAngle(shape.dx, shape.dy);

			updateTriangleVertices(shape, shape.centerX, shape.centerY, rotation);
		}
	}
	else if (animType == CIRCLESPIRAL) {

		for (int i = 0; i < existingShapes; i++) {
			Shape& shape = shapes[i];

			shape.centerX = cos(shape.angle + i) * shape.spiralLevel;
			shape.centerY = sin(shape.angle + i) * shape.spiralLevel;
			if (shape.spiralDirection) {
				shape.spiralLevel -= 0.002f; // 안쪽으로 이동
				shape.angle -= 0.05f;
				if (shape.spiralLevel < 0.1f) {
					shape.spiralLevel = 0.1f;
					shape.spiralDirection = false; // 바깥쪽으로 전환
				}
			}
			else {
				shape.spiralLevel += 0.002f; // 바깥쪽으로 이동
				shape.angle += 0.05f;
				if (shape.spiralLevel > 1.0f) {
					shape.spiralLevel = 1.0f;
					shape.spiralDirection = true; // 안쪽으로 전환
				}
			}

			float rotation = shape.spiralDirection ? shape.angle - PI / 2 : shape.angle + PI / 2; // 접선 방향으로 회전
			updateTriangleVertices(shape, shape.centerX, shape.centerY, rotation);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}
