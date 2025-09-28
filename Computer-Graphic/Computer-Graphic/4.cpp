#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <cmath>
#include <ctime>

#define maxrectcount 5 // 최대 5개 사각형

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLclampf backgroundColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f }; // 진한 회색

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
    float vx, vy; // 속도
    bool exist = false;
    int combinecount = 0;
    AnimationType animType = NONE;
    float sizeDir = 1.0f; // 크기 변화 방향
    float orig_x, orig_y; // 원래 위치 저장

    void makeRectangles(float cx, float cy) {
        color[0] = getRandomcolor();
        color[1] = getRandomcolor();
        color[2] = getRandomcolor();
        color[3] = 1.0f;
        x = cx;
        y = cy;
        w = 0.2f;
        h = 0.2f;
        exist = true;
        combinecount = 0;
        animType = NONE;
        vx = 0.01f + getRandomfloat() / 20.0f;
        vy = 0.01f + getRandomfloat() / 20.0f;
        sizeDir = 1.0f;
        orig_x = x;
        orig_y = y;
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
        vx = 0.0f;
        vy = 0.0f;
        exist = false;
        combinecount = 0;
        animType = NONE;
        sizeDir = 1.0f;
        orig_x = 0.0f;
        orig_y = 0.0f;
    }
};
rect rectangles[maxrectcount];
int rectCount = 0;
bool isDragging = false;
int draggingRect = -1;
float lastX, lastY;
bool animationEnabled = true;
int followIndex = -1; // 따라하기용

void main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 600);
    glutCreateWindow("실습4");
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
    }
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
    glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case '1': case 'a': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) rectangles[i].animType = DIAGONAL;
        animationEnabled = true;
        break;
    case '2': case 'b': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) rectangles[i].animType = ZIGZAG;
        animationEnabled = true;
        break;
    case '3': case 'c': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) rectangles[i].animType = SIZE_ANIM;
        animationEnabled = true;
        break;
    case '4': case 'd': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) rectangles[i].animType = COLOR;
        animationEnabled = true;
        break;
    case '5': case 'e': 
        if (rectCount > 0) {
            followIndex = rand() % rectCount;
            for (int i = 0; i < maxrectcount; i++)
                if (rectangles[i].exist) rectangles[i].animType = (i == followIndex ? DIAGONAL : FOLLOW);
            animationEnabled = true;
        }
        break;
    case 's': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) rectangles[i].animType = NONE;
        animationEnabled = false;
        break;
    case 'm': 
        for (int i = 0; i < maxrectcount; i++)
            if (rectangles[i].exist) {
                rectangles[i].x = rectangles[i].orig_x;
                rectangles[i].y = rectangles[i].orig_y;
            }
        break;
    case 'r': 
        for (int i = 0; i < maxrectcount; i++)
            rectangles[i].init();
        rectCount = 0;
        break;
    case 'q': 
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
    if (animationEnabled) {
        for (int i = 0; i < maxrectcount; i++) {
            if (!rectangles[i].exist) continue;
            switch (rectangles[i].animType) {
            case DIAGONAL:
                rectangles[i].x += rectangles[i].vx;
                rectangles[i].y += rectangles[i].vy;
                if (rectangles[i].x < -1.0f || rectangles[i].x + rectangles[i].w > 1.0f)
                    rectangles[i].vx *= -1;
                if (rectangles[i].y < -1.0f || rectangles[i].y + rectangles[i].h > 1.0f)
                    rectangles[i].vy *= -1;
                break;
            case ZIGZAG:
                rectangles[i].x += rectangles[i].vx;
                rectangles[i].y += rectangles[i].vy * sinf(glutGet(GLUT_ELAPSED_TIME) / 200.0f + i);
                if (rectangles[i].x < -1.0f || rectangles[i].x + rectangles[i].w > 1.0f)
                    rectangles[i].vx *= -1;
                if (rectangles[i].y < -1.0f || rectangles[i].y + rectangles[i].h > 1.0f)
                    rectangles[i].vy *= -1;
                break;
            case SIZE_ANIM:
                rectangles[i].w += 0.005f * rectangles[i].sizeDir;
                rectangles[i].h += 0.005f * rectangles[i].sizeDir;
				printf("%f %f\n", rectangles[i].w, rectangles[i].h);
                if (rectangles[i].w > 0.4f || rectangles[i].w  < 0.1f || rectangles[i].h > 0.4f || rectangles[i].h < 0.1f)
                    rectangles[i].sizeDir *= -1;
                break;
            case COLOR:
                rectangles[i].color[0] = getRandomcolor();
                rectangles[i].color[1] = getRandomcolor();
                rectangles[i].color[2] = getRandomcolor();
                break;
            case FOLLOW:
                if (followIndex >= 0 && followIndex < maxrectcount && rectangles[followIndex].exist) {
                    float tx = rectangles[followIndex].x;
                    float ty = rectangles[followIndex].y;
                    rectangles[i].x += (tx - rectangles[i].x) * 0.05f;
                    rectangles[i].y += (ty - rectangles[i].y) * 0.05f;
                }
                break;
            default:
                break;
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunction, 1); // 60fps
}

GLvoid Mouse(int button, int state, int x, int y)
{
    float x_ndc = (1.0f * x / 400 - 1.0f);
    float y_ndc = -(1.0f * y / 300 - 1.0f);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (rectCount < maxrectcount) {
            for (int i = 0; i < maxrectcount; i++) {
                if (!rectangles[i].exist) {
                    rectangles[i].makeRectangles(x_ndc, y_ndc);
                    rectCount++;
                    break;
                }
            }
        }
        else {
            // 이미 생성된 사각형 클릭 시 드래그
            draggingRect = -1;
            for (int i = 0; i < maxrectcount; i++) {
                if (rectangles[i].exist &&
                    x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
                    y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
                    draggingRect = i;
                    isDragging = true;
                    lastX = x_ndc;
                    lastY = y_ndc;
                    break;
                }
            }
        }
        glutPostRedisplay();
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        isDragging = false;
        draggingRect = -1;
        glutPostRedisplay();
    }
}

GLvoid Motion(int x, int y)
{
    if (isDragging && draggingRect != -1) {
        float x_ndc = (1.0f * x / 400 - 1.0f);
        float y_ndc = -(1.0f * y / 300 - 1.0f);
        rectangles[draggingRect].x += (x_ndc - lastX);
        rectangles[draggingRect].y += (y_ndc - lastY);
        lastX = x_ndc;
        lastY = y_ndc;
        glutPostRedisplay();
    }
}