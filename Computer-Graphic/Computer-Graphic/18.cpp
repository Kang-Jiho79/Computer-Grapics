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
	float center[3]{};
	float size = 0.5f;
	GLuint VAO, VBO[2], EBO;
	GLUquadricObj* obj = nullptr;
	bool x_rotate = false, y_rotate = false, revolution = false;
	bool origin_scale = false, self_scale = false;
	float translation[3] = { 0.0f };
	float x_rotationAngle = { 0.0f };
	float y_rotationAngle = { 0.0f };
	float revolutionAngle = { 0.0f };
	float origin_scale_value[3]{ 1.0f,1.0f,1.0f };
	float self_scale_value[3]{ 1.0f, 1.0f, 1.0f };
};

Shape axis;
Shape shape[2];

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

void reset() {
	
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

	std::cout << "=== 조작법 ===" << std::endl;
	std::cout << "1: 좌측, 2: 우측, 3: 양측" << std::endl;
	std::cout << "x/X: x축 자전, y/Y: Y축 자전, r/R: y축 공전" << std::endl;
	std::cout << "a/A: 확대/축소, b/B: 원점에 대해 확대/축소" << std::endl;
	std::cout << "d/D: x축에 대해 좌우이동, e/E: y축에대해 상하이동" << std::endl;
	std::cout << "t: 두 도형이 원점을 통과하며 상대방의 자리로 이동하는 애니메이션" << std::endl;
	std::cout << "u: 두 도형이 한개는 위로, 다른 도형은 아래로 이동하면서 상대방의 자리로 이동하는 애니메이션" << std::endl;
	std::cout << "v: 키보드5: 두 도형이 한개는 확대, 다른 한개는 축소되며 자전과 공전하기" << std::endl;
	std::cout << "c: 두도형을 다른 도형으로 바꾼다." << std::endl;
	std::cout << "s : 리셋" << std::endl;
	std::cout << "q: 프로그램 종료" << std::endl;

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
	for (int i = 0; i < 2; i++) {
		glm::mat4 modelMatrix = baseRotation;
		
		modelMatrix = glm::translate(modelMatrix, glm::vec3(shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(shape[i].revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		rotate_Matrix(modelMatrix, glm::vec3(shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]), shape[i].x_rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate_Matrix(modelMatrix, glm::vec3(shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]), shape[i].y_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(shape[i].origin_scale_value[0], shape[i].origin_scale_value[1], shape[i].origin_scale_value[2]));
		scale_Matrix(modelMatrix, glm::vec3(shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]), glm::vec3(shape[i].self_scale_value[0], shape[i].self_scale_value[1], shape[i].self_scale_value[2]));

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
	case 'q': // 프로그램 종료
		glutLeaveMainLoop();
		break;
		glutPostRedisplay();
	}
}

GLvoid Timer(int value)
{
	

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}


