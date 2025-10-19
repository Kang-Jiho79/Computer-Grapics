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

bool cube_pyramid = true;	// true: cube, false: pyramid
bool depthTest = true;
bool culling = false;
bool wireframe = false;
int x_rotate = 0;
int y_rotate = 0;
float translation[3] = { 0.0f };
float x_rotationAngle = { 0.0f };
float y_rotationAngle = { 0.0f };

// 육면체 애니메이션 변수

bool Cube_Face_0_Animation = false;	// 앞면
bool Cube_Face_1_Animation = false;	// 오른쪽면
bool Cube_Face_2_Animation = false;	// 뒷면
bool Cube_Face_3_Animation = false;  // 왼쪽면
bool Cube_Face_4_Animation = false;	// 윗면

float Cube_Face_0_Angle = 0.0f;
float Cube_Face_1_Angle = 0.0f;
float Cube_Face_2_Angle = 0.0f;
float Cube_Face_3_Angle = 0.0f;
float Cube_Face_4_Angle = 0.0f;

bool Cube_Face_1_back = false;
bool Cube_Face_3_back = false;

// 사각뿔 애니메이션 변수
bool Pyramid_Face_1_Animation = false;
bool Pyramid_Face_2_Animation = false;
bool Pyramid_Face_3_Animation = false;
bool Pyramid_Face_4_Animation = false;

float Pyramid_Face_1_Angle = 0.0f;
float Pyramid_Face_2_Angle = 0.0f;
float Pyramid_Face_3_Angle = 0.0f;
float Pyramid_Face_4_Angle = 0.0f;

bool Pyramid_Face_1_back = false;
bool Pyramid_Face_2_back = false;
bool Pyramid_Face_3_back = false;
bool Pyramid_Face_4_back = false;


bool pyramidOpenClose = false;
bool pyramid_back = false;

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
		// 앞면 (면 1) - 빨간색
		-0.5f, -0.5f, 0.5f,   // 0
		 0.5f, -0.5f, 0.5f,   // 1
		 0.5f,  0.5f, 0.5f,   // 2
		-0.5f,  0.5f, 0.5f,   // 3

		// 오른쪽면 (면 2) - 초록색
		 0.5f, -0.5f, 0.5f,   // 4
		 0.5f, -0.5f, -0.5f,  // 5
		 0.5f,  0.5f, -0.5f,  // 6
		 0.5f,  0.5f, 0.5f,   // 7

		 // 뒷면 (면 3) - 파란색
		  0.5f, -0.5f, -0.5f,  // 8
		 -0.5f, -0.5f, -0.5f,  // 9
		 -0.5f,  0.5f, -0.5f,  // 10
		  0.5f,  0.5f, -0.5f,  // 11

		  // 왼쪽면 (면 4) - 노란색
		  -0.5f, -0.5f, -0.5f,  // 12
		  -0.5f, -0.5f, 0.5f,   // 13
		  -0.5f,  0.5f, 0.5f,   // 14
		  -0.5f,  0.5f, -0.5f,  // 15

		  // 윗면 (면 5) - 자홍색
		  -0.5f,  0.5f, 0.5f,   // 16
		   0.5f,  0.5f, 0.5f,   // 17
		   0.5f,  0.5f, -0.5f,  // 18
		  -0.5f,  0.5f, -0.5f,  // 19

		  // 아랫면 (면 6) - 청록색
		  -0.5f, -0.5f, -0.5f,  // 20
		   0.5f, -0.5f, -0.5f,  // 21
		   0.5f, -0.5f, 0.5f,   // 22
		  -0.5f, -0.5f, 0.5f    // 23
	};

	shape.colors = {
		// 앞면 - 빨간색
		1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		// 오른쪽면 - 초록색
		0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
		// 뒷면 - 파란색
		0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
		// 왼쪽면 - 노란색
		1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
		// 윗면 - 자홍색
		1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,
		// 아랫면 - 청록색
		0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f
	};

	shape.index = {
		0, 1, 2,  2, 3, 0,     // 앞면
		4, 5, 6,  6, 7, 4,     // 오른쪽면
		8, 9, 10, 10, 11, 8,   // 뒷면
		12, 13, 14, 14, 15, 12, // 왼쪽면
		16, 17, 18, 18, 19, 16, // 윗면
		20, 21, 22, 22, 23, 20  // 아랫면
	};
	initBuffer(shape);
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

void reset() {
	depthTest = true;
	culling = false;
	y_rotate = 0;
	x_rotationAngle = 0.0f;
	y_rotationAngle = 0.0f;
	Cube_Face_0_Animation = false;
	Cube_Face_1_Animation = false;
	Cube_Face_2_Animation = false;
	Cube_Face_3_Animation = false;
	Cube_Face_4_Animation = false;
	Cube_Face_0_Angle = 0.0f;
	Cube_Face_1_Angle = 0.0f;
	Cube_Face_2_Angle = 0.0f;
	Cube_Face_3_Angle = 0.0f;
	Cube_Face_4_Angle = 0.0f;
	Cube_Face_1_back = false;
	Cube_Face_3_back = false;
	Pyramid_Face_1_Animation = false;
	Pyramid_Face_2_Animation = false;
	Pyramid_Face_3_Animation = false;
	Pyramid_Face_4_Animation = false;
	Pyramid_Face_1_Angle = 0.0f;
	Pyramid_Face_2_Angle = 0.0f;
	Pyramid_Face_3_Angle = 0.0f;
	Pyramid_Face_4_Angle = 0.0f;
	Pyramid_Face_1_back = false;
	Pyramid_Face_2_back = false;
	Pyramid_Face_3_back = false;
	Pyramid_Face_4_back = false;
	pyramidOpenClose = false;
	pyramid_back = false;
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

void rotate_Matrix(glm::mat4& matrix, glm::vec3 pre_trans, float angle, glm::vec3 rotate)
{
	matrix = glm::translate(matrix, pre_trans);
	matrix = glm::rotate(matrix, glm::radians(angle), rotate);
	matrix = glm::translate(matrix, -pre_trans);
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

	// 깊이 테스트 활성화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 셰이더 프로그램 먼저 생성
	make_shaderProgram();

	createAxis(axis);
	createCube(cube);
	createPyramid(pyramid);

	std::cout << "=== 조작법 ===" << std::endl;
	std::cout << "p: 정육면체, 피라미드" << std::endl;
	std::cout << "y: Y축 회전" << std::endl;
	std::cout << "h: 은면제거, u: 뒷면제거" << std::endl;
	std::cout << "c : 리셋" << std::endl;
	std::cout << "q: 프로그램 종료" << std::endl;
	std::cout << "=== 육면체 조작법 ===" << std::endl;
	std::cout << "t: 윗면 회전" << std::endl;
	std::cout << "f: 앞면 회전" << std::endl;
	std::cout << "s: 옆면 회전" << std::endl;
	std::cout << "b: 뒷면 크기변경" << std::endl;
	std::cout << "=== 사각뿔 조작법 ===" << std::endl;
	std::cout << "o: 모든면 열기닫기" << std::endl;
	std::cout << "r: 차례대로 열기닫기" << std::endl;

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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 검은 배경
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

	// 기본 객체 변환
	glm::mat4 objectMatrix = baseRotation;
	objectMatrix = glm::translate(objectMatrix, glm::vec3(translation[0], translation[1], translation[2]));
	objectMatrix = glm::rotate(objectMatrix, glm::radians(x_rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
	objectMatrix = glm::rotate(objectMatrix, glm::radians(y_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glBindVertexArray(cube_pyramid ? cube.VAO : pyramid.VAO);

	if (cube_pyramid) {
		// 정육면체 그리기

		glm::mat4 Matrix_0 = objectMatrix;	// 앞면
		rotate_Matrix(Matrix_0, glm::vec3(0.0f, 0.0f, 0.0f), Cube_Face_0_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
		drawCubeFace(0, projection * view * Matrix_0, modelLocation);

		glm::mat4 Matrix_1 = objectMatrix;
		rotate_Matrix(Matrix_1, glm::vec3(0.5f, 0.5f, 0.0f), Cube_Face_1_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
		drawCubeFace(1, projection* view* Matrix_1, modelLocation);

		glm::mat4 Matrix_2 = objectMatrix;
		rotate_Matrix(Matrix_2, glm::vec3(0.0f, 0.0f, 0.0f), Cube_Face_2_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
		drawCubeFace(2, projection * view * Matrix_2, modelLocation);

		glm::mat4 Matrix_3 = objectMatrix;
		float scale = 1.0f - (Cube_Face_3_Angle / 90.0f);
		Matrix_3 = glm::scale(Matrix_3, glm::vec3(1.0f, scale, scale));
		drawCubeFace(3, projection * view * Matrix_3, modelLocation);

		glm::mat4 Matrix_4 = objectMatrix;
		rotate_Matrix(Matrix_4, glm::vec3(0.0f, 0.5f, 0.0f), Cube_Face_4_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
		drawCubeFace(4, projection * view * Matrix_4, modelLocation);

		glm::mat4 Matrix_5 = objectMatrix;
		drawCubeFace(5, projection * view * Matrix_5, modelLocation);

	}
	else {
		// 사각뿔 그리기

		glm::mat4 Matrix_0 = objectMatrix;
		drawPyramidFace(0, projection * view * Matrix_0, modelLocation);

		glm::mat4 Matrix_1 = objectMatrix;
		rotate_Matrix(Matrix_1, glm::vec3(0.0f, -0.5f, 0.5f), Pyramid_Face_1_Angle, glm::vec3(1.0f, 0.0f, 0.0f));
		drawPyramidFace(1, projection * view * Matrix_1, modelLocation);

		glm::mat4 Matrix_2 = objectMatrix;
		rotate_Matrix(Matrix_2, glm::vec3(0.5f, -0.5f, 0.0f), Pyramid_Face_2_Angle, glm::vec3(0.0f, 0.0f, -1.0f));
		drawPyramidFace(2, projection * view * Matrix_2, modelLocation);

		glm::mat4 Matrix_3 = objectMatrix;
		rotate_Matrix(Matrix_3, glm::vec3(0.0f, -0.5f, -0.5f), Pyramid_Face_3_Angle, glm::vec3(-1.0f, 0.0f, 0.0f));
		drawPyramidFace(3, projection * view * Matrix_3, modelLocation);

		glm::mat4 Matrix_4 = objectMatrix;
		rotate_Matrix(Matrix_4, glm::vec3(-0.5f, -0.5f, 0.0f), Pyramid_Face_4_Angle, glm::vec3(0.0f, 0.0f, 1.0f));
		drawPyramidFace(4, projection * view * Matrix_4, modelLocation);
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
	case 'h': // 은면제거 설정/해제
		depthTest = !depthTest;
		culling = false;
		std::cout << "은면제거: " << (depthTest ? "ON" : "OFF") << std::endl;
		break;
	case 'u': // 뒷면제거 설정/해제
		culling = !culling;
		depthTest = false;
		std::cout << "뒷면제거: " << (culling ? "ON" : "OFF") << std::endl;
		break;
	case 'p': // 육면체/사각뿔
		cube_pyramid = !cube_pyramid;
		std::cout << (cube_pyramid ? "정육면체" : "사각뿔") << " 모드" << std::endl;
		break;
	case 'y': // 뷰 대화여 자전한다/멈춘다
		if (y_rotate == 0) y_rotate = 1;
		else y_rotate = 0;
		std::cout << "자전: " << (y_rotate ? "ON" : "OFF") << std::endl;
		break;
	case 'c': // 리셋하고 육면체 출력
		reset();
		cube_pyramid = true;
		std::cout << "리셋 및 정육면체 출력" << std::endl;
		break;
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;

		// 육면체 애니메이션
	if (cube_pyramid) {
	case 't':
		Cube_Face_4_Animation = !Cube_Face_4_Animation;
		break;
	case 'f':
		Cube_Face_1_Animation = !Cube_Face_1_Animation;
		break;
	case 's':
		Cube_Face_0_Animation = !Cube_Face_0_Animation;
		Cube_Face_2_Animation = !Cube_Face_2_Animation;
		break;
	case 'b':
		Cube_Face_3_Animation = !Cube_Face_3_Animation;
		break;
	}
	// 사각뿔 애니메이션
	else {
	case 'o': // 사각뿔의 모든 면이 함께 열린다/닫는다
		pyramidOpenClose = !pyramidOpenClose;
		break;
	case 'r': // 사각뿔이 각 면이 한 개씩 연다/닫는다
		Pyramid_Face_1_Animation = true;
		break;
	}
	}
	glutPostRedisplay();
}

GLvoid SpecialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP: // 위쪽 방향키
		translation[1] += 0.1f;
		break;
	case GLUT_KEY_DOWN: // 아래쪽 방향키
		translation[1] -= 0.1f;
		break;
	case GLUT_KEY_LEFT: // 왼쪽 방향키
		translation[0] -= 0.1f;
		break;
	case GLUT_KEY_RIGHT: // 오른쪽 방향키
		translation[0] += 0.1f;
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value)
{
	// 기본 회전
	
	if (y_rotate == 1) {
		y_rotationAngle += 1.0f;
		if (y_rotationAngle >= 360.0f) y_rotationAngle = 0.0f;
	}

	// 정육면체 애니메이션
	if (Cube_Face_0_Animation) {
		Cube_Face_0_Angle += 1.0f;
		if (Cube_Face_0_Angle >= 360.0f) Cube_Face_0_Angle = 0.0f;
	}
	if (Cube_Face_1_Animation) {
		if (Cube_Face_1_back) {
			Cube_Face_1_Angle -= 1.0f;
			if (Cube_Face_1_Angle <= 0.0f) {
				Cube_Face_1_Angle = 0.0f;
				Cube_Face_1_back = false;
				Cube_Face_1_Animation = false;
			}
		}
		else {
			Cube_Face_1_Angle += 1.0f;
			if (Cube_Face_1_Angle >= 90.0f) { 
				Cube_Face_1_Angle = 90.0f;
				Cube_Face_1_back = true;
			}
		}
	}
	if (Cube_Face_2_Animation) {
		Cube_Face_2_Angle += 1.0f;
		if (Cube_Face_2_Angle >= 360.0f) Cube_Face_2_Angle = 0.0f;
	}
	if (Cube_Face_3_Animation) {
		if (Cube_Face_3_back) {
			Cube_Face_3_Angle -= 1.0f;
			if (Cube_Face_3_Angle <= 0.0f) {
				Cube_Face_3_Angle = 0.0f;
				Cube_Face_3_back = false;
				Cube_Face_3_Animation = false;
			}
		}
		else {
			Cube_Face_3_Angle += 1.0f;
			if (Cube_Face_3_Angle >= 90.0f) {
				Cube_Face_3_Angle = 90.0f;
				Cube_Face_3_back = true;
			}
		}
	}
	if (Cube_Face_4_Animation) {
		Cube_Face_4_Angle += 1.0f;
		if (Cube_Face_4_Angle >= 360.0f) Cube_Face_4_Angle = 0.0f;
	}

	// 사각뿔 애니메이션
	if (pyramidOpenClose) {
		if (pyramid_back) {
			Pyramid_Face_1_Angle -= 1.0f;
			Pyramid_Face_2_Angle -= 1.0f;
			Pyramid_Face_3_Angle -= 1.0f;
			Pyramid_Face_4_Angle -= 1.0f;
			if (Pyramid_Face_1_Angle <= 0.0f) {
				Pyramid_Face_1_Angle = 0.0f;
				Pyramid_Face_2_Angle = 0.0f;
				Pyramid_Face_3_Angle = 0.0f;
				Pyramid_Face_4_Angle = 0.0f;
				pyramid_back = false;
			}
		}
		else {
			Pyramid_Face_1_Angle += 1.0f;
			Pyramid_Face_2_Angle += 1.0f;
			Pyramid_Face_3_Angle += 1.0f;
			Pyramid_Face_4_Angle += 1.0f;
			if (Pyramid_Face_1_Angle >= 235.0f) {
				Pyramid_Face_1_Angle = 235.0f;
				Pyramid_Face_2_Angle = 235.0f;
				Pyramid_Face_3_Angle = 235.0f;
				Pyramid_Face_4_Angle = 235.0f;
				pyramid_back = true;
			}
		}
	}
	
	// 1면
	if (Pyramid_Face_1_Animation) {
		if (Pyramid_Face_1_back) {
			Pyramid_Face_1_Angle -= 1.0f;
			if (Pyramid_Face_1_Angle <= 0.0f) {
				Pyramid_Face_1_Angle = 0.0f;
				Pyramid_Face_1_back = false;
				Pyramid_Face_1_Animation = false;
				Pyramid_Face_2_Animation = true;
			}
		}
		else {
			Pyramid_Face_1_Angle += 1.0f;
			if (Pyramid_Face_1_Angle >= 90.0f) {
				Pyramid_Face_1_Angle = 90.0f;
				Pyramid_Face_1_back = true;
				Pyramid_Face_1_Animation = false;
				Pyramid_Face_2_Animation = true;
			}
		}
	}

	// 2면
	if (Pyramid_Face_2_Animation) {
		if (Pyramid_Face_2_back) {
			Pyramid_Face_2_Angle -= 1.0f;
			if (Pyramid_Face_2_Angle <= 0.0f) {
				Pyramid_Face_2_Angle = 0.0f;
				Pyramid_Face_2_back = false;
				Pyramid_Face_2_Animation = false;
				Pyramid_Face_3_Animation = true;
			}
		}
		else {
			Pyramid_Face_2_Angle += 1.0f;
			if (Pyramid_Face_2_Angle >= 90.0f) {
				Pyramid_Face_2_Angle = 90.0f;
				Pyramid_Face_2_back = true;
				Pyramid_Face_2_Animation = false;
				Pyramid_Face_3_Animation = true;
			}
		}
	}

	// 3면
	if (Pyramid_Face_3_Animation) {
		if (Pyramid_Face_3_back) {
			Pyramid_Face_3_Angle -= 1.0f;
			if (Pyramid_Face_3_Angle <= 0.0f) {
				Pyramid_Face_3_Angle = 0.0f;
				Pyramid_Face_3_back = false;
				Pyramid_Face_3_Animation = false;
				Pyramid_Face_4_Animation = true;
			}
		}
		else {
			Pyramid_Face_3_Angle += 1.0f;
			if (Pyramid_Face_3_Angle >= 90.0f) {
				Pyramid_Face_3_Angle = 90.0f;
				Pyramid_Face_3_back = true;
				Pyramid_Face_3_Animation = false;
				Pyramid_Face_4_Animation = true;
			}
		}
	}

	// 4면
	if (Pyramid_Face_4_Animation) {
		if (Pyramid_Face_4_back) {
			Pyramid_Face_4_Angle -= 1.0f;
			if (Pyramid_Face_4_Angle <= 0.0f) {
				Pyramid_Face_4_Angle = 0.0f;
				Pyramid_Face_4_back = false;
				Pyramid_Face_4_Animation = false;
			}
		}
		else {
			Pyramid_Face_4_Angle += 1.0f;
			if (Pyramid_Face_4_Angle >= 90.0f) {
				Pyramid_Face_4_Angle = 90.0f;
				Pyramid_Face_4_back = true;
				Pyramid_Face_4_Animation = false;
				Pyramid_Face_1_Animation = true;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}


