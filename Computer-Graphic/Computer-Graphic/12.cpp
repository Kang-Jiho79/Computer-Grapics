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
	float cx = 0.0f, cy = 0.0f;
	int type;
	GLuint VAO, VBO[2];
	bool changed = false;
	float animProgress = 0.0f;  // 애니메이션 진행도 (0.0 ~ 1.0)
};

Shape center_shape;
Shape shapes[4];
float x_ndc = 0.0f, y_ndc = 0.0f;
bool center = false;
const float ANIM_SPEED = 0.01f;  // 애니메이션 속도

GLvoid initBuffer(Shape& shape);

std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

void make_line(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 0;
	shape.vertices = { 
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.2f, 0.0f, shape.cx + 0.19f, shape.cy + 0.21f, 0.0f,
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f,
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	initBuffer(shape);
}

void make_triangle(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 1;
	shape.vertices = {
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	initBuffer(shape);
}

void make_square(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 2;
	shape.vertices = {
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f,
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f,
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy + 0.2f, 0.0f,
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2]
	};
	initBuffer(shape);
}

void make_pentagon(Shape& shape)
{
	float color[3] = { getRandomcolor(), getRandomcolor(), getRandomcolor() };
	shape.type = 3;
	shape.vertices = {
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy + 0.1f, 0.0f, shape.cx - 0.1f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.1f, shape.cy - 0.1f, 0.0f, shape.cx + 0.1f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx + 0.1f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy + 0.1f, 0.0f,
	};
	shape.colors = {
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
		color[0], color[1], color[2], color[0], color[1], color[2], color[0], color[1], color[2],
	};
	initBuffer(shape);
}

void change_shape(Shape& shape)
{
	if (shape.changed) return;

	// animProgress를 0으로 초기화
	shape.animProgress = 0.0f;

	switch (shape.type) {
	case 0:
		shape.changed = true;
		shape.type = 1;
		shape.dvertices = {
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f, shape.cx, shape.cy + 0.2f, 0.0f
		};
		break;
	case 1:
		shape.changed = true;
		shape.type = 2;
		shape.dvertices = {
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f,
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f,
		shape.cx - 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy + 0.2f, 0.0f,
		};
		break;
	case 2:
		shape.changed = true;
		shape.type = 3;
		shape.dvertices = {
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy + 0.1f, 0.0f, shape.cx - 0.1f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx - 0.1f, shape.cy - 0.1f, 0.0f, shape.cx + 0.1f, shape.cy - 0.1f, 0.0f,
		shape.cx, shape.cy + 0.2f, 0.0f, shape.cx + 0.1f, shape.cy - 0.1f, 0.0f, shape.cx + 0.2f, shape.cy + 0.1f, 0.0f,
		};
		break;
	case 3:
		shape.changed = true;
		shape.type = 0;
		shape.dvertices = {
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx - 0.2f, shape.cy - 0.2f, 0.0f, shape.cx + 0.19f, shape.cy + 0.21f, 0.0f,
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f,
		shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f, shape.cx + 0.2f, shape.cy + 0.2f, 0.0f
		};
		break;
	}
}

void updateVertices(Shape& shape)
{
	if (!shape.changed) return;

	shape.animProgress += ANIM_SPEED;

	if (shape.animProgress >= 1.0f) {
		shape.animProgress = 1.0f;
		shape.changed = false;
		shape.vertices = shape.dvertices;
	}
	else {
		for (size_t i = 0; i < shape.vertices.size(); ++i) {
			shape.vertices[i] = shape.vertices[i] * (1.0f - shape.animProgress) +
				shape.dvertices[i] * shape.animProgress;
		}
	}

	initBuffer(shape);
}

void initshapes()
{
	for (int i = 0; i < 4; i++) {
		shapes[i].cx = i % 2 == 0 ? -0.5f : 0.5f;
		shapes[i].cy = i / 2 == 0 ? 0.5f : -0.5f;
	}
	center_shape.cx = 0.0f;
	center_shape.cy = 0.0f;
	make_line(shapes[0]);
	make_triangle(shapes[1]);
	make_square(shapes[2]);
	make_pentagon(shapes[3]);
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
	glutTimerFunc(16, Timer, 0);
	initshapes();
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
	if (center) {
		glDrawArrays(GL_TRIANGLES, 0, center_shape.vertices.size() / 3);
	}
	else {
		for (int i = 0; i < 4; i++) {
			glBindVertexArray(shapes[i].VAO);
			glDrawArrays(GL_TRIANGLES, 0, shapes[i].vertices.size() / 3);
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
	case 'l':
		center = true;
		make_line(center_shape);
		change_shape(center_shape);
		break;
	case 't':
		center = true;
		make_triangle(center_shape);
		change_shape(center_shape);
		break;
	case 'r':
		center = true;
		make_square(center_shape);
		change_shape(center_shape);
		break;
	case 'p':
		center = true;
		make_pentagon(center_shape);
		change_shape(center_shape);
		break;
	case 'a':
		center = false;
		make_line(shapes[0]);
		make_triangle(shapes[1]);
		make_square(shapes[2]);
		make_pentagon(shapes[3]);
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

		glutPostRedisplay();
	}
}

// 타이머 콜백 함수
GLvoid Timer(int value)
{
	if (center)
		updateVertices(center_shape);
	else {
		for (int i = 0; i < 4; ++i) {
			change_shape(shapes[i]);
			updateVertices(shapes[i]);
		}
	}

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 0);
}


