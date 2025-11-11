#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>

#define M_PI 3.14159265358979323846

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

// 색상 정의
struct Color {
    float r, g, b;
};

Color planetColors[3] = {
    {1.0f, 0.5f, 0.0f},  // 주황색
    {0.0f, 1.0f, 0.5f},  // 연두색  
    {0.5f, 0.0f, 1.0f}   // 보라색
};

Color moonColors[3] = {
    {1.0f, 1.0f, 0.0f},  // 노란색
    {0.0f, 1.0f, 1.0f},  // 청록색
    {1.0f, 0.0f, 1.0f}   // 자홍색
};

class Shape {
public:
    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<int> index;
    float center[3]{};
    float size = 0.5f;
    GLuint VAO, VBO[2], EBO;
    GLUquadricObj* obj = nullptr;
    int type = 0;
    
    // 변환 관련 변수들
    float translation[3] = { 0.0f };
    float revolutionAngle = { 0.0f };
    
    // 애니메이션 관련 변수
    bool isAnimating = false;
    float animProgress = 0.0f;
    float startPos[3] = { 0.0f };
    float targetPos[3] = { 0.0f };
    float animSpeed = 0.02f;
    int animType = 0;

    void createOrbit(float radius, int segments) {
        vertices.clear();
        index.clear();
        colors.clear();
        type = 0;

        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            vertices.push_back(x);
            vertices.push_back(0.0f);
            vertices.push_back(z);

            colors.push_back(0.7f);
            colors.push_back(0.7f);
            colors.push_back(0.7f);

            index.push_back(i);
        }
    }

    void createSphere(float radius = 0.5f) {
        obj = gluNewQuadric();
        gluQuadricDrawStyle(obj, GLU_LINE);
        gluQuadricNormals(obj, GLU_SMOOTH);
        gluQuadricTexture(obj, GL_FALSE);
        size = radius;
        type = 1;
    }

    void getTransformedPosition(float outPos[3]) const {
        glm::mat4 baseRotation = glm::mat4(1.0f);
        baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        
        glm::mat4 modelMatrix = baseRotation;
        modelMatrix = glm::rotate(modelMatrix, glm::radians(revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(translation[0], translation[1], translation[2]));

        glm::vec4 transformedPos = modelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        outPos[0] = transformedPos.x;
        outPos[1] = transformedPos.y;
        outPos[2] = transformedPos.z;
    }

    ~Shape() {
        if (obj) {
            gluDeleteQuadric(obj);
        }
    }
};

// 전역 변수들
Shape orbits[3];
Shape s_orbits[3];
Shape centerSphere;
Shape planetSpheres[3];
Shape moonSpheres[3];
Shape axis;

glm::mat4 Matrix[3];
glm::mat4 s_Matrix[3];
glm::mat4 smat[3];
glm::mat4 big_Matrix;
glm::mat4 scalemat(1.0f);
glm::mat4 zmat(1.0f);
glm::mat4 movemat(1.0f);

bool solid = true, angle = false, z_rotate = false;
bool isGlobalAnimating = false;
float scale = 1.0f, zangle = 1.0f;
int currentAnimationType = 0;

// 카메라 거리 변수 추가
float cameraDistance = 3.0f;
const float MIN_CAMERA_DISTANCE = 1.0f;
const float MAX_CAMERA_DISTANCE = 10.0f;
const float CAMERA_MOVE_SPEED = 0.2f;

GLint width = 500, height = 500;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

// 함수 선언
void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKeys(int key, int x, int y);
void TimerFunction(int value);
void CreateMatrix();
void menu();
void scaling(float s);
void updateAnimations();
void startOriginPassAnimation();
void startUpDownAnimation();
GLvoid initBuffer(Shape& shape);
void createAxis(Shape& shape);

char* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr) return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

void createAxis(Shape& shape) {
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
    shape.index = { 0, 1, 2, 3, 4, 5 };
    initBuffer(shape);
}

GLvoid initBuffer(Shape& shape) {
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

    if (!shape.index.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(int), shape.index.data(), GL_STATIC_DRAW);
    }
}

void main(int argc, char** argv) {
    width = 500;
    height = 500;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Solar System Animation");
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    glEnable(GL_DEPTH_TEST); // 은면 제거
    glDepthFunc(GL_LESS);
    
    make_shaderProgram();
    createAxis(axis);
    menu();
    CreateMatrix();
    
    // 궤도 생성
    for (int i = 0; i < 3; i++) {
        orbits[i].createOrbit(0.8f, 100);        // 행성 궤도
        initBuffer(orbits[i]);
        s_orbits[i].createOrbit(0.3f, 100);      // 달 궤도
        initBuffer(s_orbits[i]);
    }
    
    // 구체들 생성
    centerSphere.createSphere(0.15f);            // 중심 구
    for (int i = 0; i < 3; i++) {
        planetSpheres[i].createSphere(0.08f);    // 행성들
        moonSpheres[i].createSphere(0.04f);      // 달들
    }
    
    glutTimerFunc(16, TimerFunction, 0);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);
    glutMainLoop();
}

void menu() {
    std::cout << "=== 태양계 시뮬레이션 ===" << std::endl;
    std::cout << "p/P: 직각투영 / 원근투영" << std::endl;
    std::cout << "m/M: 솔리드 / 와이어프레임" << std::endl;
    std::cout << "w/a/s/d: 상하좌우 이동" << std::endl;
    std::cout << "+/-: 카메라 가까이/멀리 (줌 인/아웃)" << std::endl;
    std::cout << "y/Y: 궤도 반지름 크기 조절" << std::endl;
    std::cout << "z/Z: 행성/달 z축 회전" << std::endl;
    std::cout << "q: 종료" << std::endl;
}

void CreateMatrix() {
    big_Matrix = glm::mat4(1.0f);
    
    // 행성들의 궤도 경로 설정
    glm::mat4 transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.0f, 0.0f));
    glm::mat4 s_transmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, 0.0f, 0.0f));
    
    // 궤도 기울기 설정
    smat[0] = glm::mat4(1.0f);                                                    // xz 평면
    smat[1] = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));  // 반시계 45도
    smat[2] = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 시계 45도
    
    for (int i = 0; i < 3; i++) {
        Matrix[i] = transmat;
        s_Matrix[i] = s_transmat;
    }
}

void scaling(float s) {
    scalemat = glm::scale(glm::mat4(1.0f), glm::vec3(s, s, s));
}

void startOriginPassAnimation() {
    if (isGlobalAnimating) return;
    isGlobalAnimating = true;
    currentAnimationType = 1;
    std::cout << "원점 통과 애니메이션 시작!" << std::endl;
}

void startUpDownAnimation() {
    if (isGlobalAnimating) return;
    isGlobalAnimating = true;
    currentAnimationType = 2;
    std::cout << "위/아래 이동 애니메이션 시작!" << std::endl;
}

void updateAnimations() {
    // 기존 애니메이션 업데이트 코드 유지
}

void make_vertexShaders() {
    GLchar* vertexSource = filetobuf("vertex_3d.glsl");
    if (!vertexSource) {
        std::cerr << "ERROR: vertex shader 파일을 읽을 수 없습니다." << std::endl;
        return;
    }
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(vertexSource);
}

void make_fragmentShaders() {
    GLchar* fragmentSource = filetobuf("fragment_3d.glsl");
    if (!fragmentSource) {
        std::cerr << "ERROR: fragment shader 파일을 읽을 수 없습니다." << std::endl;
        return;
    }
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
    }
    free(fragmentSource);
}

void make_shaderProgram() {
    make_vertexShaders();
    make_fragmentShaders();
    
    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
        return;
    }
    
    glUseProgram(shaderProgramID);
    std::cout << "셰이더 프로그램 초기화 완료" << std::endl;
}

GLvoid drawScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // 카메라 거리를 이용한 뷰 매트릭스 설정
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -cameraDistance));
    
    if (angle) {
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 10.0f);
    } else {
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

    glm::mat4 baseRotation = glm::mat4(1.0f);
    baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "Matrix");

    // 축 그리기
    glm::mat4 axisMatrix = projection * view * baseRotation;
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisMatrix));
    glBindVertexArray(axis.VAO);
    glDrawElements(GL_LINES, axis.index.size(), GL_UNSIGNED_INT, 0);
    
    // 궤도 그리기
    for (int i = 0; i < 3; i++) {
        glm::mat4 orbitMatrix = projection * view * baseRotation * movemat * zmat * scalemat * smat[i];
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(orbitMatrix));
        glBindVertexArray(orbits[i].VAO);
        glDrawElements(GL_LINE_LOOP, orbits[i].index.size(), GL_UNSIGNED_INT, 0);

        glm::mat4 s_orbitMatrix = projection * view * baseRotation * movemat * zmat * scalemat * smat[i] * Matrix[i];
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(s_orbitMatrix));
        glBindVertexArray(s_orbits[i].VAO);
        glDrawElements(GL_LINE_LOOP, s_orbits[i].index.size(), GL_UNSIGNED_INT, 0);
    }

    // GLU 객체들 렌더링
    glUseProgram(0);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (angle) {
        glOrtho(-2.0, 2.0, -2.0, 2.0, -10.0, 10.0);
    } else {
        gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // 카메라 거리를 GLU 객체에도 적용
    glTranslatef(0.0f, 0.0f, -cameraDistance);
    glMultMatrixf(glm::value_ptr(baseRotation));
    
    // 스타일 설정
    if (solid) {
        if (centerSphere.obj) gluQuadricDrawStyle(centerSphere.obj, GLU_FILL);
        for (int i = 0; i < 3; i++) {
            if (planetSpheres[i].obj) gluQuadricDrawStyle(planetSpheres[i].obj, GLU_FILL);
            if (moonSpheres[i].obj) gluQuadricDrawStyle(moonSpheres[i].obj, GLU_FILL);
        }
    } else {
        if (centerSphere.obj) gluQuadricDrawStyle(centerSphere.obj, GLU_LINE);
        for (int i = 0; i < 3; i++) {
            if (planetSpheres[i].obj) gluQuadricDrawStyle(planetSpheres[i].obj, GLU_LINE);
            if (moonSpheres[i].obj) gluQuadricDrawStyle(moonSpheres[i].obj, GLU_LINE);
        }
    }
    
    // 중심 구 렌더링 - 빨간색
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(movemat * big_Matrix));
    glColor3f(1.0f, 0.0f, 0.0f);
    if (centerSphere.obj) gluSphere(centerSphere.obj, centerSphere.size, 20, 20);
    glPopMatrix();

    // 행성들 렌더링 - 각각 다른 색상
    for (int i = 0; i < 3; i++) {
        glColor3f(planetColors[i].r, planetColors[i].g, planetColors[i].b);
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(movemat * zmat * scalemat * smat[i] * Matrix[i]));
        if (planetSpheres[i].obj) gluSphere(planetSpheres[i].obj, planetSpheres[i].size, 15, 15);
        glPopMatrix();

        // 달들 렌더링 - 각각 다른 색상
        glColor3f(moonColors[i].r, moonColors[i].g, moonColors[i].b);
        glPushMatrix();
        glMultMatrixf(glm::value_ptr(movemat * zmat * scalemat * smat[i] * Matrix[i] * s_Matrix[i]));
        if (moonSpheres[i].obj) gluSphere(moonSpheres[i].obj, moonSpheres[i].size, 10, 10);
        glPopMatrix();
    }
    
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

GLvoid Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'p':
    case 'P':
        angle = !angle;
        std::cout << "투영 모드: " << (angle ? "직각투영" : "원근투영") << std::endl;
        break;
    case 'm':
    case 'M':
        solid = !solid;
        std::cout << "렌더링 모드: " << (solid ? "솔리드" : "와이어프레임") << std::endl;
        break;
    case 'w':
    case 'W':
        movemat = glm::translate(movemat, glm::vec3(0.0f, 0.1f, 0.0f));
        break;
    case 'a':
    case 'A':
        movemat = glm::translate(movemat, glm::vec3(-0.1f, 0.0f, 0.0f));
        break;
    case 's':
    case 'S':
        movemat = glm::translate(movemat, glm::vec3(0.0f, -0.1f, 0.0f));
        break;
    case 'd':
    case 'D':
        movemat = glm::translate(movemat, glm::vec3(0.1f, 0.0f, 0.0f));
        break;
    case '+':
    case '=':
        // 카메라 가까이 (줌 인)
        cameraDistance -= CAMERA_MOVE_SPEED;
        if (cameraDistance < MIN_CAMERA_DISTANCE) {
            cameraDistance = MIN_CAMERA_DISTANCE;
        }
        std::cout << "카메라 가까이: " << cameraDistance << std::endl;
        break;
    case '-':
    case '_':
        // 카메라 멀리 (줌 아웃)
        cameraDistance += CAMERA_MOVE_SPEED;
        if (cameraDistance > MAX_CAMERA_DISTANCE) {
            cameraDistance = MAX_CAMERA_DISTANCE;
        }
        std::cout << "카메라 멀리: " << cameraDistance << std::endl;
        break;
    case 'y':
        scale += 0.1f;
        scaling(scale);
        std::cout << "궤도 확대: " << scale << std::endl;
        break;
    case 'Y':
        scale -= 0.1f;
        if (scale < 0.1f) scale = 0.1f;
        scaling(scale);
        std::cout << "궤도 축소: " << scale << std::endl;
        break;
    case 'z':
        z_rotate = true;
        zangle = 2.0f;
        std::cout << "z축 양방향 회전" << std::endl;
        break;
    case 'Z':
        z_rotate = true;
        zangle = -2.0f;
        std::cout << "z축 음방향 회전" << std::endl;
        break;
    case 'q':
    case 'Q':
        std::cout << "프로그램 종료" << std::endl;
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// 특수 키 처리 (Page Up/Down을 +/- 대신 사용 가능)
GLvoid SpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_PAGE_UP: // + 키 대신
        cameraDistance -= CAMERA_MOVE_SPEED;
        if (cameraDistance < MIN_CAMERA_DISTANCE) {
            cameraDistance = MIN_CAMERA_DISTANCE;
        }
        std::cout << "카메라 가까이 (Page Up): " << cameraDistance << std::endl;
        break;
    case GLUT_KEY_PAGE_DOWN: // - 키 대신
        cameraDistance += CAMERA_MOVE_SPEED;
        if (cameraDistance > MAX_CAMERA_DISTANCE) {
            cameraDistance = MAX_CAMERA_DISTANCE;
        }
        std::cout << "카메라 멀리 (Page Down): " << cameraDistance << std::endl;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value) {
    updateAnimations();
    
    // 행성들의 공전 (다른 속도)
    if (!isGlobalAnimating) {
        glm::mat4 rotation1 = glm::rotate(glm::mat4(1.0f), glm::radians(1.2f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(0.8f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotation3 = glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        Matrix[0] = rotation1 * Matrix[0];
        Matrix[1] = rotation2 * Matrix[1];
        Matrix[2] = rotation3 * Matrix[2];

        // 달들의 공전 (더 빠른 속도)
        glm::mat4 moonRotation1 = glm::rotate(glm::mat4(1.0f), glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 moonRotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 moonRotation3 = glm::rotate(glm::mat4(1.0f), glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        s_Matrix[0] = moonRotation1 * s_Matrix[0];
        s_Matrix[1] = moonRotation2 * s_Matrix[1];
        s_Matrix[2] = moonRotation3 * s_Matrix[2];
    }
    
    if (z_rotate) {
        zmat = glm::rotate(glm::mat4(1.0f), glm::radians(zangle), glm::vec3(0.0f, 0.0f, 1.0f)) * zmat;
    }
    
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}