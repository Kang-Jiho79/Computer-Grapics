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
	std::vector<float> dvertices;
	std::vector<float> colors;  // 정점별 색상 지원
	int type;
	GLuint VAO, VBO[2];
	bool isSelected = false;
};

Shape shapes[10];
float x_ndc = 0.0f, y_ndc = 0.0f;
int existingShapes = 0;
int selectedShape = -1;

GLvoid initBuffer(Shape& shape);

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

void make_line(Shape& shape, float x, float y)
{
	shape.type = 0;
	shape.vertices = { x - 0.03f, y - 0.03f, 0.0f, x + 0.03f, y + 0.03f, 0.0f };
	shape.colors = {
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor()
	};
	initBuffer(shape);
}

void make_triangle(Shape& shape, float x, float y)
{
	shape.type = 1;
	shape.vertices = { x, y + 0.1f, 0.0f, x - 0.1f, y - 0.1f, 0.0f, x + 0.1f, y - 0.1f, 0.0f };
	shape.colors = {
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
	};
	initBuffer(shape);
}

void make_square(Shape& shape, float x, float y)
{
	shape.type = 2;
	shape.vertices = { x - 0.1f, y + 0.1f, 0.0f, x - 0.1f, y - 0.1f, 0.0f, x + 0.1f, y - 0.1f, 0.0f,
					   x - 0.1f, y + 0.1f, 0.0f, x + 0.1f, y - 0.1f, 0.0f, x + 0.1f, y + 0.1f, 0.0f };
	shape.colors = {
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
	};
	initBuffer(shape);
}

void clear()
{
	for (int i = 0; i < existingShapes; i++)
	{
		Shape& shape = shapes[i];
		glDeleteVertexArrays(1, &shape.VAO);
		glDeleteBuffers(2, shape.VBO);
	}
	existingShapes = 0;
	selectedShape = -1;
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
	//--- 프래그먼트세이더만들기
	make_shaderProgram();
	//--- 세이더프로그램만들기
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
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
	for (int i = 0; i < existingShapes; i++)
	{
		Shape& shape = shapes[i];
		glBindVertexArray(shape.VAO);
		if (shape.type == 0)
		{
			glPointSize(10.0f);
			glDrawArrays(GL_POINTS, 0, 1);
		}
		else if (shape.type == 1)
		{
			glLineWidth(5.0f);
			glDrawArrays(GL_LINES, 0, 2);
		}
		else if (shape.type == 2)
		{
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else if (shape.type == 3)
		{
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
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
	case 'p':
		if (existingShapes < 10) {
			Shape& shape = shapes[existingShapes++];
			make_dot(shape, x_ndc, y_ndc);
			initBuffer(shape);
		}
		break;
	case 'e':
		if (existingShapes < 10) {
			Shape& shape = shapes[existingShapes++];
			make_line(shape, x_ndc, y_ndc);
			initBuffer(shape);
		}
		break;
	case 't':
		if (existingShapes < 10) {
			Shape& shape = shapes[existingShapes++];
			make_triangle(shape, x_ndc, y_ndc);
			initBuffer(shape);
		}
		break;
	case 'r':
		if (existingShapes < 10) {
			Shape& shape = shapes[existingShapes++];
			make_square(shape, x_ndc, y_ndc);
			initBuffer(shape);
		}
		break;
	case 'w': // 위로 이동
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'a': // 왼쪽으로 이동
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 's': // 아래로 이동
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'd':
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'i':
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'j':
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'k':
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'l':
		if (selectedShape != -1) {
			Shape& shape = shapes[selectedShape];
			for (size_t i = 0; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] += 0.05f;
			}
			for (size_t i = 1; i < shape.vertices.size(); i += 3) {
				shape.vertices[i] -= 0.05f;
			}
			initBuffer(shape);
		}
		break;
	case 'c':
		clear();
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

		glutPostRedisplay();
	}
}


