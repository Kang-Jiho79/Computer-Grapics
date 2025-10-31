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
	int type = 0; // 0: sphere, 1: cylinder, 2: cone
	int x_rotate = 0, y_rotate = 0, revolution = 0;
	int origin_scale = 0, self_scale = 0;
	float translation[3] = { 0.0f };
	float x_rotationAngle = { 0.0f };
	float y_rotationAngle = { 0.0f };
	float revolutionAngle = { 0.0f };
	float origin_scale_value[3]{ 1.0f,1.0f,1.0f };
	float self_scale_value[3]{ 1.0f, 1.0f, 1.0f };

	// 애니메이션 관련 변수 추가
	bool isAnimating = false;
	float animProgress = 0.0f;
	float startPos[3] = { 0.0f };
	float targetPos[3] = { 0.0f };
	float animSpeed = 0.02f;
	int animType = 0; // 0: 없음, 1: 원점 통과, 2: 위/아래 이동
	
    // baseRotation도 고려한 버전 (더 정확함)
    void getTransformedPosition(float outPos[3]) const {
		// drawScene과 완전히 동일한 변환 순서 적용
		glm::mat4 baseRotation = glm::mat4(1.0f);
		baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		
		glm::mat4 modelMatrix = baseRotation;  // baseRotation 포함!
        
		// 1. 원점 기준 확대/축소
		modelMatrix = glm::scale(modelMatrix, glm::vec3(
			origin_scale_value[0], origin_scale_value[1], origin_scale_value[2]
		));

		// 2. 공전
		modelMatrix = glm::rotate(modelMatrix, glm::radians(revolutionAngle),
			glm::vec3(0.0f, 1.0f, 0.0f));

		// 3. 이동
		modelMatrix = glm::translate(modelMatrix, glm::vec3(
			translation[0], translation[1], translation[2]
		));

		// 원점을 변환하여 실제 위치 얻기
		glm::vec4 transformedPos = modelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		outPos[0] = transformedPos.x;
		outPos[1] = transformedPos.y;
		outPos[2] = transformedPos.z;
		printf("getTransformedPosition (with baseRotation): (%.2f, %.2f, %.2f)\n", 
               outPos[0], outPos[1], outPos[2]);
    }
	~Shape() {
		if (obj) {
			gluDeleteQuadric(obj);
		}
	}
};

Shape axis;
Shape shape[2];

int selected_shape = 0; // 0, 1, 2: all

bool isGlobalAnimating = false;
int currentAnimationType = 0; // 1: t 애니메이션, 2: u 애니메이션

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

void createSphere(Shape& shape, float radius = 0.5f) {
    shape.obj = gluNewQuadric();
    gluQuadricDrawStyle(shape.obj, GLU_LINE);
    gluQuadricNormals(shape.obj, GLU_SMOOTH);
    gluQuadricTexture(shape.obj, GL_FALSE);
    shape.size = radius;
	shape.type = 0;
}

void createCylinder(Shape& shape, float baseRadius = 0.3f, float topRadius = 0.3f, float height = 1.0f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size = height;
	shape.type = 1;
}

void createCone(Shape& shape, float baseRadius = 0.5f, float height = 0.5f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size = height;
	shape.type = 2;
}

void createsquare(Shape& shape, float radius = 0.5f) {
	shape.obj = gluNewQuadric();
	gluQuadricDrawStyle(shape.obj, GLU_LINE);
	gluQuadricNormals(shape.obj, GLU_SMOOTH);
	gluQuadricTexture(shape.obj, GL_FALSE);
	shape.size = radius;
	shape.type = 3;
}

void flagreset() {
	for (int i = 0; i < 2; i++) {
		shape[i].x_rotate = 0;
		shape[i].y_rotate = 0;
		shape[i].revolution = 0;
		shape[i].origin_scale = 0;
		shape[i].self_scale = 0;
	}
}

void reset() {
    // 애니메이션 상태 먼저 초기화
    isGlobalAnimating = false;
    currentAnimationType = 0;
    
    createSphere(shape[0], 0.3f);
    shape[0].translation[0] = -0.5f;

    createCylinder(shape[1], 0.5f, 0.5f, 0.5f);
    shape[1].translation[0] = 0.5f;
    
    for (int i = 0; i < 2; i++) {
        shape[i].x_rotate = 0;
        shape[i].y_rotate = 0;
        shape[i].revolution = 0;
        shape[i].origin_scale = 0;
        shape[i].self_scale = 0;
        shape[i].translation[0] = (i == 0) ? -0.5f : 0.5f;
        shape[i].translation[1] = 0.0f;
        shape[i].translation[2] = 0.0f;
        shape[i].x_rotationAngle = 0.0f;
        shape[i].y_rotationAngle = 0.0f;
        shape[i].revolutionAngle = 0.0f;
        shape[i].origin_scale_value[0] = 1.0f;
        shape[i].origin_scale_value[1] = 1.0f;
        shape[i].origin_scale_value[2] = 1.0f;
        shape[i].self_scale_value[0] = 1.0f;
        shape[i].self_scale_value[1] = 1.0f;
        shape[i].self_scale_value[2] = 1.0f;
        
        // 애니메이션 관련 변수 초기화
        shape[i].isAnimating = false;
        shape[i].animProgress = 0.0f;
        shape[i].animType = 0;
    }
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

void startOriginPassAnimation() {
    if (isGlobalAnimating) return;

    isGlobalAnimating = true;
    currentAnimationType = 1;

    // baseRotation 생성 (drawScene과 동일)
    glm::mat4 baseRotation = glm::mat4(1.0f);
    baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    for (int i = 0; i < 2; i++) {
        shape[i].isAnimating = true;
        shape[i].animProgress = 0.0f;
        shape[i].animType = 1;

        // baseRotation을 포함한 정확한 위치 계산
        shape[i].getTransformedPosition(shape[i].startPos);
    }
    
    for (int i = 0; i < 2; i++) {
        int targetIndex = (i == 0) ? 1 : 0;
        shape[i].targetPos[0] = shape[targetIndex].startPos[0];
        shape[i].targetPos[1] = shape[targetIndex].startPos[1];
        shape[i].targetPos[2] = shape[targetIndex].startPos[2];
        printf("Shape %d: start(%.2f, %.2f, %.2f) -> target(%.2f, %.2f, %.2f)\n", i,
            shape[i].startPos[0], shape[i].startPos[1], shape[i].startPos[2],
            shape[i].targetPos[0], shape[i].targetPos[1], shape[i].targetPos[2]);
    }

    std::cout << "원점 통과 애니메이션 시작!" << std::endl;
}

void startUpDownAnimation() {
    if (isGlobalAnimating) return;

    isGlobalAnimating = true;
    currentAnimationType = 2;

    for (int i = 0; i < 2; i++) {
        shape[i].isAnimating = true;
        shape[i].animProgress = 0.0f;
        shape[i].animType = 2;

        // 현재 위치를 정확히 계산
        shape[i].getTransformedPosition(shape[i].startPos);
        
        printf("Shape %d start position: (%.2f, %.2f, %.2f)\n", i,
            shape[i].startPos[0], shape[i].startPos[1], shape[i].startPos[2]);
    }
    
    // 목표 위치 설정 (상대방의 위치)
    for (int i = 0; i < 2; i++) {
        int targetIndex = (i == 0) ? 1 : 0;
        shape[i].targetPos[0] = shape[targetIndex].startPos[0];
        shape[i].targetPos[1] = shape[targetIndex].startPos[1];
        shape[i].targetPos[2] = shape[targetIndex].startPos[2];
        
        printf("Shape %d target position: (%.2f, %.2f, %.2f)\n", i,
            shape[i].targetPos[0], shape[i].targetPos[1], shape[i].targetPos[2]);
    }

    std::cout << "위/아래 이동 애니메이션 시작!" << std::endl;
}

void updateAnimations() {
    if (!isGlobalAnimating) return;

    bool allAnimationComplete = true;

    for (int i = 0; i < 2; i++) {
        if (shape[i].isAnimating) {
            shape[i].animProgress += shape[i].animSpeed;

            if (shape[i].animProgress >= 1.0f) {
                // 애니메이션 완료 처리 (기존과 동일)
                shape[i].animProgress = 1.0f;
                shape[i].isAnimating = false;
                
                shape[i].revolutionAngle = 0.0f;
                shape[i].origin_scale_value[0] = shape[i].origin_scale_value[1] = shape[i].origin_scale_value[2] = 1.0f;
                
                glm::mat4 baseRotation = glm::mat4(1.0f);
                baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                
                glm::mat4 invBaseRotation = glm::inverse(baseRotation);
                glm::vec4 finalPos = invBaseRotation * glm::vec4(shape[i].targetPos[0], shape[i].targetPos[1], shape[i].targetPos[2], 1.0f);
                
                shape[i].translation[0] = finalPos.x;
                shape[i].translation[1] = finalPos.y;
                shape[i].translation[2] = finalPos.z;
                
                printf("Animation complete for shape %d: final translation (%.2f, %.2f, %.2f)\n", 
                    i, shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]);
            }
            else {
                allAnimationComplete = false;

                // 애니메이션 중 다른 변환 무력화
                shape[i].revolutionAngle = 0.0f;
                shape[i].origin_scale_value[0] = shape[i].origin_scale_value[1] = shape[i].origin_scale_value[2] = 1.0f;
                shape[i].self_scale_value[0] = shape[i].self_scale_value[1] = shape[i].self_scale_value[2] = 1.0f;
                shape[i].x_rotationAngle = shape[i].y_rotationAngle = 0.0f;

                glm::mat4 baseRotation = glm::mat4(1.0f);
                baseRotation = glm::rotate(baseRotation, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                baseRotation = glm::rotate(baseRotation, glm::radians(50.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                glm::mat4 invBaseRotation = glm::inverse(baseRotation);

                if (currentAnimationType == 1) {
                    // t 애니메이션 (기존과 동일)
                    float t = shape[i].animProgress;
                    glm::vec3 currentWorldPos;
                    
                    if (t <= 0.5f) {
                        // 첫 번째 절반: 시작점에서 원점으로
                        float localT = t * 2.0f;
                        currentWorldPos.x = shape[i].startPos[0] * (1.0f - localT);
                        currentWorldPos.y = shape[i].startPos[1] * (1.0f - localT);
                        currentWorldPos.z = shape[i].startPos[2] * (1.0f - localT);
                    }
                    else {
                        // 두 번째 절반: 원점에서 목표점으로
                        float localT = (t - 0.5f) * 2.0f;
                        currentWorldPos.x = shape[i].targetPos[0] * localT;
                        currentWorldPos.y = shape[i].targetPos[1] * localT;
                        currentWorldPos.z = shape[i].targetPos[2] * localT;
                    }
                    
                    // baseRotation 역변환 적용하여 translation 계산
                    glm::vec4 localPos = invBaseRotation * glm::vec4(currentWorldPos.x, currentWorldPos.y, currentWorldPos.z, 1.0f);
                    shape[i].translation[0] = localPos.x;
                    shape[i].translation[1] = localPos.y;
                    shape[i].translation[2] = localPos.z;
                }
                else if (currentAnimationType == 2) {
                    // u 애니메이션: 훨씬 간단한 포물선 경로
                    float t = shape[i].animProgress;
                    
                    // 시작점과 끝점 사이의 선형 보간
                    glm::vec3 linearPos;
                    linearPos.x = shape[i].startPos[0] + (shape[i].targetPos[0] - shape[i].startPos[0]) * t;
                    linearPos.y = shape[i].startPos[1] + (shape[i].targetPos[1] - shape[i].startPos[1]) * t;
                    linearPos.z = shape[i].startPos[2] + (shape[i].targetPos[2] - shape[i].startPos[2]) * t;
                    
                    // 포물선 높이 추가 (sin 함수 사용으로 부드러운 곡선)
                    float heightOffset = sin(t * M_PI) * 1.0f; // 최대 높이 1.0
                    
                    // 각 객체마다 다른 방향으로 이동 (첫 번째는 위, 두 번째는 아래)
                    if (i == 0) {
                        linearPos.y += heightOffset; // 위로
                    } else {
                        linearPos.y -= heightOffset; // 아래로
                    }
                    
                    // baseRotation 역변환 적용
                    glm::vec4 localPos = invBaseRotation * glm::vec4(linearPos.x, linearPos.y, linearPos.z, 1.0f);
                    shape[i].translation[0] = localPos.x;
                    shape[i].translation[1] = localPos.y;
                    shape[i].translation[2] = localPos.z;
                    
                    // 디버그 출력 (필요시)
                    if (i == 0 && ((int)(t * 100) % 10 == 0)) { // 10% 간격으로 출력
                        printf("Shape %d: t=%.2f, world(%.2f,%.2f,%.2f), local(%.2f,%.2f,%.2f)\n", 
                            i, t, linearPos.x, linearPos.y, linearPos.z,
                            shape[i].translation[0], shape[i].translation[1], shape[i].translation[2]);
                    }
                }
            }
        }
    }

    if (allAnimationComplete) {
        isGlobalAnimating = false;
        currentAnimationType = 0;
        std::cout << "애니메이션 완료!" << std::endl;
    }
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

	reset(); // 초기 도형 상태 설정

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

	// GLU 객체들 렌더링
	for (int i = 0; i < 2; i++) {
        if (shape[i].obj) {
            glm::mat4 modelMatrix = baseRotation;
            
            // 올바른 변환 순서: 공전 -> 이동 -> 자전 -> 스케일
            
            // 1. 원점 기준 확대/축소 (먼저 적용)
            modelMatrix = glm::scale(modelMatrix, glm::vec3(
                shape[i].origin_scale_value[0], 
                shape[i].origin_scale_value[1], 
                shape[i].origin_scale_value[2]
            ));
            
            // 2. 공전 (원점을 중심으로)
            modelMatrix = glm::rotate(modelMatrix, glm::radians(shape[i].revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            
            // 3. 이동 (공전 후 해당 위치로)
            modelMatrix = glm::translate(modelMatrix, glm::vec3(
                shape[i].translation[0], 
                shape[i].translation[1], 
                shape[i].translation[2]
            ));
            
            // 4. 자전 (객체 자신의 중심을 기준으로)
            modelMatrix = glm::rotate(modelMatrix, glm::radians(shape[i].x_rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(shape[i].y_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            
            // 5. 자체 크기 조절 (마지막에 적용)
            modelMatrix = glm::scale(modelMatrix, glm::vec3(
                shape[i].self_scale_value[0], 
                shape[i].self_scale_value[1], 
                shape[i].self_scale_value[2]
            ));

            // 최종 행렬을 OpenGL에 전달
            glm::mat4 finalMatrix = projection * view * modelMatrix;
            
            // OpenGL 고정 기능 파이프라인으로 전환 (GLU 사용을 위해)
            glUseProgram(0);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(finalMatrix));
            
            // GLU 객체 렌더링
            switch (shape[i].type) {
            case 0: // Sphere
                gluSphere(shape[i].obj, shape[i].size, 20, 20);
                break;
            case 1: // Cylinder
                gluCylinder(shape[i].obj, shape[i].size * 0.6f, shape[i].size * 0.6f, shape[i].size, 20, 20);
                break;
            case 2: // Cone
                gluCylinder(shape[i].obj, shape[i].size, 0.0f, shape[i].size, 20, 20);
                break;
            case 3:    
                gluSphere(shape[i].obj, shape[i].size, 4, 4);
            }
            
            // 셰이더 프로그램 다시 활성화
            glUseProgram(shaderProgramID);
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
	switch (key) {
	case '1':
		selected_shape = 0;
		flagreset();
		break;
	case '2':
		selected_shape = 1;
		flagreset();
		break;
	case '3':
		selected_shape = 2;
		flagreset();
		break;
	case 'x':
		if (selected_shape != 2)
			addint(shape[selected_shape].x_rotate);
		else
			for (int i = 0; i < 2; i++)
				addint(shape[i].x_rotate);
		break;
	case 'X':
		if (selected_shape != 2)
			subint(shape[selected_shape].x_rotate);
		else
			for (int i = 0; i < 2; i++)
				subint(shape[i].x_rotate);
		break;
	case 'y':
		if (selected_shape != 2)
			addint(shape[selected_shape].y_rotate);
		else
			for (int i = 0; i < 2; i++)
				addint(shape[i].y_rotate);
		break;
	case 'Y':
		if (selected_shape != 2)
			subint(shape[selected_shape].y_rotate);
		else
			for (int i = 0; i < 2; i++)
				subint(shape[i].y_rotate);
		break;
	case 'r':
		if (selected_shape != 2)
			addint(shape[selected_shape].revolution);
		else
			for (int i = 0; i < 2; i++)
				addint(shape[i].revolution);
		break;
	case 'R':
		if (selected_shape != 2)
			subint(shape[selected_shape].revolution);
		else
			for (int i = 0; i < 2; i++)
				subint(shape[i].revolution);
		break;
	case 'a':
		if (selected_shape != 2)
			addint(shape[selected_shape].self_scale);
		else
			for (int i = 0; i < 2; i++)
				addint(shape[i].self_scale);
		break;
	case 'A':
		if (selected_shape != 2)
			subint(shape[selected_shape].self_scale);
		else
			for (int i = 0; i < 2; i++)
				subint(shape[i].self_scale);
		break;
	case 'b':
		if (selected_shape != 2)
			addint(shape[selected_shape].origin_scale);
		else
			for (int i = 0; i < 2; i++)
				addint(shape[i].origin_scale);
		break;
	case 'B':
		if (selected_shape != 2)
			subint(shape[selected_shape].origin_scale);
		else
			for (int i = 0; i < 2; i++)
				subint(shape[i].origin_scale);
		break;
	case 'd':
		if (selected_shape != 2)
			shape[selected_shape].translation[0] -= 0.1f;
		else
			for (int i = 0; i < 2; i++)
				shape[i].translation[0] -= 0.1f;
		break;
	case 'D':
		if (selected_shape != 2)
			shape[selected_shape].translation[0] += 0.1f;
		else
			for (int i = 0; i < 2; i++)
				shape[i].translation[0] += 0.1f;
		break;
	case 'e':
		if (selected_shape != 2)
			shape[selected_shape].translation[1] -= 0.1f;
		else
			for (int i = 0; i < 2; i++)
				shape[i].translation[1] -= 0.1f;
		break;
	case 'E':
		if (selected_shape != 2)
			shape[selected_shape].translation[1] += 0.1f;
		else
			for (int i = 0; i < 2; i++)
				shape[i].translation[1] += 0.1f;
		break;
	case 't':
		startOriginPassAnimation();
		break;
	case 'u':
		startUpDownAnimation();
		break;
	case 'v':
		break;
	case 'c':
		for (int i = 0; i < 2; i++) {
			shape[i].obj = nullptr;
			shape[i].type = rand() % 4;
			if (shape[i].type == 0) {
				createSphere(shape[i], 0.3f);
			}
			else if (shape[i].type == 1) {
				createCylinder(shape[i], 0.5f, 0.5f, 0.5f);
				
			}
			else if (shape[i].type == 2) {
				createCone(shape[i], 0.5f, 1.0f);
			}
			else if (shape[i].type == 3) {
				createSphere(shape[i], 0.3f);
			}
		}
		break;
	case 's':
		reset();
		break;
	case 'q': // 프로그램 종료
		break;
		glutPostRedisplay();
	}
}

GLvoid Timer(int value)
{
    // 애니메이션 업데이트 먼저 처리
    updateAnimations();
    
    // 애니메이션 중이 아닐 때만 기본 회전/스케일 처리
    if (!isGlobalAnimating) {
        for (int i = 0; i < 2; i++) {
            shape[i].x_rotationAngle += shape[i].x_rotate * 2.0f;
            if (shape[i].x_rotationAngle >= 360.0f) shape[i].x_rotationAngle -= 360.0f;
            if (shape[i].x_rotationAngle < 0.0f) shape[i].x_rotationAngle += 360.0f;
            
            shape[i].y_rotationAngle += shape[i].y_rotate * 2.0f;
            if (shape[i].y_rotationAngle >= 360.0f) shape[i].y_rotationAngle -= 360.0f;
            if (shape[i].y_rotationAngle < 0.0f) shape[i].y_rotationAngle += 360.0f;
            
            shape[i].revolutionAngle += shape[i].revolution * 2.0f;
            if (shape[i].revolutionAngle >= 360.0f) shape[i].revolutionAngle -= 360.0f;
            if (shape[i].revolutionAngle < 0.0f) shape[i].revolutionAngle += 360.0f;
            
            // 스케일 처리...
            if (shape[i].origin_scale == 1) {
                shape[i].origin_scale_value[0] += 0.01f;
                shape[i].origin_scale_value[1] += 0.01f;
                shape[i].origin_scale_value[2] += 0.01f;
            }
            else if (shape[i].origin_scale == -1) {
                shape[i].origin_scale_value[0] -= 0.01f;
                shape[i].origin_scale_value[1] -= 0.01f;
                shape[i].origin_scale_value[2] -= 0.01f;
                if (shape[i].origin_scale_value[0] < 0.1f) {
                    shape[i].origin_scale_value[0] = 0.1f;
                    shape[i].origin_scale_value[1] = 0.1f;
                    shape[i].origin_scale_value[2] = 0.1f;
                }
            }
            
            if (shape[i].self_scale == 1) {
                shape[i].self_scale_value[0] += 0.01f;
                shape[i].self_scale_value[1] += 0.01f;
                shape[i].self_scale_value[2] += 0.01f;
            }
            else if (shape[i].self_scale == -1) {
                shape[i].self_scale_value[0] -= 0.01f;
                shape[i].self_scale_value[1] -= 0.01f;
                shape[i].self_scale_value[2] -= 0.01f;
                if (shape[i].self_scale_value[0] < 0.1f) {
                    shape[i].self_scale_value[0] = 0.1f;
                    shape[i].self_scale_value[1] = 0.1f;
                    shape[i].self_scale_value[2] = 0.1f;
                }
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0);
}