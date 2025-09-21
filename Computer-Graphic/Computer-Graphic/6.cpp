#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <cmath>
#include <ctime>

#define maxrectcount 40 // 분할된 사각형들을 위해 충분한 개수

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);

GLclampf backgroundColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f }; // 진한 회색좌우상하

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

enum AnimationType { NONE, HORVER, DIAGONAL, ONEDIRECTION, EIGHTDIRECTION};

float Vx = 0.00f;
float Vy = 0.00f;

class rect {
public:
    GLclampf color[4];
    float x, y, w, h;
    float vx, vy; // 속도
    bool exist = false;
    bool isDivided = false; // 분할된 사각형인지
    bool isMoving = false; // 이동 중인지
    AnimationType animType = NONE;
    float shrinkRate = 0.995f; // 축소 비율

    void makeRectangles(float cx = 0, float cy = 0, float size = 0) {
        color[0] = getRandomcolor();
        color[1] = getRandomcolor();
        color[2] = getRandomcolor();
        color[3] = 1.0f;
        
        if (size == 0) { // 초기 사각형
            x = getRandomfloat();
            y = getRandomfloat();
            w = 0.15f + getRandomfloat() * 0.1f; // 0.15 ~ 0.25
            h = 0.15f + getRandomfloat() * 0.1f;
        } else { // 분할된 사각형
            x = cx;
            y = cy;
            w = size;
            h = size;
        }
        
        exist = true;
        isDivided = false;
        isMoving = false;
        animType = NONE;
        vx = 0.0f;
        vy = 0.0f;
    }

    void startAnimation(int position, AnimationType type) {
        animType = type;
        isMoving = true;
        
        float speed = 0.02f;
        switch (type) {
        case HORVER:
            switch (position)
            {
            case 0: {
                vx = -speed;
                vy = 0;
            } break;
            case 1: {
                vx = 0;
				vy = speed;
            }break;
            case 2: {
                vx = 0;
				vy = -speed;
			} break;
            case 3: {
                vx = speed;
                vy = 0;
		    } break;
            default:
                break;
            }
            break;
        case DIAGONAL:
            switch (position)
            {
            case 0: {
                vx = -speed;
                vy = speed;
            } break;
            case 1: {
                vx = speed;
                vy = speed;
            }break;
            case 2: {
                vx = -speed;
                vy = -speed;
            } break;
            case 3: {
                vx = speed;
                vy = -speed;
            } break;
            default:
                break;
            }
            break;
        case ONEDIRECTION:
            vx = Vx;
            vy = Vy;
            break;
        case EIGHTDIRECTION:
            switch (position)
            {
            case 0: {
                vx = -speed;
                vy = speed;
            } break;
            case 1: {
                vx = 0;
                vy = speed;
            }break;
            case 2: {
                vx = speed;
                vy = speed;
            }break;
            case 3: {
                vx = speed;
                vy = 0;
            }break;
            case 4: {
                vx = -speed;
                vy = -speed;
            } break;
            case 5: {
                vx = 0;
                vy = -speed;
            }break;
            case 6: {
                vx = speed;
                vy = -speed;
            } break;
            case 7: {
                vx = -speed;
                vy = 0;
            }break;
            default:
                break;
            }
            break;
        default:
            break;
        }
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
        isDivided = false;
        isMoving = false;
        animType = NONE;
    }
};

rect rectangles[maxrectcount];
int rectCount = 0;
bool isDragging = false;
int draggingRect = -1;
float lastX, lastY;
bool animationEnabled = true;


void divideRectangle(int index) {
    if (!rectangles[index].exist || rectangles[index].isDivided) return;
    
    float originalX = rectangles[index].x;
    float originalY = rectangles[index].y;
    float halfW = rectangles[index].w / 2.0f;
    float halfH = rectangles[index].h / 2.0f;
    
    // 원본 사각형을 분할된 것으로 표시하고 비활성화
    rectangles[index].isDivided = true;
    rectangles[index].exist = false;
    
    // 4개의 작은 사각형 생성
    int created = 0;
    AnimationType animType;
    animType = static_cast<AnimationType>(1 + rand() % 4);
    if (animType == ONEDIRECTION) {
        float angle = (rand() % 360) * 3.14159f / 180.0f; // 랜덤 각도
        Vx = 0.02f * cos(angle);
        Vy = 0.02f * sin(angle);
	}
    if (animType != EIGHTDIRECTION) {
        for (int i = 0; i < maxrectcount && created < 4; i++) {
            if (!rectangles[i].exist) {
                float newX, newY;

                switch (created) {
                case 0: // 좌상
                    newX = originalX;
                    newY = originalY;

                    break;
                case 1: // 우상
                    newX = originalX + halfW;
                    newY = originalY;
                    break;
                case 2: // 좌하
                    newX = originalX;
                    newY = originalY + halfH;
                    break;
                case 3: // 우하
                    newX = originalX + halfW;
                    newY = originalY + halfH;
                    break;
                }
                rectangles[i].makeRectangles(newX, newY, halfW);
                rectangles[i].isDivided = true;
                rectangles[i].startAnimation(created, animType);
                rectCount++;
                created++;
            }
        }
    }
    else {
		float onethirdW = rectangles[index].w / 3.0f;
		float onethirdH = rectangles[index].h / 3.0f;
        for (int i = 0; i < maxrectcount && created < 8; i++) {
            if (!rectangles[i].exist) {
                float newX, newY;
                switch (created) {
                case 0: // 좌상
                    newX = originalX;
                    newY = originalY;
                    break;
                case 1:
                    newX = originalX + onethirdW;
                    newY = originalY;
                    break;
                case 2:
                    newX = originalX + onethirdW * 2;
                    newY = originalY;
                    break;
                case 3:
                    newX = originalX + onethirdW * 2;
                    newY = originalY + onethirdH;
                    break;
                case 4:
                    newX = originalX + onethirdW * 2;
                    newY = originalY + onethirdH * 2;
                    break;
                case 5:
                    newX = originalX + onethirdW;
                    newY = originalY + onethirdH * 2;
                    break;
                case 6:
                    newX = originalX;
                    newY = originalY + onethirdH * 2;
                    break;
                case 7:
                    newX = originalX;
                    newY = originalY + onethirdH;
                    break;
                }
                rectangles[i].makeRectangles(newX, newY, halfW);
                rectangles[i].isDivided = true;
                rectangles[i].startAnimation(created, animType);
                rectCount++;
                created++;
            }
        }
    }
}

void main(int argc, char** argv)
{
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 600);
    glutCreateWindow("실습6");
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "GLEW Initialized\n";
    
    // 초기화
    for (int i = 0; i < maxrectcount; i++) {
        rectangles[i].init();
    }
    
    // 5~10개의 사각형 랜덤 생성
    int numRects = 5 + rand() % 6; // 5~10개
    for (int i = 0; i < numRects; i++) {
        rectangles[i].makeRectangles();
        rectCount++;
    }
    
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, TimerFunction, 1); // 60fps
    glutMouseFunc(Mouse);
    glutMainLoop();
}

GLvoid drawScene()
{
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (int i = 0; i < maxrectcount; i++) {
        if (rectangles[i].exist) {
            glColor3f(rectangles[i].color[0], rectangles[i].color[1], rectangles[i].color[2]);
            glRectf(rectangles[i].x, rectangles[i].y, 
                   rectangles[i].x + rectangles[i].w, rectangles[i].y + rectangles[i].h);
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
    case 'r': { // 전체 재시작
        for (int i = 0; i < maxrectcount; i++) {
            rectangles[i].init();
        }
        rectCount = 0;
        
        // 5~10개의 사각형 랜덤 생성
        int numRects = 5 + rand() % 6;
        for (int i = 0; i < numRects; i++) {
            rectangles[i].makeRectangles();
            rectCount++;
        }
        break;
    }
    case 'q': // 종료
        glutLeaveMainLoop();
        break;
    }
    glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
    if (animationEnabled) {
        for (int i = 0; i < maxrectcount; i++) {
            if (!rectangles[i].exist || !rectangles[i].isMoving) continue;
            
            // 이동
            rectangles[i].x += rectangles[i].vx;
            rectangles[i].y += rectangles[i].vy;
            
            // 크기 축소
            rectangles[i].w *= rectangles[i].shrinkRate;
            rectangles[i].h *= rectangles[i].shrinkRate;
            
            // 화면 경계 벗어나거나 너무 작아지면 제거
            if (rectangles[i].x < -1.5f || rectangles[i].x > 1.5f ||
                rectangles[i].y < -1.5f || rectangles[i].y > 1.5f ||
                rectangles[i].w < 0.01f || rectangles[i].h < 0.01f) {
                rectangles[i].exist = false;
                rectangles[i].isMoving = false;
                rectCount--;
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
        // 클릭한 위치의 사각형 찾기
        for (int i = 0; i < maxrectcount; i++) {
            if (rectangles[i].exist && !rectangles[i].isDivided && !rectangles[i].isMoving &&
                x_ndc >= rectangles[i].x && x_ndc <= rectangles[i].x + rectangles[i].w &&
                y_ndc >= rectangles[i].y && y_ndc <= rectangles[i].y + rectangles[i].h) {
                
                printf("사각형 %d를 분할합니다!\n", i);
                divideRectangle(i);
                break;
            }
        }
        glutPostRedisplay();
    }
}