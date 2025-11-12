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
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);
GLvoid Timer(int value);

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> vel_dis(-0.05f, 0.05f);

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
};

// 카메라 클래스
class Camera {
public:
	glm::vec3 eye;
	glm::vec3 at;
	glm::vec3 up;
	
	int x_rotate = 0, y_rotate = 0, revolution = 0;
	float revolutionAngle = 0.0f;
	float y_rotationAngle = 0.0f;
	float x_rotationAngle = 0.0f;
	float revolutionRadius = 3.0f;
	glm::vec3 initialEye;
	bool isRevolving = false;

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
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
		initialEye = eye;
	}

	glm::mat4 getViewMatrix() {
		glm::vec3 currentEye = eye;
		glm::vec3 currentAt = at;
		glm::vec3 currentUp = up;

		if (revolutionAngle != 0.0f) {
			if (!isRevolving) {
				isRevolving = true;
				glm::vec3 toEye = eye - at;
				revolutionRadius = glm::length(toEye);
				initialEye = eye;
			}
			float rad = glm::radians(revolutionAngle);
			glm::vec3 initialDirection = glm::normalize(initialEye - at);
			float initialAngle = atan2(initialDirection.z, initialDirection.x);
			float currentAngle = initialAngle + rad;
			currentEye.x = at.x + cos(currentAngle) * revolutionRadius;
			currentEye.z = at.z + sin(currentAngle) * revolutionRadius;
			currentEye.y = initialEye.y;
		}
		else {
			isRevolving = false;
		}

		if (y_rotationAngle != 0.0f) {
			glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(y_rotationAngle), glm::vec3(0, 1, 0));
			glm::vec4 newAt = rotY * glm::vec4(currentAt - currentEye, 1.0f);
			currentAt = currentEye + glm::vec3(newAt);
		}

		return glm::lookAt(currentEye, currentAt, currentUp);
	}

	void moveX(float delta) {
		glm::vec3 right = glm::normalize(glm::cross(at - eye, up));
		eye += right * delta;
		at += right * delta;
		isRevolving = false;
		initialEye = eye;
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
	}

	void moveZ(float delta) {
		glm::vec3 forward = glm::normalize(at - eye);
		eye += forward * delta;
		at += forward * delta;
		isRevolving = false;
		initialEye = eye;
		glm::vec3 toEye = eye - at;
		revolutionRadius = glm::length(toEye);
	}
};

// 전역 변수들
Shape box; 
Shape smallBoxes[3];
GLUquadric* spheres[5];
glm::vec3 smallBoxPositions[3];
glm::vec3 spherePositions[5];
glm::vec3 sphereVelocities[5];
float cubeRotateAngle = 0.0f;
float boxSize = 1.0f;
float ballSize = 0.1f;
float openSize = 0.0f;
int ballCount = 0;
int bottomFaceIndex = 2;

Camera camera;
bool isClicked = false;
bool isOpen = false;
bool showSmallBoxes = true;

GLvoid initBuffer(Shape& shape);
void createCube(Shape& s);
void menu();
void updateSmallBoxPositions();
void updateSphereMovement();
int calculateBottomFace();

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

float MouseX(int x) {
	return 2.0f * x / width - 1.0f;
}

float MouseY(int y) {
	return 1.0f - 2.0f * y / height;
}

void createCube(Shape& s)
{
	float x = s.size[0];
	float y = s.size[1];
	float height = s.size[2];
	
	s.vertices = {
		// 앞면
		-x, y, -height,		-x, -y, -height,	x, -y, -height,		x, y, -height,
		// 뒷면
		x, y, height,		x, -y, height,		-x, -y, height,		-x, y, height,
		// 아래면
		-x, -y, -height,	-x, -y, height,		x, -y, height,		x, -y, -height,
		// 윗면 
		-x, y, height,		-x, y, -height,		x, y, -height,		x, y, height,
		// 오른면
		x, y, -height,		x, -y, -height,		x, -y, height,		x, y, height,
		// 왼면 
		-x, y, height,		-x, -y, height,		-x, -y, -height,	-x, y, -height
	};
	
	s.index = {
		// 앞면
		0, 1, 2, 0, 2, 3,
		// 뒷면
		4, 5, 6, 4, 6, 7,
		// 아래면
		8, 9, 10, 8, 10, 11,
		// 위면
		12, 13, 14, 12, 14, 15,
		// 오른면
		16, 17, 18, 16, 18, 19,
		// 왼면
		20, 21, 22, 20, 22, 23
	};
	
	s.colors = {
		// 앞면 - 연한회색
		0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f,
		// 뒷면 - 연한 회색
		0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f,
		// 아랫면 - 진한 회색
		0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f,
		// 윗면 - 진한 회색
		0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f, 0.3f,
		// 오른면 - 회색
		0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
		// 왼면 - 회색
		0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
	};
	
	initBuffer(s);
}

void updateSphereMovement() {
	for (int i = 0; i < ballCount; i++) {
		// 위치 업데이트
		spherePositions[i] += sphereVelocities[i];

		// X축 충돌 체크 및 튕김
		if (spherePositions[i].x - ballSize < -boxSize) {
			spherePositions[i].x = -boxSize + ballSize;
			sphereVelocities[i].x = -sphereVelocities[i].x;
		}
		else if (spherePositions[i].x + ballSize > boxSize) {
			spherePositions[i].x = boxSize - ballSize;
			sphereVelocities[i].x = -sphereVelocities[i].x;
		}

		// Y축 충돌 체크 및 튕김
		if (spherePositions[i].y - ballSize < -boxSize) {
			spherePositions[i].y = -boxSize + ballSize;
			sphereVelocities[i].y = -sphereVelocities[i].y;
		}
		else if (spherePositions[i].y + ballSize > boxSize) {
			spherePositions[i].y = boxSize - ballSize;
			sphereVelocities[i].y = -sphereVelocities[i].y;
		}

		// Z축 충돌 체크 및 튕김
		if (spherePositions[i].z - ballSize < -boxSize) {
			spherePositions[i].z = -boxSize + ballSize;
			sphereVelocities[i].z = -sphereVelocities[i].z;
		}
		else if (spherePositions[i].z + ballSize > boxSize) {
			spherePositions[i].z = boxSize - ballSize;
			sphereVelocities[i].z = -sphereVelocities[i].z;
		}
	}
}

void updateSmallBoxPositions() {
	for (int i = 0; i < 3; i++) {
		float halfSize = 0.1f * (i + 1);

		// 새로운 위치 계산
		float newX = smallBoxPositions[i].x - sin(glm::radians(cubeRotateAngle)) * 0.02f;
		float newY = smallBoxPositions[i].y - cos(glm::radians(cubeRotateAngle)) * 0.02f;

		// X축 충돌 체크
		if (newX - halfSize < -boxSize - openSize) newX = -boxSize - openSize + halfSize;
		else if (newX + halfSize > boxSize + openSize) newX = boxSize + openSize - halfSize;

		// Y축 충돌 체크
		if (newY - halfSize < -boxSize - openSize) newY = -boxSize - openSize + halfSize;
		else if (newY + halfSize > boxSize + openSize) newY = boxSize + openSize - halfSize;

		// 위치 업데이트
		smallBoxPositions[i].x = newX;
		smallBoxPositions[i].y = newY;
	}
}

int calculateBottomFace() {
	glm::vec3 faceNormals[6] = {
		glm::vec3(0.0f, 0.0f, -1.0f), // 앞면
		glm::vec3(0.0f, 0.0f, 1.0f),  // 뒷면
		glm::vec3(0.0f, -1.0f, 0.0f), // 아래면
		glm::vec3(0.0f, 1.0f, 0.0f),  // 위면
		glm::vec3(1.0f, 0.0f, 0.0f),  // 오른면
		glm::vec3(-1.0f, 0.0f, 0.0f)  // 왼면
	};

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cubeRotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	int bottomIndex = 2;
	float minY = 1.0f;

	for (int i = 0; i < 6; i++) {
		glm::vec4 rotatedNormal = rotationMatrix * glm::vec4(faceNormals[i], 0.0f);
		if (rotatedNormal.y < minY) {
			minY = rotatedNormal.y;
			bottomIndex = i;
		}
	}

	return bottomIndex;
}

void menu() {
	std::cout << "=== 조작법 ===" << std::endl;
	std::cout << "z/Z: 카메라 z축 이동" << std::endl;
	std::cout << "x/X: 카메라 x축 이동" << std::endl;
	std::cout << "y/Y: 카메라 y축 회전" << std::endl;
	std::cout << "r/R: 카메라 공전" << std::endl;
	std::cout << "a: 바닥면 열림" << std::endl;
	std::cout << "b: 공추가 (최대 5개)" << std::endl;
	std::cout << "c: 초기화" << std::endl;
	std::cout << "q: 종료" << std::endl;
	std::cout << "마우스 드래그: 큐브 회전 및 작은 큐브 이동" << std::endl;
}

void main(int argc, char** argv)
{
	width = 800;
	height = 600;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Cube Animation with Spheres");
	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	make_shaderProgram();
	
	// 큰 상자 초기화
	box.size[0] = boxSize;
	box.size[1] = boxSize;
	box.size[2] = boxSize;
	createCube(box);

	// 작은 상자들 초기화
	for (int i = 0; i < 3; i++) {
		smallBoxes[i].size[0] = 0.1f * (i + 1);
		smallBoxes[i].size[1] = 0.1f * (i + 1);
		smallBoxes[i].size[2] = 0.1f * (i + 1);
		createCube(smallBoxes[i]);
		
		smallBoxPositions[i].x = 0.0f;
		smallBoxPositions[i].y = -1.0f + (0.1f * (i + 1));
		smallBoxPositions[i].z = -0.1f * ((i + 1) * (i + 1) - 1) + 0.3f;
	}

	menu();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutTimerFunc(50, Timer, 1);
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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// 뷰와 프로젝션 행렬 계산
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	// 유니폼 위치 가져오기
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "Matrix");

	// 메인 박스 그리기
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_FRONT);
	
	// Model-View-Projection 행렬 결합
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(cubeRotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 mvp = projection * view * model;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(mvp));
	
	glBindVertexArray(box.VAO);
	
	if (isOpen) {
		for (int i = 0; i < 6; i++) {
			if (i == bottomFaceIndex) continue;
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
		}
	}
	else {
		glDrawElements(GL_TRIANGLES, box.index.size(), GL_UNSIGNED_INT, 0);
	}

	// 작은 박스들 그리기
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	
	if (showSmallBoxes) {
		for (int i = 0; i < 3; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(cubeRotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, smallBoxPositions[i]);
			glm::mat4 mvp = projection * view * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(mvp));
			glBindVertexArray(smallBoxes[i].VAO);
			glDrawElements(GL_TRIANGLES, smallBoxes[i].index.size(), GL_UNSIGNED_INT, 0);
		}
	}

	// 구체들 그리기
	for (int i = 0; i < ballCount; i++) {
		spheres[i] = gluNewQuadric();
		gluQuadricDrawStyle(spheres[i], GLU_FILL);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(cubeRotateAngle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, spherePositions[i]);
		glm::mat4 mvp = projection * view * model;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(mvp));
		gluSphere(spheres[i], ballSize, 20, 20);
		gluDeleteQuadric(spheres[i]);
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
	case 'z': // 카메라 z축 이동 (앞으로)
		camera.moveZ(0.1f);
		break;
	case 'Z': // 카메라 z축 이동 (뒤로)
		camera.moveZ(-0.1f);
		break;
	case 'x': // 카메라 x축 이동 (오른쪽)
		camera.moveX(0.1f);
		break;
	case 'X': // 카메라 x축 이동 (왼쪽)
		camera.moveX(-0.1f);
		break;
	case 'y': // 카메라 y축 회전
		addint(camera.y_rotate);
		break;
	case 'Y': // 카메라 y축 반대 회전
		subint(camera.y_rotate);
		break;
	case 'r': // 카메라 공전
		addint(camera.revolution);
		break;
	case 'R': // 카메라 반대 공전
		subint(camera.revolution);
		break;
	case 'b': // 공 생성
		if (ballCount >= 5) return;
		spherePositions[ballCount] = glm::vec3(0.0f);
		
		do {
			sphereVelocities[ballCount].x = vel_dis(gen);
			sphereVelocities[ballCount].y = vel_dis(gen);
			sphereVelocities[ballCount].z = vel_dis(gen);
		} while (abs(sphereVelocities[ballCount].x) < 0.01f &&
			abs(sphereVelocities[ballCount].y) < 0.01f &&
			abs(sphereVelocities[ballCount].z) < 0.01f);
		
		ballCount++;
		std::cout << "공 생성! 총 " << ballCount << "개" << std::endl;
		break;
	case 'a': // 바닥 열기
		openSize = 10.0f;
		isOpen = true;
		bottomFaceIndex = calculateBottomFace();
		std::cout << "바닥 열림!" << std::endl;
		break;
	case 'c': // 초기화
		ballCount = 0;
		cubeRotateAngle = 0.0f;
		isOpen = false;
		showSmallBoxes = true;
		openSize = 0.0f;
		camera.reset();
		for (int i = 0; i < 3; i++) {
			smallBoxPositions[i].x = 0.0f;
			smallBoxPositions[i].y = -1.0f + (0.1f * (i + 1));
			smallBoxPositions[i].z = -0.1f * ((i + 1) * (i + 1) - 1) + 0.3f;
		}
		std::cout << "초기화 완료!" << std::endl;
		break;
	case 'q': // 종료
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isClicked = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isClicked = false;
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y)
{
	if (isClicked) {
		if (MouseX(x) < 0.0f) cubeRotateAngle += 0.2f;
		else cubeRotateAngle -= 0.2f;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	// 카메라 애니메이션
	if (camera.y_rotate != 0) {
		camera.y_rotationAngle += 2.0f * camera.y_rotate;
		if (camera.y_rotationAngle >= 360.0f)
			camera.y_rotationAngle -= 360.0f;
		else if (camera.y_rotationAngle < 0.0f)
			camera.y_rotationAngle += 360.0f;
	}
	
	if (camera.revolution != 0) {
		camera.revolutionAngle += 1.0f * camera.revolution;
		if (camera.revolutionAngle >= 360.0f)
			camera.revolutionAngle -= 360.0f;
		else if (camera.revolutionAngle < 0.0f)
			camera.revolutionAngle += 360.0f;
	}

	updateSmallBoxPositions();
	updateSphereMovement();
	
	// 작은 박스들이 경계를 벗어나면 숨김
	if (smallBoxPositions[0].y <= -2.0f || smallBoxPositions[0].x <= -2.0f || 
		smallBoxPositions[0].x >= 2.0f || smallBoxPositions[0].y >= 2.0f) {
		isOpen = false;
		showSmallBoxes = false;
	}
	
	glutPostRedisplay();
	glutTimerFunc(50, Timer, 1);
}	