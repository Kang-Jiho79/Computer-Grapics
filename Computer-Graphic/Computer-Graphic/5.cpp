#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <cmath>
#include <ctime>

#define maxrectcount 40 // 최대 5개 사각형

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLclampf backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 진한 회색

float getRandomcolor()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
}

float getRandomfloat()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-0.8f, 0.8f);
    return dis(gen);
}

enum AnimationType { NONE, DIAGONAL, ZIGZAG, SIZE_ANIM, COLOR, FOLLOW };

class rect {
public:
    GLclampf color[4];
    float x, y, w, h;
    bool exist = false;

    void makeRectangles() {
        color[0] = getRandomcolor();
        color[1] = getRandomcolor();
        color[2] = getRandomcolor();
        color[3] = 1.0f;
        x = getRandomfloat();
        y = getRandomfloat();
        w = 0.05f;
        h = 0.05f;
        exist = true;
    }

    void init() {
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
        x = 0.0f;
        y = 0.0f;
        w = 0.0f;
        h = 0.0f;
        exist = false;
    }
};

// 두 사각형이 충돌하는지 확인하는 함수 (AABB 충돌 검사)
bool isColliding(const rect& rect1, const rect& rect2) {
    // 두 사각형이 모두 존재하는지 확인
    if (!rect1.exist || !rect2.exist) {
        return false;
    }

    // AABB (Axis-Aligned Bounding Box) 충돌 검사
    return (rect1.x < rect2.x + rect2.w &&
        rect1.x + rect1.w > rect2.x &&
        rect1.y < rect2.y + rect2.h &&
        rect1.y + rect1.h > rect2.y);
}

// 특정 사각형과 배열의 다른 모든 사각형과의 충돌을 확인하는 함수
int checkCollisionWithAll(const rect& targetRect, const rect rectangles[], int arraySize, int excludeIndex = -1) {
    for (int i = 0; i < arraySize; i++) {
        // 자기 자신과는 비교하지 않음
        if (i == excludeIndex) {
            continue;
        }

        if (isColliding(targetRect, rectangles[i])) {
            return i; // 충돌한 사각형의 인덱스 반환
        }
    }
    return -1; // 충돌하지 않음
}

rect rectangles[maxrectcount];
rect erase;
bool isDragging = false;
int rectCount = 0;

void main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 600);
    glutCreateWindow("실습5");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    for (int i = 0; i < maxrectcount; i++) {
        rectangles[i].init();
		rectangles[i].makeRectangles();
		rectCount++;
    }
	erase.init();
	erase.makeRectangles();
    erase.w += 0.05;
    erase.h += 0.05;

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, TimerFunction, 1); // 60fps
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < maxrectcount; i++) {
        if (rectangles[i].exist) {
            glColor3f(rectangles[i].color[0], rectangles[i].color[1], rectangles[i].color[2]);
            glRectf(rectangles[i].x, rectangles[i].y, rectangles[i].x + rectangles[i].w, rectangles[i].y + rectangles[i].h);
        }
    }
    if (isDragging) {
		glColor3b(erase.color[0], erase.color[1], erase.color[2]);
		glRectf(erase.x, erase.y, erase.x + erase.w, erase.y + erase.h);
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
    case 'r': // 전체 삭제
        for (int i = 0; i < maxrectcount; i++) {
            rectangles[i].init();
            rectangles[i].makeRectangles();
            erase.init();
            erase.makeRectangles();
            erase.w += 0.05;
            erase.h += 0.05;
			rectCount = maxrectcount;
        }
        break;
    case 'q': // 종료
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunction, 1); // 60fps
}

GLvoid Mouse(int button, int state, int x, int y)
{
    float x_ndc = (1.0f * x / 400 - 1.0f);
    float y_ndc = -(1.0f * y / 300 - 1.0f);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isDragging = true;
		erase.x = x_ndc - erase.w / 2;
		erase.y = x_ndc - erase.w / 2;
        glutPostRedisplay();
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        isDragging = false;
		erase.color[0] = 1.0f;
		erase.color[1] = 1.0f;
		erase.color[2] = 1.0f;
        glutPostRedisplay();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (rectCount < maxrectcount) {
            for (int i = 0; i < maxrectcount; i++) {
                if (!rectangles[i].exist) {
                    rectangles[i].makeRectangles();
					rectangles[i].x = x_ndc - rectangles[i].w / 2;
					rectangles[i].y = y_ndc - rectangles[i].h / 2;
                    rectCount++;
                    break;
                }
            }
        }
        glutPostRedisplay();
	}
}

GLvoid Motion(int x, int y)
{
    if (isDragging) {
        float x_ndc = (1.0f * x / 400 - 1.0f);
        float y_ndc = -(1.0f * y / 300 - 1.0f);
        erase.x = x_ndc - erase.w / 2;
        erase.y = y_ndc - erase.h / 2;
		int collidedIndex = checkCollisionWithAll(erase, rectangles, maxrectcount);
        if (collidedIndex != -1) {
			rectCount--;
            erase.w += 0.05;
            erase.h += 0.05;
			erase.color[0] = rectangles[collidedIndex].color[0];
			erase.color[1] = rectangles[collidedIndex].color[1];
			erase.color[2] = rectangles[collidedIndex].color[2];
            rectangles[collidedIndex].init();
        }
        glutPostRedisplay();
    }
}