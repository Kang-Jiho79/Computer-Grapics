#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <random>
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
GLvoid Timer(int value);  // 타이머 콜백 함수 추가
GLvoid Motion(int x, int y);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
GLvoid Mouse(int button, int state, int x, int y);
const GLfloat triShape[3][3]{};
const GLfloat colors[3][3]{};

class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;  // 정점별 색상 지원
	float cx = 0.0f, cy = 0.0f;
	float dx = 1.0f, dy = 1.0f;
	int type;
	GLuint VAO, VBO[2];
	bool exists = false;
	bool isMerged = false;
	float size = 0.08f;
};

Shape shapes[20];
float x_ndc = 0.0f, y_ndc = 0.0f;
bool animation = true;
int existingShapes = 0;
int selectedShape = -1;
bool isDragging = false;

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

void make_point(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 0;
	// 점을 더 크게 만들기 위해 작은 삼각형들로 구성
	float size = 0.015f;
	shape.vertices = {
		// 첫 번째 삼각형 (위)
		shape.cx, shape.cy + size, 0.0f,
		shape.cx - size, shape.cy, 0.0f,
		shape.cx + size, shape.cy, 0.0f,
		// 두 번째 삼각형 (아래)
		shape.cx, shape.cy - size, 0.0f,
		shape.cx - size, shape.cy, 0.0f,
		shape.cx + size, shape.cy, 0.0f,
		// 세 번째 삼각형 (추가)
		shape.cx - size, shape.cy + size * 0.5f, 0.0f,
		shape.cx - size, shape.cy - size * 0.5f, 0.0f,
		shape.cx + size, shape.cy, 0.0f
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	shape.exists = true;
	initBuffer(shape);
}

void make_line(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 1;
	// 선을 더 굵게 만들기 위해 직사각형으로 구성
	float length = 0.08f;
	float thickness = 0.008f;
	shape.vertices = {
		// 첫 번째 삼각형
		shape.cx - length, shape.cy - thickness, 0.0f,
		shape.cx - length, shape.cy + thickness, 0.0f,
		shape.cx + length, shape.cy + thickness, 0.0f,
		// 두 번째 삼각형
		shape.cx - length, shape.cy - thickness, 0.0f,
		shape.cx + length, shape.cy + thickness, 0.0f,
		shape.cx + length, shape.cy - thickness, 0.0f,
		// 세 번째 삼각형 (추가 두께)
		shape.cx - length * 0.8f, shape.cy - thickness * 1.5f, 0.0f,
		shape.cx - length * 0.8f, shape.cy + thickness * 1.5f, 0.0f,
		shape.cx + length * 0.8f, shape.cy, 0.0f
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	shape.exists = true;
	initBuffer(shape);
}

void make_triangle(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 2;
	float size = 0.08f;
	shape.vertices = {
		shape.cx, shape.cy + size, 0.0f,
		shape.cx - size, shape.cy - size * 0.5f, 0.0f,
		shape.cx + size, shape.cy - size * 0.5f, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx + size, shape.cy - size * 0.5f, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx, shape.cy + size, 0.0f
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	shape.exists = true;
	initBuffer(shape);
}

void make_square(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 3;
	float size = 0.08f;
	shape.vertices = {
		shape.cx - size, shape.cy + size, 0.0f,
		shape.cx - size, shape.cy - size, 0.0f,
		shape.cx + size, shape.cy - size, 0.0f,
		shape.cx - size, shape.cy + size, 0.0f,
		shape.cx + size, shape.cy - size, 0.0f,
		shape.cx + size, shape.cy + size, 0.0f,
		shape.cx - size, shape.cy + size, 0.0f,
		shape.cx + size, shape.cy + size, 0.0f,
		shape.cx - size, shape.cy + size, 0.0f,
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	shape.exists = true;
	initBuffer(shape);
}

void make_pentagon(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 4;
	float size = 0.08f;
	shape.vertices = {
		shape.cx, shape.cy + size, 0.0f,
		shape.cx - size * 0.95f, shape.cy + size * 0.31f, 0.0f,
		shape.cx - size * 0.59f, shape.cy - size * 0.81f, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx - size * 0.59f, shape.cy - size * 0.81f, 0.0f,
		shape.cx + size * 0.59f, shape.cy - size * 0.81f, 0.0f,
		shape.cx, shape.cy + size, 0.0f,
		shape.cx + size * 0.59f, shape.cy - size * 0.81f, 0.0f,
		shape.cx + size * 0.95f, shape.cy + size * 0.31f, 0.0f,
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
	};
	shape.exists = true;
	initBuffer(shape);
}

void updateShapeVertices(Shape& shape)
{
	switch (shape.type) {
	case 0:
	{
		float size = 0.015f;
		shape.vertices = {
			shape.cx, shape.cy + size, 0.0f,
			shape.cx - size, shape.cy, 0.0f,
			shape.cx + size, shape.cy, 0.0f,
			shape.cx, shape.cy - size, 0.0f,
			shape.cx - size, shape.cy, 0.0f,
			shape.cx + size, shape.cy, 0.0f,
			shape.cx - size, shape.cy + size * 0.5f, 0.0f,
			shape.cx - size, shape.cy - size * 0.5f, 0.0f,
			shape.cx + size, shape.cy, 0.0f
		};
		break;
	}
	case 1:
	{
		float length = 0.08f;
		float thickness = 0.008f;
		shape.vertices = {
			shape.cx - length, shape.cy - thickness, 0.0f,
			shape.cx - length, shape.cy + thickness, 0.0f,
			shape.cx + length, shape.cy + thickness, 0.0f,
			shape.cx - length, shape.cy - thickness, 0.0f,
			shape.cx + length, shape.cy + thickness, 0.0f,
			shape.cx + length, shape.cy - thickness, 0.0f,
			shape.cx - length * 0.8f, shape.cy - thickness * 1.5f, 0.0f,
			shape.cx - length * 0.8f, shape.cy + thickness * 1.5f, 0.0f,
			shape.cx + length * 0.8f, shape.cy, 0.0f
		};
		break;
	}
	case 2:
	{
		float size = 0.08f;
		shape.vertices = {
			shape.cx, shape.cy + size, 0.0f,
			shape.cx - size, shape.cy - size * 0.5f, 0.0f,
			shape.cx + size, shape.cy - size * 0.5f, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx + size, shape.cy - size * 0.5f, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx, shape.cy + size, 0.0f
		};
		break;
	}
	case 3:
	{
		float size = 0.08f;
		shape.vertices = {
			shape.cx - size, shape.cy + size, 0.0f,
			shape.cx - size, shape.cy - size, 0.0f,
			shape.cx + size, shape.cy - size, 0.0f,
			shape.cx - size, shape.cy + size, 0.0f,
			shape.cx + size, shape.cy - size, 0.0f,
			shape.cx + size, shape.cy + size, 0.0f,
			shape.cx - size, shape.cy + size, 0.0f,
			shape.cx + size, shape.cy + size, 0.0f,
			shape.cx - size, shape.cy + size, 0.0f,
		};
		break;
	}
	case 4:
	{
		float size = 0.08f;
		shape.vertices = {
			shape.cx, shape.cy + size, 0.0f,
			shape.cx - size * 0.95f, shape.cy + size * 0.31f, 0.0f,
			shape.cx - size * 0.59f, shape.cy - size * 0.81f, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx - size * 0.59f, shape.cy - size * 0.81f, 0.0f,
			shape.cx + size * 0.59f, shape.cy - size * 0.81f, 0.0f,
			shape.cx, shape.cy + size, 0.0f,
			shape.cx + size * 0.59f, shape.cy - size * 0.81f, 0.0f,
			shape.cx + size * 0.95f, shape.cy + size * 0.31f, 0.0f,
		};
		break;
	}
	}
	initBuffer(shape);
}

void move_shape(Shape& shape)
{
	shape.cx += shape.dx;
	shape.cy += shape.dy;

	// 벽에 닿으면 튕기기
	if (shape.cx - shape.size < -1.0f || shape.cx + shape.size > 1.0f)
		shape.dx *= -1;
	if (shape.cy - shape.size < -1.0f || shape.cy + shape.size > 1.0f)
		shape.dy *= -1;

	updateShapeVertices(shape);
}

bool checkCollision(const Shape& shape1, const Shape& shape2)
{
	float distance = sqrt((shape1.cx - shape2.cx) * (shape1.cx - shape2.cx) +
		(shape1.cy - shape2.cy) * (shape1.cy - shape2.cy));
	return distance < 0.08f; // 충돌 거리 임계값
}

void mergeShapes(int shape1Index, int shape2Index)
{
	Shape& shape1 = shapes[shape1Index];
	Shape& shape2 = shapes[shape2Index];

	// type은 0부터 시작하므로 계산할 때 1을 더함
	int value1 = shape1.type + 1; // 점=1, 선=2, 삼각형=3, 사각형=4, 오각형=5
	int value2 = shape2.type + 1;
	int sum = value1 + value2;

	// 5보다 크면 다시 점부터 시작 (6이면 1, 7이면 2, ...)
	if (sum > 5) {
		sum = sum - 5;
	}

	// 새로운 위치는 두 도형의 중점
	shape1.cx = (shape1.cx + shape2.cx) / 2.0f;
	shape1.cy = (shape1.cy + shape2.cy) / 2.0f;

	// 새로운 타입 설정 (sum - 1을 해서 0부터 시작하도록)
	shape1.type = sum - 1;

	// 합쳐진 도형은 애니메이션 설정
	shape1.isMerged = true;
	shape1.dx = getRandomfloat(-0.02f, 0.02f); // 랜덤한 x 방향 속도
	shape1.dy = getRandomfloat(-0.02f, 0.02f); // 랜덤한 y 방향 속도

	// 속도가 너무 작으면 조정
	if (abs(shape1.dx) < 0.01f) shape1.dx = shape1.dx < 0 ? -0.01f : 0.01f;
	if (abs(shape1.dy) < 0.01f) shape1.dy = shape1.dy < 0 ? -0.01f : 0.01f;

	// 새로운 도형 생성
	switch (shape1.type) {
	case 0:
		make_point(shape1);
		break;
	case 1:
		make_line(shape1);
		break;
	case 2:
		make_triangle(shape1);
		break;
	case 3:
		make_square(shape1);
		break;
	case 4:
		make_pentagon(shape1);
		break;
	}

	// 합쳐진 도형으로 설정 (도형 생성 후에 다시 설정)
	shape1.isMerged = true;

	// 두 번째 도형 제거
	shape2.exists = false;

	printf("도형 합치기: %d + %d = %d (타입: %d)\n", value1, value2, sum, shape1.type);
}

void initshapes()
{
	for (int i = 0; i < 20; i++) {
		float x = getRandomfloat(-0.9f, 0.9f);
		float y = getRandomfloat(-0.9f, 0.9f);
		shapes[i].cx = x;
		shapes[i].cy = y;
		shapes[i].isMerged = false;
		if (i % 5 == 0) {
			make_point(shapes[i]);
		}
		else if (i % 5 == 1) {
			make_line(shapes[i]);
		}
		else if (i % 5 == 2) {
			make_triangle(shapes[i]);
		}
		else if (i % 5 == 3) {
			make_square(shapes[i]);
		}
		else if (i % 5 == 4) {
			make_pentagon(shapes[i]);
		}
		existingShapes++;
	}
}

bool isPointInDot(const Shape& shape, float mouseX, float mouseY) {
	float dotX = shape.cx;
	float dotY = shape.cy;
	float distance = sqrt((mouseX - dotX) * (mouseX - dotX) + (mouseY - dotY) * (mouseY - dotY));
	return distance <= 0.04f; // 선택 범위도 증가
}

bool isPointInLine(const Shape& shape, float mouseX, float mouseY) {
	float centerX = shape.cx;
	float centerY = shape.cy;
	float distance = sqrt((mouseX - centerX) * (mouseX - centerX) + (mouseY - centerY) * (mouseY - centerY));
	return distance <= 0.05f; // 선택 범위 증가
}

bool isPointInTriangle(const Shape& shape, float mouseX, float mouseY) {
	float centerX = shape.cx;
	float centerY = shape.cy;
	float distance = sqrt((mouseX - centerX) * (mouseX - centerX) + (mouseY - centerY) * (mouseY - centerY));
	return distance <= 0.06f; // 선택 범위 증가
}

bool isPointInSquare(const Shape& shape, float mouseX, float mouseY) {
	float size = 0.08f;
	return (mouseX >= shape.cx - size && mouseX <= shape.cx + size &&
		mouseY >= shape.cy - size && mouseY <= shape.cy + size);
}

bool isPointInPentagonSimple(const Shape& shape, float mouseX, float mouseY) {
	float centerX = shape.cx;
	float centerY = shape.cy;
	float distance = sqrt((mouseX - centerX) * (mouseX - centerX) +
		(mouseY - centerY) * (mouseY - centerY));
	return distance <= 0.08f; // 선택 범위 증가
}

int selectShape(float mouseX, float mouseY) {
	for (int i = existingShapes - 1; i >= 0; i--) {
		const Shape& shape = shapes[i];
		if (shape.isMerged) continue;

		bool isSelected = false;
		switch (shape.type) {
		case 0:
			isSelected = isPointInDot(shape, mouseX, mouseY);
			break;
		case 1:
			isSelected = isPointInLine(shape, mouseX, mouseY);
			break;
		case 2:
			isSelected = isPointInTriangle(shape, mouseX, mouseY);
			break;
		case 3:
			isSelected = isPointInSquare(shape, mouseX, mouseY);
			break;
		case 4:
			isSelected = isPointInPentagonSimple(shape, mouseX, mouseY);
			break;
		}

		if (isSelected) {
			printf("도형 %d 선택됨! (타입: %d)\n", i, shape.type);
			return i;
		}
	}

	printf("선택된 도형 없음\n");
	return -1;
}

void main(int argc, char** argv)
{
	width = 500;
	height = 500;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");
	glewExperimental = GL_TRUE;
	glewInit();

	initshapes();
	//--- 프래그먼트세이더만들기
	make_shaderProgram();
	//--- 세이더프로그램만들기
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
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

//--- 버텍스세이더객체만들기
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

//--- 프래그먼트세이더객체만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트세이더읽어저장하고컴파일하기
	fragmentSource = filetobuf("fragment.glsl");    // 프래그세이더 읽어오기
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
	for (int i = 0; i < existingShapes; i++) {
		if (shapes[i].exists == false) continue;
		glBindVertexArray(shapes[i].VAO);
		glDrawArrays(GL_TRIANGLES, 0, shapes[i].vertices.size() / 3);
	}
	glutSwapBuffers();

}
//--- 다시그리기콜백함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	x_ndc = (2.0f * x / width - 1.0f);
	y_ndc = -(2.0f * y / height - 1.0f);
	switch (key) {
	case 'c':
		initshapes();
		break;
	case 's':
		animation = !animation;
		break;
	case 'q': // 종료
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
		selectedShape = selectShape(x_ndc, y_ndc);
		if (selectedShape != -1) {
			isDragging = true;
		}
		glutPostRedisplay();
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (isDragging && selectedShape != -1) {
			// 충돌 검사 및 합치기
			for (int i = 0; i < existingShapes; i++) {
				if (i != selectedShape && shapes[i].exists &&
					checkCollision(shapes[selectedShape], shapes[i])) {
					mergeShapes(selectedShape, i);
					break;
				}
			}
		}
		selectedShape = -1;
		isDragging = false;
		glutPostRedisplay();
	}
}

GLvoid Motion(int x, int y)
{
	x_ndc = (2.0f * x / width - 1.0f);
	y_ndc = -(2.0f * y / height - 1.0f);
	if (isDragging && selectedShape != -1) {
		shapes[selectedShape].cx = x_ndc;
		shapes[selectedShape].cy = y_ndc;
		updateShapeVertices(shapes[selectedShape]);
	}
	glutPostRedisplay();
}

// 타이머 콜백 함수
GLvoid Timer(int value)
{
	if (animation) {
		for (int i = 0; i < existingShapes; i++) {
			if (shapes[i].exists && shapes[i].isMerged) {
				move_shape(shapes[i]);
			}
		}
	}


	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}


