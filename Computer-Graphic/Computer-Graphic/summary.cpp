#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKeys(int key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
GLvoid Motion(int x, int y);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;
	std::vector<int> index;
	std::vector<std::vector<int>> drawface;
	float center[3]{};
	float size[3]{};
	float color[3]{};
	GLuint VAO, VBO[2], EBO;
	GLUquadricObj* obj = nullptr;
	int x_rotate = 0, y_rotate = 0, revolution = 0;
	int origin_scale = 0, self_scale = 0;
	float translation[3] = { 0.0f };
	float x_rotationAngle = { 0.0f };
	float y_rotationAngle = { 0.0f };
	float revolutionAngle = { 0.0f };
	float origin_scale_value[3]{ 1.0f,1.0f,1.0f };
	float self_scale_value[3]{ 1.0f, 1.0f, 1.0f };
};

Shape axis;
Shape test;
Shape* shapes[1] = { &test };

bool depthTest = true;
bool culling = false;
bool wireframe = false;

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

void createCube(Shape& s)
{
	float x1 = s.center[0] - s.size[0];
	float x2 = s.center[0] + s.size[0];
	float y1 = s.center[1] - s.size[1];
	float y2 = s.center[1] + s.size[1];
	float z1 = s.center[2] - s.size[2];
	float z2 = s.center[2] + s.size[2];
	s.vertices = {
		// 앞면 (면 1) - 빨간색
		x1, y1, z2,   // 0
		x2, y1, z2,   // 1
		x1,  y2, z2,   // 2
		x2,  y2, z2,   // 3

		// 오른쪽면 (면 2) - 초록색
		x2, y1, z2,   // 4
		x2, y1, z1,  // 5
		x2,  y2, z1,  // 6
		x2,  y2, z2,   // 7

		// 뒷면 (면 3) - 파란색
		x2, y1, z1,  // 8
		x1, y1, z1,  // 9
		x1,  y2, z1,  // 10
		x2,  y2, z1,  // 11

		// 왼쪽면 (면 4) - 노란색
		x1, y1, z1,  // 12
		x1, y1, z2,   // 13
		x1,  y2, z2,   // 14
		x1,  y2, z1,  // 15

		// 윗면 (면 5) - 자홍색
		x1,  y2, z2,   // 16
		x2,  y2, z2,   // 17
		x2,  y2, z1,  // 18
		x1,  y2, z1,  // 19

		// 아랫면 (면 6) - 청록색
		x1, y1, z1,  // 20
		x2, y1, z1,  // 21
		x2, y1, z2,   // 22
		x1, y1, z2    // 23
	};
	s.colors = {
		// 앞면 - 빨간색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 오른쪽면 - 초록색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 뒷면 - 파란색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 왼쪽면 - 노란색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 윗면 - 자홍색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 아랫면 - 청록색
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2]
	};

	s.index = {
		0, 1, 2,  2, 3, 0,     // 앞면
		4, 5, 6,  6, 7, 4,     // 오른쪽면
		8, 9, 10, 10, 11, 8,   // 뒷면
		12, 13, 14, 14, 15, 12, // 왼쪽면
		16, 17, 18, 18, 19, 16, // 윗면
		20, 21, 22, 22, 23, 20  // 아랫면
	};
	s.drawface = {
		{0, 1, 2, 2, 3, 0},
		{4, 5, 6, 6, 7, 4},
		{8, 9, 10, 10, 11, 8},
		{12, 13, 14, 14, 15, 12},
		{16, 17, 18, 18, 19, 16},
		{20, 21, 22, 22, 23, 20}
	};
	initBuffer(s);
}	

void createPyramid(Shape& shape)
{
	shape.vertices = {
		// 바닥면
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		// 면 1
		 0.0f, 0.5f,  0.0f,
		 -0.5f, -0.5f, 0.5f,
		 0.5f, -0.5f, 0.5f,
		 // 면 2
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		// 면 3
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		// 면 4
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f
	};
	shape.colors = {
		// 바닥면 - 회색
		0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
		// 면 1 - 빨간색
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		// 면 2 - 초록색
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		// 면 3 - 파란색
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		// 면 4 - 노란색
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f
	};
	shape.index = {
		0, 1, 2,  2, 3, 0,       // 바닥면
		4, 5, 6,                 // 면 1
		7, 8, 9,                 // 면 2
		10, 11, 12,              // 면 3
		13, 14, 15               // 면 4
	};
	initBuffer(shape);
}

void createSphere(Shape& shape, float radius = 0.5f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size[0] = radius;
	shape.size[1] = radius;
	shape.size[2] = radius;
}

void createCylinder(Shape& shape, float baseRadius = 0.3f, float topRadius = 0.3f, float height = 1.0f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size[0] = height;
	shape.size[1] = baseRadius;
	shape.size[2] = topRadius;
}

void createCone(Shape& shape, float baseRadius = 0.5f, float height = 0.5f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size[0] = height;
	shape.size[1] = baseRadius;
}

void reset() {
	test.center[0] = 0.0f;
	test.center[1] = 0.0f;
	test.center[2] = 0.0f;
	test.size[0] = 0.5f;
	test.size[1] = 0.5f;
	test.size[2] = 0.5f;
	test.color[0] = getRandomcolor();
	test.color[1] = getRandomcolor();
	test.color[2] = getRandomcolor();
	createCube(test);
}

void addint(int& value) {
	if (value == 0)
		value = 1;
	else
		value = 0;
}
void subint(int& value) {
	if (value == 0)
		value = -1;
	else
		value = 0;
}

void rotate_Matrix(glm::mat4& matrix, glm::vec3 pre_trans, float angle, glm::vec3 rotate)
{
	matrix = glm::translate(matrix, pre_trans);
	matrix = glm::rotate(matrix, glm::radians(angle), rotate);
	matrix = glm::translate(matrix, -pre_trans);
}

void scale_Matrix(glm::mat4& matrix, glm::vec3 pre_trans, glm::vec3 scale)
{
	matrix = glm::translate(matrix, pre_trans);
	matrix = glm::scale(matrix, scale);
	matrix = glm::translate(matrix, -pre_trans);
}

void drawCubeFace(Shape& s, int face)
{
	GLuint temp;
	glGenBuffers(1, &temp);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, s.drawface[face].size() * sizeof(unsigned int), s.drawface[face].data(), GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, s.drawface[face].size(), GL_UNSIGNED_INT, 0);
	glDeleteBuffers(1, &temp);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s.EBO);
}

void drawCubeFace(int faceIndex, glm::mat4 transform, unsigned int modelLocation) {
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(transform));

	// 각 면별 인덱스 오프셋과 개수
	int indexOffset = faceIndex * 6; // 각 면마다 6개의 인덱스
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(unsigned int)));
}

void drawPyramidFace(int faceIndex, glm::mat4 transform, unsigned int modelLocation) {
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(transform));

	if (faceIndex == 0) {
		// 바닥면 (사각형)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	else {
		// 삼각형 면들
		int indexOffset = 6 + (faceIndex - 1) * 3; // 바닥면 6개 + 삼각형면들
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(unsigned int)));
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

	// 깊이 테스트 활성화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 셰이더 프로그램 먼저 생성
	make_shaderProgram();

	createAxis(axis);

	reset(); // 초기 도형 상태 설정

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys);
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
	vertexSource = filetobuf("vertex_3d.glsl");
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
	fragmentSource = filetobuf("fragment_3d.glsl");
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);
}

GLvoid drawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	if (depthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	if (culling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	glm::mat4 baseRotation = glm::mat4(1.0f);
	baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "Matrix");

	// 축 그리기
	glm::mat4 axisMatrix = projection * view * baseRotation;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisMatrix));
	glBindVertexArray(axis.VAO);
	glDrawElements(GL_LINES, axis.index.size(), GL_UNSIGNED_INT, 0);

	for (auto& shape : shapes) {
		glm::mat4 modelMatrix = baseRotation;

		// 1. 원점 기준 스케일링
		modelMatrix = glm::scale(modelMatrix, glm::vec3(
			shape->origin_scale_value[0],
			shape->origin_scale_value[1],
			shape->origin_scale_value[2]
		));
		// 2. 공전
		modelMatrix = glm::rotate(modelMatrix, glm::radians(shape->revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		// 3. 이동
		modelMatrix = glm::translate(modelMatrix, glm::vec3(
			shape->translation[0],
			shape->translation[1],
			shape->translation[2]
		));
		// 4. 자전
		rotate_Matrix(modelMatrix, glm::vec3(shape->center[0], shape->center[1], shape->center[2]), shape->x_rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate_Matrix(modelMatrix, glm::vec3(shape->center[0], shape->center[1], shape->center[2]), shape->y_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		// 5. 도형 기준 스케일링
		scale_Matrix(modelMatrix, glm::vec3(shape->center[0], shape->center[1], shape->center[2]), glm::vec3(
			shape->self_scale_value[0],
			shape->self_scale_value[1],
			shape->self_scale_value[2]
		));
		glm::mat4 finalMatrix = projection * view * modelMatrix;
		// 쉐이더 사용
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glBindVertexArray(shape->VAO);
		glDrawElements(GL_TRIANGLES, shape->index.size(), GL_UNSIGNED_INT, 0);

		// GLU 객체 그리기
		// OpenGL 고정 기능 파이프라인으로 전환 (GLU 사용을 위해)
		//glUseProgram(0);
		//glMatrixMode(GL_MODELVIEW);
		//glLoadMatrixf(glm::value_ptr(finalMatrix));

		//// GLU 객체 렌더링
		//switch (shape->type) {
		//case 0: // Sphere
		//	gluSphere(shape->obj, shape->size[0], 20, 20);
		//	break;
		//case 1: // Cylinder
		//	gluCylinder(shape->obj, shape->size[0] * 0.6f, shape->size[0] * 0.6f, shape->size[0], 20, 20);
		//	break;
		//case 2: // Cone
		//	gluCylinder(shape->obj, shape->size[0], 0.0f, shape->size[0], 20, 20);
		//	break;
		//case 3:
		//	gluSphere(shape->obj, shape->size[0], 4, 4);
		//}

		//// 셰이더 프로그램 다시 활성화
		//glUseProgram(shaderProgramID);
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
	case 'x': // x축 회전
		for (auto& shape : shapes) {
			if (shape == &test) {
				addint(shape->x_rotate);
			}
		}
		break;
	case 'y': // y축 회전
		for (auto& shape : shapes) {
			if (shape == &test) {
				addint(shape->y_rotate);
			}
		}
		break;
	case 'r': // y축 공전
		for (auto& shape : shapes) {
			if (shape == &test) {
				addint(shape->revolution);
			}
		}
		break;
	case 's': 
		for (auto& shape : shapes) {
			if (shape == &test) {
				addint(shape->self_scale);
			}
		}
		break;
	case 'o':
		for (auto& shape : shapes) {
			if (shape == &test) {
				addint(shape->origin_scale);
			}
		}
		break;
	case 'c':
		reset();
		break;
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
		glutPostRedisplay();
	}
}

GLvoid SpecialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP: // 위쪽 방향키
		for (auto& shape : shapes) {
			shape->translation[2] += 0.1f;
		}
		break;
	case GLUT_KEY_DOWN: // 아래쪽 방향키
		for (auto& shape : shapes) {
			shape->translation[2] -= 0.1f;
		}
		break;
	case GLUT_KEY_LEFT: // 왼쪽 방향키
		for (auto& shape : shapes) {
			shape->translation[0] += 0.1f;
		}
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 방향키
		for (auto& shape : shapes) {
			shape->translation[0] -= 0.1f;
		}
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	for (auto& shape : shapes) {
		if (shape->x_rotate != 0) {
			shape->x_rotationAngle += 2.0f * shape->x_rotate;
			if (shape->x_rotationAngle >= 360.0f)
				shape->x_rotationAngle -= 360.0f;
			else if (shape->x_rotationAngle < 0.0f)
				shape->x_rotationAngle += 360.0f;
		}
		if (shape->y_rotate != 0) {
			shape->y_rotationAngle += 2.0f * shape->y_rotate;
			if (shape->y_rotationAngle >= 360.0f)
				shape->y_rotationAngle -= 360.0f;
			else if (shape->y_rotationAngle < 0.0f)
				shape->y_rotationAngle += 360.0f;
		}
		if (shape->revolution != 0) {
			shape->revolutionAngle += 2.0f * shape->revolution;
			if (shape->revolutionAngle >= 360.0f)
				shape->revolution -= 360.0f;
			else if (shape->revolutionAngle < 0.0f)
				shape->revolution += 360.0f;
		}
		if (shape->origin_scale != 0) {
			float scaleFactor = 0.01f * shape->origin_scale;
			shape->origin_scale_value[0] += scaleFactor;
			shape->origin_scale_value[1] += scaleFactor;
			shape->origin_scale_value[2] += scaleFactor;
		}
		if (shape->self_scale != 0) {
			float scaleFactor = 0.01f * shape->self_scale;
			shape->self_scale_value[0] += scaleFactor;
			shape->self_scale_value[1] += scaleFactor;
			shape->self_scale_value[2] += scaleFactor;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}