#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid initBuffer();

GLint width, height;
GLuint shaderProgramID; 
GLuint vertexShader;	
GLuint fragmentShader; 
GLuint VAO, VBO[2];
GLvoid Mouse(int button, int state, int x, int y);
const GLfloat triShape[3][3]{};
const GLfloat colors[3][3]{};


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
	initBuffer();
	//--- 세이더프로그램만들기
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
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
	if(!result)
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

void initBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(2, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	//--- 변수 diamond 에서버텍스데이터값을버퍼에복사한다.
	//--- triShape 배열의 사이즈: 9 * float
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), triShape, GL_STATIC_DRAW);
	//--- 좌표값을attribute 인덱스0번에명시한다: 버텍스당3*float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute 인덱스 0번을 사용가능하게함
	glEnableVertexAttribArray(0);
	//--- 2번째 VBO를활성화하여바인드하고, 버텍스속성(색상)을저장
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	//--- 변수 colors에서 버텍스색상을복사한다.
	//--- colors 배열의 사이즈: 9 *float 
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
	//--- 색상값을attribute 인덱스1번에명시한다: 버텍스당3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute 인덱스 1번을 사용가능하게함.
	glEnableVertexAttribArray(1);
}



GLvoid drawScene()
{
	//--- 변경된배경색설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//glClearColor(1.0, 1.0, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- 렌더링파이프라인에세이더불러오기
	glUseProgram(shaderProgramID);
	//--- 사용할VAO 불러오기
	glBindVertexArray(VAO);
	//--- 삼각형그리기
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glutSwapBuffers();

}
//--- 다시그리기콜백함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	//--- 배경색을파랑색으로설정
   //--- 렌더링하기: 0번인덱스에서1개의버텍스를사용하여점그리기
   // 화면에출력하기
   //--- 콜백 함수: 다시 그리기 콜백 함수
}




