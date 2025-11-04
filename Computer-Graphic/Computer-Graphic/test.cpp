#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// 객체 구조체
struct Shape {
    std::vector<float> vertices;
    std::vector<unsigned int> index;
    std::vector<float> colors;
    std::vector<glm::mat4> Matrices;
    GLuint VAO, VBO[2], EBO;
};
Shape cube;
GLUquadric* sphere;

char* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb"); // Open file for reading
    if (!fptr) // Return NULL on failure
        return NULL;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file
    length = ftell(fptr); // Find out how many bytes into the file we are
    buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
    fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
    fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
    fclose(fptr); // Close the file
    buf[length] = 0; // Null terminator
    return buf; // Return the buffer
}

//--- 사용자 정의 함수 선언
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKeys(int key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
float MouseX(int x) { return 2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f; }
float MouseY(int y) { return 1.0f - 2.0f * y / glutGet(GLUT_WINDOW_HEIGHT); }
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void CreateCube(Shape& cube, float x, float y, float z);

//--- 필요한 변수 선언
GLint width = 800, height = 600;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example");
    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    glewInit();
    //--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
    make_vertexShaders(); //--- 버텍스 세이더 만들기
    make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    shaderProgramID = make_shaderProgram();
    //--- 콜백 함수 등록
    glEnable(GL_DEPTH_TEST);

    CreateCube(cube, 0.5f, 0.5f, 0.5f); // 큐브 생성

    glutTimerFunc(50, TimerFunction, 1); // 타이머 함수 등록
    glutDisplayFunc(drawScene); // 출력 함수의 지정
    glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
    glutKeyboardFunc(Keyboard); // 키보드 입력
    glutSpecialFunc(SpecialKeys); // 특수키 입력
    glutMouseFunc(Mouse); // 마우스 입력
    glutMotionFunc(Motion); // 마우스 움직임
    glutMainLoop();
}

// 큐브 생성 함수
void CreateCube(Shape& a, float x, float y, float z) {
    a.vertices = {
        // 앞면
        -x, y, -z,
        -x, -y, -z,
        x, -y, -z,
        x, y, -z,

        // 뒷면
        x, y, z,
        x, -y, z,
        -x, -y, z,
        -x, y, z,

        // 아래면
        -x, -y, -z,
        -x, -y, z,
        x, -y, z,
        x, -y, -z,

        // 윗면 
        -x, y, z,
        -x, y, -z,
        x, y, -z,
        x, y, z,

        // 오른면
        x, y, -z,
        x, -y, -z,
        x, -y, z,
        x, y, z,

        // 왼면 
        -x, y, z,
        -x, -y, z,
        -x, -y, -z,
        -x, y, -z
    };
    a.index = {
        // 앞 / 뒤 / 아래 / 위 / 오 / 왼
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    InitBuffers(cube);
}

// 버퍼 설정 함수
GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);           // 버텍스 배열 객체id 생성
    glBindVertexArray(shape.VAO);               // 버텍스 배열 객체 바인딩

    glGenBuffers(2, shape.VBO);                 // 버퍼id 생성

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);   // 좌표
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &shape.EBO);                 // 인덱스 버퍼id 생성
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);   // 색상
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
}

// 키보드 콜백 함수
GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 특수키 콜백 함수
GLvoid SpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        break;
    case GLUT_KEY_DOWN:
        break;
    case GLUT_KEY_LEFT:
        break;
    case GLUT_KEY_RIGHT:
        break;
    }
    glutPostRedisplay();
}

// 마우스 콜백 함수
GLvoid Mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {}
    glutPostRedisplay();
}

// 마우스 움직임 콜백 함수
void Motion(int x, int y) {
    glutPostRedisplay();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
    GLchar* vertexSource;
    //--- 버텍스 세이더 읽어 저장하고 컴파일 하기
    //--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
    vertexSource = filetobuf("vertex_3d.glsl");
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
        return;
    }
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
    GLchar* fragmentSource;
    //--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
    fragmentSource = filetobuf("fragment_3d.glsl"); // 프래그세이더 읽어오기
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

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
    GLuint shaderID;
    GLint result;
    GLchar errorLog[512];
    shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
    glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
    glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
    glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
    glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return false;
    }
    glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
    //--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
    //--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
    //--- 사용하기 직전에 호출할 수 있다.
    return shaderID;
}

// 출력 콜백 함수
GLvoid drawScene() {
    // 배경을 흰색으로 설정
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 은면 제거
    glEnable(GL_DEPTH_TEST);
    // 컬링
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glUseProgram(shaderProgramID);
    unsigned int transformLocation = glGetUniformLocation(shaderProgramID, "matrix");

    // 투영 행렬 설정
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);

    // 도형 그리기
    glm::mat4 model = glm::mat4(1.0f);
    model = projection * model;
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(cube.VAO);
    glDrawElements(GL_TRIANGLES, cube.index.size(), GL_UNSIGNED_INT, 0);
    // 분할해서 출력할때
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
    // 구 그리기
 //   sphere = gluNewQuadric();
 //   gluQuadricDrawStyle(sphere, GLU_FILL);
 //   //glm::mat4 model = glm::mat4(1.0f);
 //   glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(model));
    //gluSphere(sphere, 0.5, 20, 20); // 0.5 = 반지름, 20 = 세로분할, 20 = 가로분할
 //   gluDeleteQuadric(sphere);

    glutSwapBuffers();
}

// 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) {
    width = w; height = h;
    glViewport(0, 0, w, h);
}

// 타이머 콜백 함수
void TimerFunction(int value) {

    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}