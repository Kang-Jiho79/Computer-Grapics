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
	std::vector<float> normals; // 법선 벡터 추가
	std::vector<int> index;
	std::vector<std::vector<int>> drawface;
	float center[3]{};
	float size[3]{};
	float color[3]{};
	GLuint VAO, VBO[3], EBO; // VBO 배열 크기 3으로 증가
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
Shape cube;
Shape pyramid;
Shape lightCube; // 조명 큐브 추가
Shape lightOrbit; // 조명 궤도 추가
Shape* shapes[2] = { &cube, &pyramid };

bool cube_pyramid = true;	// true: cube, false: pyramid
bool depthTest = true;
bool culling = false;
bool wireframe = false;
bool lighting = true;
bool lightRevolution = false; // 조명 공전 상태
bool showOrbit = true;
float lightAngle = 0.0f;      // 조명 각도
float lightRadius = 1.0f;     // 조명 거리 변수 추가

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
	shape.normals = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
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
		// 앞면 (z2) - 정면을 향하는 면
		x1, y1, z2,   // 0: 왼쪽 아래
		x2, y1, z2,   // 1: 오른쪽 아래
		x2, y2, z2,   // 2: 오른쪽 위
		x1, y2, z2,   // 3: 왼쪽 위

		// 뒷면 (z1) - 뒤쪽을 향하는 면
		x2, y1, z1,   // 4: 오른쪽 아래
		x1, y1, z1,   // 5: 왼쪽 아래
		x1, y2, z1,   // 6: 왼쪽 위
		x2, y2, z1,   // 7: 오른쪽 위

		// 오른쪽면 (x2)
		x2, y1, z2,   // 8: 앞 아래
		x2, y1, z1,   // 9: 뒤 아래
		x2, y2, z1,   // 10: 뒤 위
		x2, y2, z2,   // 11: 앞 위

		// 왼쪽면 (x1)
		x1, y1, z1,   // 12: 뒤 아래
		x1, y1, z2,   // 13: 앞 아래
		x1, y2, z2,   // 14: 앞 위
		x1, y2, z1,   // 15: 뒤 위

		// 윗면 (y2)
		x1, y2, z2,   // 16: 앞 왼쪽
		x2, y2, z2,   // 17: 앞 오른쪽
		x2, y2, z1,   // 18: 뒤 오른쪽
		x1, y2, z1,   // 19: 뒤 왼쪽

		// 아랫면 (y1)
		x1, y1, z1,   // 20: 뒤 왼쪽
		x2, y1, z1,   // 21: 뒤 오른쪽
		x2, y1, z2,   // 22: 앞 오른쪽
		x1, y1, z2    // 23: 앞 왼쪽
	};

	// 법선 벡터 추가
	s.normals = {
		// 앞면 법선 (z 방향)
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		// 뒷면 법선 (-z 방향)
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		// 오른쪽면 법선 (x 방향)
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		// 왼쪽면 법선 (-x 방향)
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		// 윗면 법선 (y 방향)
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		// 아랫면 법선 (-y 방향)
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f
	};

	s.colors = {
		// 앞면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 뒷면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 오른쪽면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 왼쪽면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 윗면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 아랫면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2]
	};

	// 올바른 와인딩 순서로 인덱스 정의 (반시계 방향)
	s.index = {
		// 앞면 (z2)
		0, 1, 2,  2, 3, 0,

		// 뒷면 (z1)
		4, 5, 6,  6, 7, 4,

		// 오른쪽면 (x2)
		8, 9, 10,  10, 11, 8,

		// 왼쪽면 (x1)
		12, 13, 14,  14, 15, 12,

		// 윗면 (y2)
		16, 17, 18,  18, 19, 16,

		// 아랫면 (y1)
		20, 21, 22,  22, 23, 20
	};

	initBuffer(s);
}

void createLightCube(Shape& s)
{
	float x1 = s.center[0] - s.size[0];
	float x2 = s.center[0] + s.size[0];
	float y1 = s.center[1] - s.size[1];
	float y2 = s.center[1] + s.size[1];
	float z1 = s.center[2] - s.size[2];
	float z2 = s.center[2] + s.size[2];

	s.vertices = {
		// 앞면 (z2) - 정면을 향하는 면
		x1, y1, z2,   // 0: 왼쪽 아래
		x2, y1, z2,   // 1: 오른쪽 아래
		x2, y2, z2,   // 2: 오른쪽 위
		x1, y2, z2,   // 3: 왼쪽 위

		// 뒷면 (z1) - 뒤쪽을 향하는 면
		x2, y1, z1,   // 4: 오른쪽 아래
		x1, y1, z1,   // 5: 왼쪽 아래
		x1, y2, z1,   // 6: 왼쪽 위
		x2, y2, z1,   // 7: 오른쪽 위

		// 오른쪽면 (x2)
		x2, y1, z2,   // 8: 앞 아래
		x2, y1, z1,   // 9: 뒤 아래
		x2, y2, z1,   // 10: 뒤 위
		x2, y2, z2,   // 11: 앞 위

		// 왼쪽면 (x1)
		x1, y1, z1,   // 12: 뒤 아래
		x1, y1, z2,   // 13: 앞 아래
		x1, y2, z2,   // 14: 앞 위
		x1, y2, z1,   // 15: 뒤 위

		// 윗면 (y2)
		x1, y2, z2,   // 16: 앞 왼쪽
		x2, y2, z2,   // 17: 앞 오른쪽
		x2, y2, z1,   // 18: 뒤 오른쪽
		x1, y2, z1,   // 19: 뒤 왼쪽

		// 아랫면 (y1)
		x1, y1, z1,   // 20: 뒤 왼쪽
		x2, y1, z1,   // 21: 뒤 오른쪽
		x2, y1, z2,   // 22: 앞 오른쪽
		x1, y1, z2    // 23: 앞 왼쪽
	};

	// 조명 큐브는 밝은 색상 (흰색)
	s.colors = {
		// 모든 면을 밝은 흰색으로
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,
		1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f,  1.0f, 1.0f, 0.8f
	};

	// 법선 벡터는 일반 큐브와 동일
	s.normals = {
		// 앞면 법선 (z 방향)
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		// 뒷면 법선 (-z 방향)
		0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
		// 오른쪽면 법선 (x 방향)
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		// 왼쪽면 법선 (-x 방향)
		-1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		// 윗면 법선 (y 방향)
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		// 아랫면 법선 (-y 방향)
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f
	};

	// 인덱스는 일반 큐브와 동일
	s.index = {
		// 앞면 (z2)
		0, 1, 2,  2, 3, 0,

		// 뒷면 (z1)
		4, 5, 6,  6, 7, 4,

		// 오른쪽면 (x2)
		8, 9, 10,  10, 11, 8,

		// 왼쪽면 (x1)
		12, 13, 14,  14, 15, 12,

		// 윗면 (y2)
		16, 17, 18,  18, 19, 16,

		// 아랫면 (y1)
		20, 21, 22,  22, 23, 20
	};

	initBuffer(s);
}

void createPyramid(Shape& s)
{
	float x1 = s.center[0] - s.size[0];
	float x2 = s.center[0] + s.size[0];
	float y1 = s.center[1] - s.size[1];
	float y2 = s.center[1] + s.size[1];
	float z1 = s.center[2] - s.size[2];
	float z2 = s.center[2] + s.size[2];

	// 피라미드 꼭지점 (중앙 위쪽)
	float apex_x = s.center[0];
	float apex_y = y2;
	float apex_z = s.center[2];

	s.vertices = {
		// 바닥면 (사각형)
		x1, y1, z1,   // 0: 왼쪽 뒤
		x2, y1, z1,   // 1: 오른쪽 뒤
		x2, y1, z2,   // 2: 오른쪽 앞
		x1, y1, z2,   // 3: 왼쪽 앞

		// 앞면 삼각형
		apex_x, apex_y, apex_z,   // 4: 꼭지점
		x1, y1, z2,               // 5: 왼쪽 앞
		x2, y1, z2,               // 6: 오른쪽 앞

		// 오른쪽면 삼각형
		apex_x, apex_y, apex_z,   // 7: 꼭지점
		x2, y1, z2,               // 8: 오른쪽 앞
		x2, y1, z1,               // 9: 오른쪽 뒤

		// 뒷면 삼각형
		apex_x, apex_y, apex_z,   // 10: 꼭지점
		x2, y1, z1,               // 11: 오른쪽 뒤
		x1, y1, z1,               // 12: 왼쪽 뒤

		// 왼쪽면 삼각형
		apex_x, apex_y, apex_z,   // 13: 꼭지점
		x1, y1, z1,               // 14: 왼쪽 뒤
		x1, y1, z2                // 15: 왼쪽 앞
	};

	// 법선 벡터 추가
	s.normals = {
		// 바닥면 법선 (-y 방향)
		0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		// 앞면 삼각형 법선 (대략적인 값)
		0.0f, 0.7f, 0.7f,  0.0f, 0.7f, 0.7f,  0.0f, 0.7f, 0.7f,
		// 오른쪽면 삼각형 법선
		0.7f, 0.7f, 0.0f,  0.7f, 0.7f, 0.0f,  0.7f, 0.7f, 0.0f,
		// 뒷면 삼각형 법선
		0.0f, 0.7f, -0.7f, 0.0f, 0.7f, -0.7f, 0.0f, 0.7f, -0.7f,
		// 왼쪽면 삼각형 법선
		-0.7f, 0.7f, 0.0f, -0.7f, 0.7f, 0.0f, -0.7f, 0.7f, 0.0f
	};

	s.colors = {
		// 바닥면
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 앞면 삼각형
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 오른쪽면 삼각형
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 뒷면 삼각형
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],
		// 왼쪽면 삼각형
		s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2],  s.color[0], s.color[1], s.color[2]
	};

	s.index = {
		// 바닥면 (반시계 방향)
		0, 1, 2,  2, 3, 0,

		// 앞면 삼각형
		4, 5, 6,

		// 오른쪽면 삼각형
		7, 8, 9,

		// 뒷면 삼각형
		10, 11, 12,

		// 왼쪽면 삼각형
		13, 14, 15
	};

	initBuffer(s);
}

void createOrbit(Shape& s, float radius, int segments) {
	s.vertices.clear();
	s.index.clear();
	s.colors.clear();
	s.normals.clear(); // 법선도 초기화

	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		s.vertices.push_back(x);
		s.vertices.push_back(0.0f); // y 위치를 1.0f로 (조명 높이와 맞춤)
		s.vertices.push_back(z);

		s.colors.push_back(0.7f);
		s.colors.push_back(0.7f);
		s.colors.push_back(0.7f);

		// 법선 벡터 (궤도는 선이므로 0으로 설정)
		s.normals.push_back(0.0f);
		s.normals.push_back(0.0f);
		s.normals.push_back(0.0f);

		s.index.push_back(i);
	}

	// 마지막 점을 첫 번째 점과 연결하여 원을 닫기
	if (segments > 0) {
		s.index.push_back(0);
	}

	initBuffer(s); // 버퍼 초기화 호출
}

void updateLightOrbit() {
	// 현재 반지름으로 궤도 다시 생성
	createOrbit(lightOrbit, lightRadius, 50);
	std::cout << "조명 궤도 업데이트 - 반지름: " << lightRadius << std::endl;
}

void reset() {
	cube.center[0] = 0.0f;
	cube.center[1] = 0.0f;
	cube.center[2] = 0.0f;
	cube.size[0] = 0.5f;
	cube.size[1] = 0.5f;
	cube.size[2] = 0.5f;
	cube.color[0] = getRandomcolor();
	cube.color[1] = getRandomcolor();
	cube.color[2] = getRandomcolor();
	createCube(cube);

	pyramid.center[0] = 0.0f;
	pyramid.center[1] = 0.0f;
	pyramid.center[2] = 0.0f;
	pyramid.size[0] = 0.5f;
	pyramid.size[1] = 0.5f;
	pyramid.size[2] = 0.5f;
	pyramid.color[0] = getRandomcolor();
	pyramid.color[1] = getRandomcolor();
	pyramid.color[2] = getRandomcolor();
	createPyramid(pyramid);

	// 조명 큐브 초기화
	lightCube.center[0] = 0.0f;
	lightCube.center[1] = 0.0f;
	lightCube.center[2] = 0.0f;
	lightCube.size[0] = 0.15f;  // 적당한 크기
	lightCube.size[1] = 0.15f;
	lightCube.size[2] = 0.15f;
	lightCube.color[0] = 1.0f; // 밝은 흰색
	lightCube.color[1] = 1.0f;
	lightCube.color[2] = 0.8f;
	createLightCube(lightCube);

	// 조명 거리 초기화
	lightRadius = 1.0f;

	// 조명 궤도 초기화 (동적 반지름 사용)
	createOrbit(lightOrbit, lightRadius, 50);

	// 조명 각도 초기화
	lightAngle = 0.0f;
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


void main(int argc, char** argv)
{
	width = 500;
	height = 500;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("3D Lighting with Dynamic Orbit");
	glewExperimental = GL_TRUE;
	glewInit();

	// 깊이 테스트 활성화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 셰이더 프로그램 먼저 생성
	make_shaderProgram();

	createAxis(axis);

	reset(); // 초기 도형 상태 설정

	// 조작법 출력
	std::cout << "======= 3D 동적 조명 + 궤도 데모 =======" << std::endl;
	std::cout << "n: 도형 전환 (큐브/피라미드)" << std::endl;
	std::cout << "m: 조명 켜기/끄기" << std::endl;
	std::cout << "y: 도형 y축 회전" << std::endl;
	std::cout << "r: 조명 공전" << std::endl;
	std::cout << "z: 조명 가까이 (궤도 축소)" << std::endl;
	std::cout << "Z: 조명 멀리 (궤도 확장)" << std::endl;
	std::cout << "c: 초기화" << std::endl;
	std::cout << "q: 프로그램 종료" << std::endl;
	std::cout << "방향키: 도형 이동" << std::endl;
	std::cout << "====================================" << std::endl;

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
	vertexSource = filetobuf("vertex_light.glsl");
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
	fragmentSource = filetobuf("fragment_light.glsl");
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
	glGenBuffers(3, shape.VBO); // 3개 버퍼 생성
	glGenBuffers(1, &shape.EBO);

	// 정점 버퍼
	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// 색상 버퍼
	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// 법선 버퍼
	glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, shape.normals.size() * sizeof(float), shape.normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// 인덱스 버퍼
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

	// 조명 위치 계산 (동적 반지름 사용)
	glm::vec3 lightPos = glm::vec3(
		lightRadius * cos(glm::radians(lightAngle)),
		0.0f,
		lightRadius * sin(glm::radians(lightAngle))
	);

	// 조명 설정
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 3.0f);

	// 유니폼 변수 위치 가져오기
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "Matrix");
	unsigned int modelMatLocation = glGetUniformLocation(shaderProgramID, "model");
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "view");
	unsigned int projLocation = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
	unsigned int lightingLocation = glGetUniformLocation(shaderProgramID, "enableLighting");

	// 공통 유니폼 설정
	glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
	glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));
	glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projection));

	// 축 그리기 (조명 끄기)
	glUniform1i(lightingLocation, false);
	glm::mat4 axisMatrix = projection * view * baseRotation;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisMatrix));
	glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(baseRotation));
	glBindVertexArray(axis.VAO);
	glDrawElements(GL_LINES, axis.index.size(), GL_UNSIGNED_INT, 0);

	// 조명 궤도 그리기 (조명 끄기)
	if (showOrbit) {
		glUniform1i(lightingLocation, false);
		glm::mat4 orbitMatrix = projection * view * baseRotation;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitMatrix));
		glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(baseRotation));
		glBindVertexArray(lightOrbit.VAO);
		glDrawElements(GL_LINE_STRIP, lightOrbit.index.size(), GL_UNSIGNED_INT, 0);
	}

	// 조명 큐브 그리기 (더 크고 밝게)
	glUniform1i(lightingLocation, false);  // 조명 큐브는 조명 효과 받지 않음

	glm::mat4 lightMatrix = baseRotation;
	lightMatrix = glm::translate(lightMatrix, lightPos);  // 조명 위치로 이동
	lightMatrix = glm::scale(lightMatrix, glm::vec3(0.1f, 0.1f, 0.1f)); // 크기 조정

	glm::mat4 lightFinalMatrix = projection * view * lightMatrix;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(lightFinalMatrix));
	glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(lightMatrix));

	glBindVertexArray(lightCube.VAO);
	glDrawElements(GL_TRIANGLES, lightCube.index.size(), GL_UNSIGNED_INT, 0);

	// 메인 객체들 그리기 (조명 설정)
	glUniform1i(lightingLocation, lighting);

	for (auto& shape : shapes) {
		if (shape == &cube && !cube_pyramid)
			continue;
		if (shape == &pyramid && cube_pyramid)
			continue;

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

		// 유니폼 설정
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glUniformMatrix4fv(modelMatLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glBindVertexArray(shape->VAO);
		glDrawElements(GL_TRIANGLES, shape->index.size(), GL_UNSIGNED_INT, 0);
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
	case 'n': // 도형 전환
		cube_pyramid = !cube_pyramid;
		std::cout << "도형 전환: " << (cube_pyramid ? "큐브" : "피라미드") << std::endl;
		break;
	case 'm': //  조명 켜기/끄기
		lighting = !lighting;
		std::cout << "조명: " << (lighting ? "켜짐" : "꺼짐") << std::endl;
		break;
	case 'y': // y축 회전
		for (auto& shape : shapes) {
			addint(shape->y_rotate);
		}
		std::cout << "Y축 회전 토글" << std::endl;
		break;
	case 'r': // 조명 공전
		lightRevolution = !lightRevolution;
		std::cout << "조명 공전: " << (lightRevolution ? "시작" : "정지") << std::endl;
		break;
	case 'z': // 조명 가까이
		lightRadius -= 0.2f;
		if (lightRadius < 0.2f) lightRadius = 0.2f; // 최소 거리 제한
		updateLightOrbit();
		std::cout << "조명 거리: " << lightRadius << std::endl;
		break;
	case 'Z': // 조명 멀리
		lightRadius += 0.2f;
		if (lightRadius > 5.0f) lightRadius = 5.0f; // 최대 거리 제한
		updateLightOrbit();
		std::cout << "조명 거리: " << lightRadius << std::endl;
		break;
	case 'c':
		reset();
		std::cout << "초기화 완료" << std::endl;
		break;
	case 'q': // 프로그램 종료
		std::cout << "프로그램 종료" << std::endl;
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
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
	// 조명 공전
	if (lightRevolution) {
		lightAngle += 2.0f;
		if (lightAngle >= 360.0f)
			lightAngle -= 360.0f;
	}

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
				shape->revolutionAngle -= 360.0f;
			else if (shape->revolutionAngle < 0.0f)
				shape->revolutionAngle += 360.0f;
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