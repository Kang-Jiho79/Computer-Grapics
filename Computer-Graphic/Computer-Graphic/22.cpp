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
	float center[3]{};
	float size[3]{};
	float color[3]{};
	GLuint VAO, VBO[2], EBO;
	int x_rotate = 0, y_rotate = 0, revolution = 0;
	float translation[3] = { 0.0f };
	float x_rotationAngle = { 0.0f };
	float y_rotationAngle = { 0.0f };
	float revolutionAngle = { 0.0f };

	// 애니메이션을 위한 추가 변수들
	float animOffset[3] = { 0.0f }; // 애니메이션 오프셋
};

// 카메라 클래스 정의
class Camera {
public:
	glm::vec3 eye;    // 카메라 위치
	glm::vec3 at;     // 카메라가 바라보는 지점
	glm::vec3 up;     // 업 벡터

	// 애니메이션 제어 변수
	int x_rotate = 0, y_rotate = 0, revolution = 0;
	float revolutionAngle = 0.0f;
	float y_rotationAngle = 0.0f;
	float x_rotationAngle = 0.0f;

	// 공전 반지름과 초기 설정
	float revolutionRadius = 3.0f;
	glm::vec3 initialEye;  // 공전 시작 시의 위치 저장
	bool isRevolving = false;  // 공전 상태 추가

	Camera() {
		reset();
	}

	void reset() {
		eye = glm::vec3(0.0f, 2.0f, 3.0f);
		at = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);

		x_rotate = 0;
		y_rotate = 0;
		revolution = 0;
		revolutionAngle = 0.0f;
		y_rotationAngle = 0.0f;
		x_rotationAngle = 0.0f;
		isRevolving = false;

		// 초기 위치에서 공전 반지름 계산
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
		initialEye = eye;
	}

	// View 행렬 생성
	glm::mat4 getViewMatrix() {
		glm::vec3 currentEye = eye;
		glm::vec3 currentAt = at;
		glm::vec3 currentUp = up;

		// 공전 (원점을 중심으로 카메라가 회전)
		if (revolutionAngle != 0.0f) {
			// 공전 시작 시점에서 초기 각도 계산
			if (!isRevolving) {
				isRevolving = true;
				glm::vec3 toEye = eye - at;
				revolutionRadius = glm::length(toEye);
				initialEye = eye;
			}

			// 원점을 중심으로 공전하되, Y 좌표는 초기값 유지
			float rad = glm::radians(revolutionAngle);

			// 초기 위치에서의 각도 계산
			glm::vec3 initialDirection = glm::normalize(initialEye - at);
			float initialAngle = atan2(initialDirection.z, initialDirection.x);

			// 현재 각도 = 초기 각도 + 회전량
			float currentAngle = initialAngle + rad;

			currentEye.x = at.x + cos(currentAngle) * revolutionRadius;
			currentEye.z = at.z + sin(currentAngle) * revolutionRadius;
			currentEye.y = initialEye.y;  // Y 좌표는 초기값 유지
		}
		else {
			isRevolving = false;
		}

		// Y축 자전 (카메라가 제자리에서 좌우 회전)
		if (y_rotationAngle != 0.0f) {
			glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(y_rotationAngle), glm::vec3(0, 1, 0));
			glm::vec4 newAt = rotY * glm::vec4(currentAt - currentEye, 1.0f);
			currentAt = currentEye + glm::vec3(newAt);
		}

		return glm::lookAt(currentEye, currentAt, currentUp);
	}

	// 카메라 이동
	void moveX(float delta) {
		glm::vec3 right = glm::normalize(glm::cross(at - eye, up));
		eye += right * delta;
		at += right * delta;

		// 이동 후에는 공전 상태 리셋
		isRevolving = false;
		initialEye = eye;
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
	}

	void moveZ(float delta) {
		glm::vec3 forward = glm::normalize(at - eye);
		eye += forward * delta;
		at += forward * delta;

		// 이동 후에는 공전 상태 리셋
		isRevolving = false;
		initialEye = eye;
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
	}
};

Shape axis;
Shape ground; // 바닥 추가
Shape under_body;
Shape mid_body;
Shape top_body[2];
Shape gun_barrel[2];
Shape flag[2];
Shape* shapes[8] = { &under_body, &mid_body, &top_body[0], &top_body[1], &gun_barrel[0], &gun_barrel[1], &flag[0], &flag[1] };

GLvoid initBuffer(Shape& shape);

Camera camera;
bool cameraAnimation = false;

// 상부 몸체 위치 교환을 위한 변수
bool topBodySwapped = false;

// 애니메이션 관련 변수들
bool isSwapping = false;
float swapProgress = 0.0f;
const float SWAP_DURATION = 1.0f; // 1초간 애니메이션
float originalPos0X, originalPos1X; // 원래 위치 저장

// 바닥 경계 설정
const float GROUND_SIZE = 5.0f;
const float TANK_BOUNDARY = GROUND_SIZE - 0.7f; // 탱크 크기 고려한 경계

// 선형 보간 함수
float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

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

// 바닥 생성 함수 추가
void createGround(Shape& shape) {
	shape.center[0] = 0.0f;
	shape.center[1] = -0.5f; // 탱크 아래 위치
	shape.center[2] = 0.0f;
	shape.size[0] = GROUND_SIZE;
	shape.size[1] = 0.1f;
	shape.size[2] = GROUND_SIZE;
	shape.color[0] = 0.3f;
	shape.color[1] = 0.7f;
	shape.color[2] = 0.3f; // 초록색 바닥

	createCube(shape);
}

// 상부 몸체 위치 교환 애니메이션 시작 함수
void startSwapAnimation() {
	if (isSwapping) return; // 이미 애니메이션 중이면 무시

	isSwapping = true;
	swapProgress = 0.0f;

	// 원래 위치 저장
	originalPos0X = top_body[0].center[0];
	originalPos1X = top_body[1].center[0];

	std::cout << "상부 몸체 교환 애니메이션 시작!" << std::endl;
}

// 상부 몸체 위치 교환 함수 (즉시 실행)
void swapTopBodies() {
	topBodySwapped = !topBodySwapped;

	// 상부 몸체 위치 정보 교환
	float tempX = top_body[0].center[0];
	top_body[0].center[0] = top_body[1].center[0];
	top_body[1].center[0] = tempX;

	// 포신 위치도 함께 교환 (상부 몸체를 따라 이동)
	gun_barrel[0].center[0] = top_body[0].center[0];
	gun_barrel[1].center[0] = top_body[1].center[0];

	// 깃대 위치도 함께 교환 (상부 몸체를 따라 이동)
	flag[0].center[0] = top_body[0].center[0];
	flag[1].center[0] = top_body[1].center[0];

	// 회전 상태도 교환
	float tempRevolution = gun_barrel[0].revolutionAngle;
	gun_barrel[0].revolutionAngle = gun_barrel[1].revolutionAngle;
	gun_barrel[1].revolutionAngle = tempRevolution;

	int tempRevolutionState = gun_barrel[0].revolution;
	gun_barrel[0].revolution = gun_barrel[1].revolution;
	gun_barrel[1].revolution = tempRevolutionState;

	// 깃대 회전 상태도 교환
	tempRevolution = flag[0].revolutionAngle;
	flag[0].revolutionAngle = flag[1].revolutionAngle;
	flag[1].revolutionAngle = tempRevolution;

	tempRevolutionState = flag[0].revolution;
	flag[0].revolution = flag[1].revolution;
	flag[1].revolution = tempRevolutionState;

	// 버퍼 업데이트
	createCube(top_body[0]);
	createCube(top_body[1]);
	createCube(gun_barrel[0]);
	createCube(gun_barrel[1]);
	createCube(flag[0]);
	createCube(flag[1]);
}

void reset() {
	// 모든 탱크 부품의 translation 초기화
	for (auto& shape : shapes) {
		shape->translation[0] = 0.0f;
		shape->translation[1] = 0.0f;
		shape->translation[2] = 0.0f;
		shape->x_rotationAngle = 0.0f;
		shape->y_rotationAngle = 0.0f;
		shape->revolutionAngle = 0.0f;
		shape->x_rotate = 0;
		shape->y_rotate = 0;
		shape->revolution = 0;
		shape->animOffset[0] = 0.0f;
		shape->animOffset[1] = 0.0f;
		shape->animOffset[2] = 0.0f;
	}

	// 애니메이션 상태 초기화
	isSwapping = false;
	swapProgress = 0.0f;

	// 탱크를 바닥 위에 위치시키기 (겹치지 않도록 약간의 여백 추가)
	float groundTop = ground.center[1] + ground.size[1];

	under_body.center[0] = 0.0f;
	under_body.center[1] = groundTop + 0.21f; // 바닥과 약간의 거리
	under_body.center[2] = 0.0f;
	under_body.size[0] = 0.5f;
	under_body.size[1] = 0.2f;
	under_body.size[2] = 0.3f;
	under_body.color[0] = getRandomcolor();
	under_body.color[1] = getRandomcolor();
	under_body.color[2] = getRandomcolor();

	mid_body.size[0] = 0.3f;
	mid_body.size[1] = 0.2f;
	mid_body.size[2] = 0.2f;
	mid_body.center[0] = 0.0f;
	mid_body.center[1] = under_body.center[1] + under_body.size[1] + mid_body.size[1] + 0.01f; // 약간의 간격
	mid_body.center[2] = 0.0f;
	mid_body.color[0] = getRandomcolor();
	mid_body.color[1] = getRandomcolor();
	mid_body.color[2] = getRandomcolor();

	// 상부 몸체 위치 초기화 (교환 상태 리셋)
	topBodySwapped = false;

	top_body[0].size[0] = 0.15f;
	top_body[0].size[1] = 0.1f;
	top_body[0].size[2] = 0.15f;
	top_body[0].center[0] = -0.2f;
	top_body[0].center[1] = mid_body.center[1] + mid_body.size[1] + top_body[0].size[1] + 0.01f; // 약간의 간격
	top_body[0].center[2] = 0.0f;

	top_body[1].size[0] = 0.15f;
	top_body[1].size[1] = 0.1f;
	top_body[1].size[2] = 0.15f;
	top_body[1].center[0] = 0.2f;
	top_body[1].center[1] = mid_body.center[1] + mid_body.size[1] + top_body[1].size[1] + 0.01f; // 약간의 간격
	top_body[1].center[2] = 0.0f;

	top_body[0].color[0] = getRandomcolor();
	top_body[0].color[1] = getRandomcolor();
	top_body[0].color[2] = getRandomcolor();
	top_body[1].color[0] = top_body[0].color[0];
	top_body[1].color[1] = top_body[0].color[1];
	top_body[1].color[2] = top_body[0].color[2];

	gun_barrel[0].size[0] = 0.05f;
	gun_barrel[0].size[1] = 0.05f;
	gun_barrel[0].size[2] = 0.3f;
	gun_barrel[0].center[0] = top_body[0].center[0];
	gun_barrel[0].center[1] = top_body[0].center[1];
	gun_barrel[0].center[2] = top_body[0].center[2] + top_body[0].size[2] + gun_barrel[0].size[2] + 0.01f; // 약간의 간격

	gun_barrel[1].size[0] = 0.05f;
	gun_barrel[1].size[1] = 0.05f;
	gun_barrel[1].size[2] = 0.3f;
	gun_barrel[1].center[0] = top_body[1].center[0];
	gun_barrel[1].center[1] = top_body[1].center[1];
	gun_barrel[1].center[2] = top_body[1].center[2] + top_body[1].size[2] + gun_barrel[1].size[2] + 0.01f; // 약간의 간격

	gun_barrel[0].color[0] = getRandomcolor();
	gun_barrel[0].color[1] = getRandomcolor();
	gun_barrel[0].color[2] = getRandomcolor();
	gun_barrel[1].color[0] = gun_barrel[0].color[0];
	gun_barrel[1].color[1] = gun_barrel[0].color[1];
	gun_barrel[1].color[2] = gun_barrel[0].color[2];

	flag[0].size[0] = 0.01f;
	flag[0].size[1] = 0.2f;
	flag[0].size[2] = 0.01f;
	flag[0].center[0] = top_body[0].center[0];
	flag[0].center[1] = top_body[0].center[1] + top_body[0].size[1] + flag[0].size[1] + 0.01f; // 약간의 간격
	flag[0].center[2] = top_body[0].center[2];

	flag[1].size[0] = 0.01f;
	flag[1].size[1] = 0.2f;
	flag[1].size[2] = 0.01f;
	flag[1].center[0] = top_body[1].center[0];
	flag[1].center[1] = top_body[1].center[1] + top_body[1].size[1] + flag[1].size[1] + 0.01f; // 약간의 간격
	flag[1].center[2] = top_body[1].center[2];

	flag[0].color[0] = getRandomcolor();
	flag[0].color[1] = getRandomcolor();
	flag[0].color[2] = getRandomcolor();
	flag[1].color[0] = flag[0].color[0];
	flag[1].color[1] = flag[0].color[1];
	flag[1].color[2] = flag[0].color[2];

	createCube(under_body);
	createCube(mid_body);
	createCube(top_body[0]);
	createCube(top_body[1]);
	createCube(gun_barrel[0]);
	createCube(gun_barrel[1]);
	createCube(flag[0]);
	createCube(flag[1]);
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

// 포신이 바깥쪽으로 회전하는지 확인하는 함수
bool isGunBarrelOutwardRotation(int gunIndex) {
	// 현재 상부 몸체의 실제 X 위치 (애니메이션 오프셋 포함)
	float currentTopBodyX = (gunIndex == 0) ?
		(top_body[0].center[0] + top_body[0].animOffset[0]) :
		(top_body[1].center[0] + top_body[1].animOffset[0]);

	// 왼쪽에 있는 포신 (X < 0)은 왼쪽으로만(음의 방향), 
	// 오른쪽에 있는 포신 (X > 0)은 오른쪽으로만(양의 방향) 회전
	if (currentTopBodyX < 0) {
		// 왼쪽 포신: 음의 회전만 허용 (바깥쪽)
		return gun_barrel[gunIndex].revolutionAngle <= 0;
	}
	else {
		// 오른쪽 포신: 양의 회전만 허용 (바깥쪽)
		return gun_barrel[gunIndex].revolutionAngle >= 0;
	}
}

// 포신 회전 방향 결정 함수
void rotateGunBarrelOutward(int gunIndex) {
	// 현재 상부 몸체의 실제 X 위치 (애니메이션 오프셋 포함)
	float currentTopBodyX = (gunIndex == 0) ?
		(top_body[0].center[0] + top_body[0].animOffset[0]) :
		(top_body[1].center[0] + top_body[1].animOffset[0]);

	if (currentTopBodyX < 0) {
		// 왼쪽 포신: 바깥쪽(왼쪽, 음의 방향)으로 회전
		subint(gun_barrel[gunIndex].revolution);
	}
	else {
		// 오른쪽 포신: 바깥쪽(오른쪽, 양의 방향)으로 회전  
		addint(gun_barrel[gunIndex].revolution);
	}
}

void main(int argc, char** argv)
{
	width = 500;
	height = 500;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Tank Animation");
	glewExperimental = GL_TRUE;
	glewInit();

	// 깊이 테스트 활성화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 면 제거 활성화 (겹침 문제 해결)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// 셰이더 프로그램 먼저 생성
	make_shaderProgram();

	createAxis(axis);
	createGround(ground); // 바닥 생성

	std::cout << "=== 조작법 ===" << std::endl;
	std::cout << "←/↑/→/↓: 탱크가 xz평면에서 x축과 z축 방향으로 이동한다." << std::endl;
	std::cout << "t: 중앙몸체가 y축에 대하여 회전한다." << std::endl;
	std::cout << "l: 상부몸체가 이동하여 서로 위치를 바꾼다 (애니메이션)." << std::endl;
	std::cout << "g: 상부몸체 앞의 포신이 바깥쪽으로만 y축 회전한다." << std::endl;
	std::cout << "p: 상부몸체 위의 깃대가 x축에 대하여 회전한다. 양쪽의 깃대는 서로 반대방향으로 회전한다." << std::endl;
	std::cout << "=== 카메라 변환 ===" << std::endl;
	std::cout << "z/Z: 카메라가 z축 양/음방향으로 이동" << std::endl;
	std::cout << "x/X: 카메라가 x축 양/음방향으로 이동" << std::endl;
	std::cout << "y/Y: 카메라기준 y축에 대하여 회전(카메라가 제자리에서 자전)" << std::endl;
	std::cout << "r/R: 화면의 중심의 y축에 대하여 카메라가 회전(중점에 대하여 공전)" << std::endl;
	std::cout << "a: 카메라 공전 애니메이션" << std::endl;
	std::cout << "o: 모든 움직임 멈추기" << std::endl;
	std::cout << "c: 모든 움직임이 초기화된다." << std::endl;
	std::cout << "q: 프로그램 종료하기" << std::endl;

	camera.reset();
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

	// 새로운 카메라 시스템 사용
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	// 기본 회전은 제거하거나 최소화
	glm::mat4 baseRotation = glm::mat4(1.0f);

	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "Matrix");

	// 축 그리기
	glm::mat4 axisMatrix = projection * view * baseRotation;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisMatrix));
	glBindVertexArray(axis.VAO);
	glDrawElements(GL_LINES, axis.index.size(), GL_UNSIGNED_INT, 0);

	// 바닥 그리기
	glm::mat4 groundMatrix = baseRotation;
	glm::mat4 finalGroundMatrix = projection * view * groundMatrix;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(finalGroundMatrix));
	glBindVertexArray(ground.VAO);
	glDrawElements(GL_TRIANGLES, ground.index.size(), GL_UNSIGNED_INT, 0);

	for (auto& shape : shapes) {
		glm::mat4 modelMatrix = baseRotation;

		// 기본 이동 변환 (애니메이션 오프셋 포함)
		modelMatrix = glm::translate(modelMatrix, glm::vec3(
			shape->translation[0] + shape->animOffset[0],
			shape->translation[1] + shape->animOffset[1],
			shape->translation[2] + shape->animOffset[2]
		));

		glm::mat4 finalMatrix = projection * view * modelMatrix;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glBindVertexArray(shape->VAO);
		glDrawElements(GL_TRIANGLES, shape->index.size(), GL_UNSIGNED_INT, 0);
	}

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'z': // 카메라 z축 양방향 이동
		camera.moveZ(0.1f);
		break;
	case 'Z': // 카메라 z축 음방향 이동
		camera.moveZ(-0.1f);
		break;
	case 'x': // 카메라 x축 양방향 이동
		camera.moveX(0.1f);
		break;
	case 'X': // 카메라 x축 음방향 이동
		camera.moveX(-0.1f);
		break;
	case 'y': // 카메라 자전 (Y축 양방향)
		addint(camera.revolution);
		break;
	case 'Y': // 카메라 자전 (Y축 음방향)
		subint(camera.revolution);
		break;
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKeys(int key, int x, int y)
{
	switch (key) {
	}
	glutPostRedisplay();
}

// Timer 함수에 애니메이션 업데이트 추가
GLvoid Timer(int value)
{

	// 카메라 애니메이션
	if (camera.x_rotate != 0) {
		camera.x_rotationAngle += 2.0f * camera.x_rotate;
		if (camera.x_rotationAngle >= 360.0f)
			camera.x_rotationAngle -= 360.0f;
		else if (camera.x_rotationAngle < 0.0f)
			camera.x_rotationAngle += 360.0f;
	}
	if (camera.y_rotate != 0) {
		camera.y_rotationAngle += 2.0f * camera.y_rotate;
		if (camera.y_rotationAngle >= 360.0f)
			camera.y_rotationAngle -= 360.0f;
		else if (camera.y_rotationAngle < 0.0f)
			camera.y_rotationAngle += 360.0f;
	}
	if (camera.revolution != 0) {
		camera.revolutionAngle += 1.0f * camera.revolution; // 카메라 공전은 조금 더 천천히
		if (camera.revolutionAngle >= 360.0f)
			camera.revolutionAngle -= 360.0f;
		else if (camera.revolutionAngle < 0.0f)
			camera.revolutionAngle += 360.0f;
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}