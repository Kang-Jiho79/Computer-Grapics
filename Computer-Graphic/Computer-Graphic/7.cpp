#include <iostream>
#include <random>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <cmath>
#include <ctime>

#define maxrectcount 10

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

GLclampf backgroundColor[4] = { 0.9f, 0.9f, 0.9f, 1.0f }; // 연한 회색

// 전역 랜덤 생성기 (문제 해결!)
std::random_device rd;
std::mt19937 gen(rd());

float getRandomcolor()
{
	std::uniform_real_distribution<float> dis(0.2f, 0.8f);
	return dis(gen);
}

float getRandomfloat(float min = -0.5f, float max = 0.5f)
{
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

class rect {
public:
	GLclampf color[4];
	float x, y, w, h;
	bool exist = false;
	bool isMatched = false; // 매칭되었는지 확인
	bool isTarget = false;  // 목표 블록인지 확인

	// 좌측 목표 블록 생성 (검은색, 반투명)
	void makeTargetRectangles() {
		color[0] = 0.2f;
		color[1] = 0.2f;
		color[2] = 0.2f;
		color[3] = 1.0f; // 반투명

		// 좌측에 더 넓게 배치 
		x = getRandomfloat(-0.9f, -0.2f);
		y = getRandomfloat(-0.7f, 0.7f);
		w = getRandomfloat(0.08f, 0.15f);
		h = getRandomfloat(0.08f, 0.15f);

		exist = true;
		isTarget = true;
		isMatched = false;
	}

	// 우측 플레이어 블록 생성 - 격자 형태로 배치
	void makePlayerRectangle(float targetW, float targetH, int index) {
		color[0] = getRandomcolor();
		color[1] = getRandomcolor();
		color[2] = getRandomcolor();
		color[3] = 1.0f;

		// 격자 형태로 배치 (3x4 또는 2x5)
		int cols = 2; // 열 개수
		int rows = (maxrectcount + cols - 1) / cols; // 행 개수

		int row = index / cols;
		int col = index % cols;

		float spacing = 0.25f;
		float startX = 0.1f;
		float startY = 0.6f;

		x = startX + col * spacing;
		y = startY - row * spacing;

		// 크기 조정
		w = targetW;
		h = targetH;

		exist = true;
		isTarget = false;
		isMatched = false;
	}

	void init() {
		color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.0f; color[3] = 1.0f;
		x = 0.0f; y = 0.0f; w = 0.0f; h = 0.0f;
		exist = false;
		isMatched = false;
		isTarget = false;
	}

	// 다른 블록과 가까운지 확인 (매칭 판정)
	bool isNear(const rect& other, float tolerance = 0.1f) {
		if (!exist || !other.exist) return false;

		float centerX1 = x + w / 2;
		float centerY1 = y + h / 2;
		float centerX2 = other.x + other.w / 2;
		float centerY2 = other.y + other.h / 2;

		float distance = sqrt(pow(centerX1 - centerX2, 2) + pow(centerY1 - centerY2, 2));
		float sizeMatch = abs(w - other.w) < 0.03f && abs(h - other.h) < 0.03f;

		return distance < tolerance && sizeMatch;
	}
};

rect targets[maxrectcount];    // 좌측 목표 블록들
rect playerBlocks[maxrectcount]; // 우측 플레이어 블록들
bool isDragging = false;
int draggingRect = -1;
float lastX, lastY;
int matchedCount = 0;
bool gameComplete = false;

// 매칭 확인 함수
void checkMatching() {
	matchedCount = 0;

	// 모든 플레이어 블록의 매칭 상태 초기화
	for (int i = 0; i < maxrectcount; i++) {
		playerBlocks[i].isMatched = false;
	}

	// 각 플레이어 블록에 대해 목표 블록과 매칭 확인
	for (int i = 0; i < maxrectcount; i++) {
		if (!playerBlocks[i].exist) continue;
		if (playerBlocks[i].isNear(targets[i])) {
			playerBlocks[i].isMatched = true;
			matchedCount++;
		}
	}

	// 게임 완료 확인
	gameComplete = (matchedCount == maxrectcount);
	if (gameComplete) {
		printf("축하합니다! 모든 블록을 맞췄습니다!\n");
	}
}

void main(int argc, char** argv)
{
	// 시드 설정
	srand(static_cast<unsigned int>(time(NULL)));
	gen.seed(static_cast<unsigned int>(time(NULL)));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("실습7 - 블록 맞추기 퍼즐");
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	// 목표 블록들 생성
	for (int i = 0; i < maxrectcount; i++) {
		targets[i].init();
		playerBlocks[i].init();

		targets[i].makeTargetRectangles();
		playerBlocks[i].makePlayerRectangle(targets[i].w, targets[i].h, i);
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

	glColor4f(0.8f, 0.9f, 1.0f, 0.3f);
	glRectf(-1.0f, -1.0f, 0.0f, 1.0f);

	glColor4f(1.0f, 0.9f, 0.8f, 0.3f);
	glRectf(0.0f, -1.0f, 1.0f, 1.0f);

	// 목표 블록들 그리기 (좌측)
	for (int i = 0; i < maxrectcount; i++) {
		if (targets[i].exist) {
			glColor4f(targets[i].color[0], targets[i].color[1], targets[i].color[2], targets[i].color[3]);
			glRectf(targets[i].x, targets[i].y, targets[i].x + targets[i].w, targets[i].y + targets[i].h);
		}
	}

	// 플레이어 블록들 그리기 (우측)
	for (int i = 0; i < maxrectcount; i++) {
		if (playerBlocks[i].exist) {
			if (playerBlocks[i].isMatched) {
				glColor4f(0.0f, 0.9f, 0.0f, 0.9f); // 밝은 초록색
			}
			else {
				glColor4f(playerBlocks[i].color[0], playerBlocks[i].color[1], playerBlocks[i].color[2], playerBlocks[i].color[3]);
			}

			glRectf(playerBlocks[i].x, playerBlocks[i].y,
				playerBlocks[i].x + playerBlocks[i].w, playerBlocks[i].y + playerBlocks[i].h);
		}
	}
	glEnd();

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'r': { // 새 게임
		gameComplete = false;
		matchedCount = 0;
		for (int i = 0; i < maxrectcount; i++) {
			targets[i].init();
			playerBlocks[i].init();

			targets[i].makeTargetRectangles();
			playerBlocks[i].makePlayerRectangle(targets[i].w, targets[i].h, i);
		}
		break;
	}
	case 'q': { // 종료
		glutLeaveMainLoop();
		break;
	}
	}
	glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
	checkMatching(); // 매 프레임마다 매칭 확인
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float x_ndc = (1.0f * x / 400 - 1.0f);
	float y_ndc = -(1.0f * y / 300 - 1.0f);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		draggingRect = -1;

		// 우측 영역의 플레이어 블록만 드래그 가능
		if (x_ndc > 0.0f) {
			for (int i = 0; i < maxrectcount; i++) {
				if (playerBlocks[i].exist &&
					x_ndc >= playerBlocks[i].x && x_ndc <= playerBlocks[i].x + playerBlocks[i].w &&
					y_ndc >= playerBlocks[i].y && y_ndc <= playerBlocks[i].y + playerBlocks[i].h) {

					draggingRect = i;
					isDragging = true;
					lastX = x_ndc;
					lastY = y_ndc;
					printf("블록 %d 드래그 시작 위치: (%.2f, %.2f)\n", i, x_ndc, y_ndc);
					break;
				}
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (draggingRect != -1) {
			printf("블록 % d 드래그 종료 위치 : (% .2f, % .2f)\n", draggingRect,
				playerBlocks[draggingRect].x, playerBlocks[draggingRect].y);
			isDragging = false;
			draggingRect = -1;
		}
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y)
{
	if (isDragging && draggingRect != -1) {
		float x_ndc = (1.0f * x / 400 - 1.0f);
		float y_ndc = -(1.0f * y / 300 - 1.0f);

		float deltaX = x_ndc - lastX;
		float deltaY = y_ndc - lastY;

		playerBlocks[draggingRect].x += deltaX;
		playerBlocks[draggingRect].y += deltaY;

		lastX = x_ndc;
		lastY = y_ndc;
		glutPostRedisplay();
	}
}