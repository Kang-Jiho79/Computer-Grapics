#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <vector>
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
    glm::vec3 position;
    GLuint VAO, VBO[2], EBO;
};

Shape robot[7]; // 0 머리, 1 몸통, 2 왼팔, 3 오른팔, 4 왼다리, 5 오른다리, 6 코
Shape box;
Shape obstacles[3];

// 애니메이션 및 움직임 관련 변수들
glm::mat4 lanimation = glm::mat4(1.0f);
glm::mat4 ranimation = glm::mat4(1.0f);
glm::mat4 movement = glm::mat4(1.0f);
glm::mat4 front = glm::mat4(1.0f);
float robot_speed = 0.05f;
float wall_size = 3.0f;
bool movemotion = false;
bool open = false;
float open_angle = 0.0f;
bool jumping = false;
bool rotate = true;

// 카메라
struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
} camera = { glm::vec3(0.0f, 0.0f, 8.0f),
             glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3(0.0f, 1.0f, 0.0f) };

char* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr) {
        std::cerr << "파일을 열 수 없습니다: " << file << std::endl;
        return NULL;
    }
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

//--- 함수 선언
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyboardUp(unsigned char key, int x, int y);
void TimerFunction(int value);
void InitBuffers(Shape& shape);
void CreateCube(Shape& cube, float x, float y, float z);

float getRandomcolor()
{
    std::uniform_real_distribution<float> dis(0.2f, 0.8f);
    return dis(gen);
}

// 충돌 검사 함수들
bool AABB(glm::vec3 pos1, float size1, glm::vec3 pos2, float size2) {
    return (pos1.x - size1 <= pos2.x + size2 && pos1.x + size1 >= pos2.x - size2 &&
        pos1.z - size1 <= pos2.z + size2 && pos1.z + size1 >= pos2.z - size2);
}

bool wall_collision(glm::vec3 pos1, float size1, float size2) {
    return (pos1.x - size1 <= -size2 || pos1.x + size1 >= size2 ||
        pos1.z - size1 <= -size2 || pos1.z + size1 >= size2);
}

// 게임 관련 함수들
void location();
void robot_movement();
void robot_fall();
void robot_jump();
void menu();
bool robot_collision();
void robot_turn(float angle);
void CubeFrontOpen();

//--- 필요한 변수 선언
GLint width = 800, height = 600;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

//--- 메인 함수
void main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutCreateWindow("Example22");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW 초기화 실패!" << std::endl;
        exit(1);
    }

    make_vertexShaders();
    make_fragmentShaders();
    shaderProgramID = make_shaderProgram();

    menu();

    // 상자 생성
    CreateCube(box, wall_size, wall_size, wall_size);

    // 장애물 생성
    for (int i = 0; i < 3; i++) {
        CreateCube(obstacles[i], 0.5f, 0.5f, 0.5f);
    }

    // 로봇 생성
    CreateCube(robot[0], 0.2f, 0.2f, 0.1f); // 머리
    CreateCube(robot[1], 0.2f, 0.4f, 0.1f); // 몸통
    CreateCube(robot[2], 0.05f, 0.3f, 0.05f); // 왼팔
    CreateCube(robot[3], 0.05f, 0.3f, 0.05f); // 오른팔
    CreateCube(robot[4], 0.05f, 0.3f, 0.05f); // 왼다리
    CreateCube(robot[5], 0.05f, 0.3f, 0.05f); // 오른다리
    CreateCube(robot[6], 0.03f, 0.05f, 0.03f); // 코

    location();

    glutTimerFunc(50, TimerFunction, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutKeyboardUpFunc(KeyboardUp);
    glutMainLoop();
}

void menu() {
    std::cout << "========== 조작법 ==========" << std::endl;
    std::cout << "o: 앞면이 열렸다 닫혔다" << std::endl;
    std::cout << "w/a/s/d: 로봇 이동" << std::endl;
    std::cout << "+/-: 로봇 속도 조절" << std::endl;
    std::cout << "j: 로봇 점프" << std::endl;
    std::cout << "i: 초기화" << std::endl;
    std::cout << "z/Z: 카메라 z축 이동" << std::endl;
    std::cout << "x/X: 카메라 x축 이동" << std::endl;
    std::cout << "y/Y: 카메라 y축 공전" << std::endl;
    std::cout << "q: 종료" << std::endl;
    std::cout << "============================" << std::endl;
}

void CreateCube(Shape& cube, float x, float y, float z) {
    cube.vertices = {
        // 앞면
       -x, y, z,  -x, -y, z,  x, -y, z,  x, y, z,
       // 뒷면
       -x, -y, -z, -x, y, -z, x, y, -z,  x, -y, -z,
       // 윗면
       -x, y, -z,  -x, y, z,  x, y, z,   x, y, -z,
       // 아래면 
       -x, -y, z,  -x, -y, -z, x, -y, -z, x, -y, z,
       // 왼면 
       -x, y, -z,  -x, -y, -z, -x, -y, z, -x, y, z,
       // 오른면
       x, y, z,    x, -y, z,   x, -y, -z,  x, y, -z
    };

    cube.index = {
        // 앞면
        0, 1, 2, 0, 2, 3,
        // 뒷면
        4, 5, 6, 4, 6, 7,
        // 윗면
        8, 9, 10, 8, 10, 11,
        // 아래면
        12, 13, 14, 12, 14, 15,
        // 왼면
        16, 17, 18, 16, 18, 19,
        // 오른면
        20, 21, 22, 20, 22, 23
    };

    // 색상 설정 (회색 계열)
    cube.colors.clear();
    for (int face = 0; face < 6; face++) {
        for (int vertex = 0; vertex < 4; vertex++) {
            cube.colors.push_back(getRandomcolor());
            cube.colors.push_back(getRandomcolor());
            cube.colors.push_back(getRandomcolor());
        }
    }

    InitBuffers(cube);
}

void CubeFrontOpen() {
    const float rotation_speed = 2.0f;
    const float max_angle = 90.0f;

    if (open_angle < max_angle) {
        glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -wall_size, -wall_size));
        glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(-rotation_speed), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, wall_size, wall_size));
        front = translateBack * rotMat * translateToOrigin * front;
        open_angle += rotation_speed;
    }
    else {
        open = false;
        open_angle = max_angle;
    }
}

void location() {
    // 로봇 파츠 위치 설정
    robot[0].position = glm::vec3(0.0f, 1.35f, 0.0f);
    robot[1].position = glm::vec3(0.0f, 0.85f, 0.0f);
    robot[2].position = glm::vec3(-0.25f, 0.95f, 0.0f);
    robot[3].position = glm::vec3(0.25f, 0.95f, 0.0f);
    robot[4].position = glm::vec3(-0.1f, 0.3f, 0.0f);
    robot[5].position = glm::vec3(0.1f, 0.3f, 0.0f);
    robot[6].position = glm::vec3(0.0f, 1.3f, 0.1f);

    // 로봇 색상 설정
    const glm::vec3 colors[7] = {
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor()),
        glm::vec3(getRandomcolor(), getRandomcolor(), getRandomcolor())
    };

    for (int i = 0; i < 7; i++) {
        robot[i].colors.clear();
        size_t vertexCount = robot[i].vertices.size() / 3;
        for (size_t j = 0; j < vertexCount; j++) {
            robot[i].colors.push_back(colors[i].r);
            robot[i].colors.push_back(colors[i].g);
            robot[i].colors.push_back(colors[i].b);
        }
        InitBuffers(robot[i]);
    }

    // 장애물 위치 설정
    obstacles[0].position = glm::vec3(-1.0f, -2.5f, 1.0f);
    obstacles[1].position = glm::vec3(0.0f, -2.5f, 2.0f);
    obstacles[2].position = glm::vec3(1.0f, -2.5f, -1.5f);
}

float leftArmAngle = 0.0f;
float rightArmAngle = 0.0f; 
float leftLegAngle = 0.0f;
float rightLegAngle = 0.0f;
bool armIncreasing = true;

void robot_movement() {
    const float max_angle = 20.0f;
    const float angle_increment = 2.0f;

    if (armIncreasing) {
        leftArmAngle += angle_increment;
        rightArmAngle -= angle_increment;
        leftLegAngle -= angle_increment;  // 팔과 반대
        rightLegAngle += angle_increment; // 팔과 반대
        
        if (leftArmAngle >= max_angle) {
            armIncreasing = false;
        }
    } else {
        leftArmAngle -= angle_increment;
        rightArmAngle += angle_increment;
        leftLegAngle += angle_increment;  // 팔과 반대
        rightLegAngle -= angle_increment; // 팔과 반대
        
        if (leftArmAngle <= -max_angle) {
            armIncreasing = true;
        }
    }

    // 어깨 기준 회전 행렬 계산 (팔의 길이 0.3f의 절반인 0.15f를 위쪽 기준점으로 사용)
    glm::mat4 shoulderPivot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
    glm::mat4 shoulderPivotBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f));
    
    // 골반 기준 회전 행렬 계산 (다리의 길이 0.3f의 절반인 0.15f를 위쪽 기준점으로 사용)
    glm::mat4 hipPivot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
    glm::mat4 hipPivotBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f));
    
    // 팔 애니메이션 행렬
    lanimation = shoulderPivotBack * glm::rotate(glm::mat4(1.0f), glm::radians(leftArmAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * shoulderPivot;  // 왼팔
    ranimation = shoulderPivotBack * glm::rotate(glm::mat4(1.0f), glm::radians(rightArmAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * shoulderPivot; // 오른팔
}

void robot_jump() {
    static float jump_height = 0.0f;
    const float jump_speed = 0.1f;
    const float max_jump_height = 2.0f;

    if (jumping) {
        jump_height += jump_speed;
        for (int i = 0; i < 7; i++) {
            robot[i].position.y += jump_speed;
        }

        if (jump_height >= max_jump_height) {
            jump_height = 0.0f;
            jumping = false;
        }
    }
}

void robot_fall() {
    if (jumping) return;

    glm::vec3 robot_pos = glm::vec3(movement[3][0], movement[3][1], movement[3][2]);
    float robot_bottom_y = robot[4].position.y - 0.3f;
    const float fall_speed = 0.1f;
    const float ground_level = -wall_size;

    bool on_obstacle = false;
    for (int i = 0; i < 3; i++) {
        if (AABB(robot_pos, 0.2f, obstacles[i].position, 0.5f)) {
            float obstacle_top = obstacles[i].position.y + 0.5f;
            if (robot_bottom_y <= obstacle_top + 0.1f && robot_bottom_y >= obstacle_top - 0.1f) {
                on_obstacle = true;
                break;
            }
        }
    }

    if (!on_obstacle && robot_bottom_y > ground_level) {
        for (int i = 0; i < 7; i++) {
            robot[i].position.y -= fall_speed;
        }
    }
}

bool robot_collision() {
    glm::vec3 current_pos = glm::vec3(movement[3][0], movement[3][1], movement[3][2]);
    glm::vec3 movement_dir = glm::mat3(movement) * glm::vec3(0.0f, 0.0f, robot_speed);
    glm::vec3 next_pos = current_pos + movement_dir;

    if (wall_collision(next_pos, 0.2f, wall_size)) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        float robot_bottom = robot[4].position.y - 0.3f;
        float obstacle_top = obstacles[i].position.y + 0.5f;
        bool is_above_obstacle = robot_bottom >= obstacle_top;

        if (!is_above_obstacle && AABB(next_pos, 0.2f, obstacles[i].position, 0.5f)) {
            return false;
        }
    }

    return true;
}

void robot_turn(float turn_angle) {
    glm::vec3 robot_pos = glm::vec3(movement[3][0], movement[3][1], movement[3][2]);
    movement = glm::translate(glm::mat4(1.0f), robot_pos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(turn_angle), glm::vec3(0.0f, 1.0f, 0.0f));
}

GLvoid InitBuffers(Shape& shape) {
    glGenVertexArrays(1, &shape.VAO);
    glBindVertexArray(shape.VAO);

    glGenBuffers(2, shape.VBO);

    // 버텍스 데이터
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 데이터
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, shape.colors.size() * sizeof(float), shape.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // 인덱스 데이터
    glGenBuffers(1, &shape.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape.index.size() * sizeof(unsigned int), shape.index.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void make_vertexShaders() {
    GLchar* vertexSource = filetobuf("vertex_3d.glsl");
    if (!vertexSource) {
        std::cerr << "버텍스 셰이더 파일을 읽을 수 없습니다." << std::endl;
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
        std::cerr << "프래그먼트 셰이더 파일을 읽을 수 없습니다." << std::endl;
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

GLuint make_shaderProgram() {
    GLuint shaderID = glCreateProgram();

    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
        std::cerr << "ERROR: shader program 링크 실패\n" << errorLog << std::endl;
        exit(1);
    }

    glUseProgram(shaderID);
    return shaderID;
}

GLvoid drawScene() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProgramID);

    // 변환 행렬 계산
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(camera.eye, camera.at, camera.up);

    // Matrix 유니폼 위치 가져오기
    unsigned int matrixLocation = glGetUniformLocation(shaderProgramID, "Matrix");

    // 메인 상자 그리기
    glBindVertexArray(box.VAO);
    if (open && open_angle < 90.0f) {
        for (int i = 0; i < 6; i++) {
            glm::mat4 model = (i == 0) ? front : glm::mat4(1.0f);
            glm::mat4 mvp = projection * view * model;
            glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
        }
    }
    else {
        if (open_angle >= 90.0f) {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CW);
        }
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, box.index.size(), GL_UNSIGNED_INT, 0);
        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
    }

    // 로봇 그리기
    for (int i = 0; i < 7; i++) {
        glm::mat4 model = movement * glm::translate(glm::mat4(1.0f), robot[i].position);

        if (i == 2) {          // 왼팔
            model = model * lanimation;
        }
        else if (i == 3) {     // 오른팔  
            model = model * ranimation;
        }
        else if (i == 4) {     // 왼다리
            glm::mat4 hipPivot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
            glm::mat4 hipPivotBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f));
            glm::mat4 leftLegTransform = hipPivotBack * glm::rotate(glm::mat4(1.0f), glm::radians(leftLegAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * hipPivot;
            model = model * leftLegTransform;
        }
        else if (i == 5) {     // 오른다리
            glm::mat4 hipPivot = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, 0.0f));
            glm::mat4 hipPivotBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.3f, 0.0f));
            glm::mat4 rightLegTransform = hipPivotBack * glm::rotate(glm::mat4(1.0f), glm::radians(rightLegAngle), glm::vec3(1.0f, 0.0f, 0.0f)) * hipPivot;
            model = model * rightLegTransform;
        }

        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(robot[i].VAO);
        glDrawElements(GL_TRIANGLES, robot[i].index.size(), GL_UNSIGNED_INT, 0);
    }

    // 장애물 그리기
    for (int i = 0; i < 3; i++) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), obstacles[i].position);
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(obstacles[i].VAO);
        glDrawElements(GL_TRIANGLES, obstacles[i].index.size(), GL_UNSIGNED_INT, 0);
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
    case 'o':
        open = true;
        break;
    case 'z':
        camera.eye.z -= 0.1f;
        break;
    case 'Z':
        camera.eye.z += 0.1f;
        break;
    case 'x':
        camera.eye.x += 0.1f;
        break;
    case 'X':
        camera.eye.x -= 0.1f;
        break;
    case 'y':
    case 'Y': {
        float rotation_angle = (key == 'y') ? 5.0f : -5.0f;
        float angle = glm::radians(rotation_angle);
        float cos_angle = cos(angle);
        float sin_angle = sin(angle);
        float new_eye_x = camera.eye.x * cos_angle - camera.eye.z * sin_angle;
        float new_eye_z = camera.eye.x * sin_angle + camera.eye.z * cos_angle;
        camera.eye.x = new_eye_x;
        camera.eye.z = new_eye_z;
        break;
    }
    case 'w':
        if (rotate) {
            robot_turn(180.0f);
            rotate = false;
        }
        if (robot_collision()) {
            movement = glm::translate(movement, glm::vec3(0.0f, 0.0f, robot_speed));
            movemotion = true;
        }
        break;
    case 's':
        if (rotate) {
            robot_turn(0.0f);
            rotate = false;
        }
        if (robot_collision()) {
            movement = glm::translate(movement, glm::vec3(0.0f, 0.0f, robot_speed));
            movemotion = true;
        }
        break;
    case 'a':
        if (rotate) {
            robot_turn(-90.0f);
            rotate = false;
        }
        if (robot_collision()) {
            movement = glm::translate(movement, glm::vec3(0.0f, 0.0f, robot_speed));
            movemotion = true;
        }
        break;
    case 'd':
        if (rotate) {
            robot_turn(90.0f);
            rotate = false;
        }
        if (robot_collision()) {
            movement = glm::translate(movement, glm::vec3(0.0f, 0.0f, robot_speed));
            movemotion = true;
        }
        break;
    case '+':
        robot_speed = std::min(robot_speed + 0.01f, 0.2f);
        std::cout << "Robot speed: " << robot_speed << std::endl;
        break;
    case '-':
        robot_speed = std::max(robot_speed - 0.01f, 0.01f);
        std::cout << "Robot speed: " << robot_speed << std::endl;
        break;
    case 'j':
        if (!jumping) jumping = true;
        break;
    case 'i':
        // 초기화
        location();
        movement = glm::mat4(1.0f);
        lanimation = glm::mat4(1.0f);
        ranimation = glm::mat4(1.0f);
        front = glm::mat4(1.0f);
        movemotion = false;
        open = false;
        open_angle = 0.0f;
        jumping = false;
        rotate = true;
        
        // 팔다리 각도 초기화
        leftArmAngle = 0.0f;
        rightArmAngle = 0.0f;
        leftLegAngle = 0.0f;
        rightLegAngle = 0.0f;
        armIncreasing = true;
        
        camera.eye = glm::vec3(0.0f, 0.0f, 10.0f);
        camera.at = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case 'q':
        exit(0);
        break;
    }
    glutPostRedisplay();
}

GLvoid KeyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'a':
    case 's':
    case 'd':
        rotate = true;
        movemotion = false;
        // 각도 변수들은 초기화하지 않음 - 애니메이션을 유지하기 위해
        // lanimation과 ranimation도 초기화하지 않음
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value) {
    if (movemotion) robot_movement();
    if (open) CubeFrontOpen();
    if (!jumping) robot_fall();
    else robot_jump();

    glutPostRedisplay();
    glutTimerFunc(50, TimerFunction, 1);
}