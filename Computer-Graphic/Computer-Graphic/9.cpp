#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

#define MaxShapes 4

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

class Shape {
public:
	std::vector<float> vertices;
	std::vector<float> colors;  // 정점별 색상 지원
	int type;
	GLuint VAO, VBO[2];
};

Shape shapes[MaxShapes][MaxShapes];
float x_ndc = 0.0f, y_ndc = 0.0f;
int existingShapes[MaxShapes]{};
bool lineMode = false;

Shape xAxis, yAxis;

GLvoid initBuffer(Shape& shape);

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

float getRandomfloat(float min = 0.05f, float max = 0.2f)
{
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

void createAxes()
{
	xAxis.vertices = { -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	xAxis.colors = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	yAxis.vertices = { 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
	yAxis.colors = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; 
	initBuffer(xAxis);
	initBuffer(yAxis);
}

void drawAxes()
{
	glLineWidth(2.0f); 
	glBindVertexArray(xAxis.VAO);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(yAxis.VAO);
	glDrawArrays(GL_LINES, 0, 2);
}
void make_triangle(Shape& shape, float x, float y)
{
	float size = getRandomfloat();
	shape.type = 2;
	shape.vertices = { x, y + size, 0.0f, x - size, y - size, 0.0f, x + size, y - size, 0.0f };
	shape.colors = {
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor(),
		getRandomcolor(), getRandomcolor(), getRandomcolor()
	};
	initBuffer(shape);
}

void clear()
{
	for (int i = 0; i < MaxShapes; i++)
	{
		for (int j = 0; j < MaxShapes; j++)
		{
			Shape& shape = shapes[i][j];
			glDeleteVertexArrays(1, &shape.VAO);
			glDeleteBuffers(2, shape.VBO);
		}
		existingShapes[i] = 0;
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
	glutCreateWindow("Example1");
	glewExperimental = GL_TRUE;
	glewInit();
	//--- 프래그먼트세이더만들기
	make_shaderProgram();
	//--- 세이더프로그램만들기
	glutDisplayFunc(drawScene);
	createAxes();
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
	drawAxes();
	for (int i = 0; i < MaxShapes; i++)
	{
		for (int j = 0; j < existingShapes[i]; j++) {
			Shape& shape = shapes[i][j];
			glBindVertexArray(shape.VAO);
			if (lineMode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	glutSwapBuffers();

}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	x_ndc = (2.0f * x / width - 1.0f);
	y_ndc = -(2.0f * y / height - 1.0f);
	switch (key) {
	case 'a':
		lineMode = false;
	break;
	case 'b':
		lineMode = true;
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

		if (x_ndc < 0.0f && y_ndc >= 0.0f)
		{
			make_triangle(shapes[0][0], x_ndc, y_ndc);
			if (existingShapes[0] == 0)
				existingShapes[0] = 1;
		}
		else if (x_ndc >= 0.0f && y_ndc >= 0.0f )
		{
			make_triangle(shapes[1][0], x_ndc, y_ndc);
			if (existingShapes[1] == 0)
				existingShapes[1] = 1;
		}
		else if (x_ndc < 0.0f && y_ndc < 0.0f)
		{
			make_triangle(shapes[2][0], x_ndc, y_ndc);
			if (existingShapes[2] == 0)
				existingShapes[2] = 1;
		}
		else if (x_ndc >= 0.0f && y_ndc < 0.0f)
		{
			make_triangle(shapes[3][0], x_ndc, y_ndc);
			if (existingShapes[3] == 0)
				existingShapes[3] = 1;
		}

		glutPostRedisplay();
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		x_ndc = (2.0f * x / width - 1.0f);
		y_ndc = -(2.0f * y / height - 1.0f);
		if (x_ndc < 0.0f && y_ndc >= 0.0f)
		{
			if (existingShapes[0] == MaxShapes) {
				for (int i = 1; i < MaxShapes; i++) {
					shapes[0][i - 1] = shapes[0][i];
				}
				make_triangle(shapes[0][MaxShapes - 1], x_ndc, y_ndc);
			}
			else {
				make_triangle(shapes[0][existingShapes[0]], x_ndc, y_ndc);
				existingShapes[0]++;
			}
		}
		else if (x_ndc >= 0.0f && y_ndc >= 0.0f)
		{
			if (existingShapes[1] == MaxShapes) {
				for (int i = 1; i < MaxShapes; i++) {
					shapes[1][i - 1] = shapes[1][i];
				}
				make_triangle(shapes[1][MaxShapes - 1], x_ndc, y_ndc);
			}
			else {
				make_triangle(shapes[1][existingShapes[1]], x_ndc, y_ndc);
				existingShapes[1]++;
			}
		}
		else if (x_ndc < 0.0f && y_ndc < 0.0f)
		{
			if( existingShapes[2] == MaxShapes) {
				for (int i = 1; i < MaxShapes; i++) {
					shapes[2][i - 1] = shapes[2][i];
				}
				make_triangle(shapes[2][MaxShapes - 1], x_ndc, y_ndc);
			}
			else {
				make_triangle(shapes[2][existingShapes[2]], x_ndc, y_ndc);
				existingShapes[2]++;
			}
		}
		else if (x_ndc >= 0.0f && y_ndc < 0.0f)
		{
			if (existingShapes[3] == MaxShapes) {
				for (int i = 1; i < MaxShapes; i++) {
					shapes[3][i - 1] = shapes[3][i];
				}
				make_triangle(shapes[3][MaxShapes - 1], x_ndc, y_ndc);
			}
			else {
				make_triangle(shapes[3][existingShapes[3]], x_ndc, y_ndc);
				existingShapes[3]++;
			}
		}
		glutPostRedisplay();
	}
}


