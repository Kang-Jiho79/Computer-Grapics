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
	bool isMoving = false;
	float targetX, targetY;
	float moveSpeed = 0.02f;

	// 충돌 검사 함수
	bool isOverlapping(const rect& other) const {
		if (!exist || !other.exist) return false;

		return !(x + w <= other.x || other.x + other.w <= x ||
			y + h <= other.y || other.y + other.h <= y);
	}

	// 인접 검사 함수 (붙어있는지 확인)
	bool isAdjacent(const rect& other, float tolerance = 0.02f) const {
		if (!exist || !other.exist) return false;

		// 수평으로 붙어있는지 확인
		bool horizontallyAdjacent =
			(abs((x + w) - other.x) < tolerance || abs((other.x + other.w) - x) < tolerance) &&
			!(y + h <= other.y || other.y + other.h <= y);

		// 수직으로 붙어있는지 확인
		bool verticallyAdjacent =
			(abs((y + h) - other.y) < tolerance || abs((other.y + other.h) - y) < tolerance) &&
			!(x + w <= other.x || other.x + other.w <= x);

		return horizontallyAdjacent || verticallyAdjacent;
	}

	// 애니메이션 시작 함수
	void startMoveAnimation(float destX, float destY) {
		if (!isMatched) {
			isMoving = true;
			targetX = destX;
			targetY = destY;
		}
	}

	// 애니메이션 업데이트 함수
	void updateAnimation() {
		if (isMoving) {
			float dx = targetX - x;
			float dy = targetY - y;
			float distance = sqrt(dx * dx + dy * dy);

			if (distance < 0.005f) {
				// 목표에 도달
				x = targetX;
				y = targetY;
				isMoving = false;
				isMatched = true;
				printf("블록 매칭 완료!\n");
			}
			else {
				// 목표를 향해 이동
				x += dx * moveSpeed;
				y += dy * moveSpeed;
			}
		}
	}

	// 좌측 목표 블록 생성 (충돌 방지 + 인접 배치)
	void makeTargetRectangles(int index, rect targets[], int targetCount) {
		color[0] = 0.2f;
		color[1] = 0.2f;
		color[2] = 0.2f;
		color[3] = 1.0f;

		// 블록 크기 (고정 크기로 패턴 생성을 쉽게)
		w = 0.12f + getRandomfloat(0.0f, 0.03f);
		h = 0.12f + getRandomfloat(0.0f, 0.03f);

		if (index == 0) {
			// 첫 번째 블록은 좌측 중앙에 배치
			x = -0.8f;
			y = 0.0f;
		}
		else {
			// 기존 블록들과 인접하게 배치
			bool placed = false;
			int attempts = 0;
			const int maxAttempts = 100;

			while (!placed && attempts < maxAttempts) {
				// 기존 블록 중 하나를 랜덤 선택
				int baseIndex = rand() % index;
				if (!targets[baseIndex].exist) {
					attempts++;
					continue;
				}

				// 선택된 블록의 인접 위치 중 하나를 랜덤 선택
				int direction = rand() % 4; // 0:오른쪽, 1:왼쪽, 2:위, 3:아래

				switch (direction) {
				case 0: // 오른쪽
					x = targets[baseIndex].x + targets[baseIndex].w;
					y = targets[baseIndex].y + getRandomfloat(-0.05f, 0.05f);
					break;
				case 1: // 왼쪽
					x = targets[baseIndex].x - w;
					y = targets[baseIndex].y + getRandomfloat(-0.05f, 0.05f);
					break;
				case 2: // 위
					x = targets[baseIndex].x + getRandomfloat(-0.05f, 0.05f);
					y = targets[baseIndex].y + targets[baseIndex].h;
					break;
				case 3: // 아래
					x = targets[baseIndex].x + getRandomfloat(-0.05f, 0.05f);
					y = targets[baseIndex].y - h;
					break;
				}

				// 경계 확인
				if (x < -0.95f || x + w > -0.05f || y < -0.85f || y + h > 0.85f) {
					attempts++;
					continue;
				}

				// 다른 블록들과 충돌 확인
				bool hasCollision = false;
				for (int i = 0; i < index; i++) {
					if (targets[i].exist) {
						rect tempRect = *this;
						tempRect.exist = true;
						if (tempRect.isOverlapping(targets[i])) {
							hasCollision = true;
							break;
						}
					}
				}

				if (!hasCollision) {
					placed = true;
				}
				else {
					attempts++;
				}
			}

			// 배치 실패 시 폴백: 랜덤 위치
			if (!placed) {
				x = getRandomfloat(-0.9f, -0.2f);
				y = getRandomfloat(-0.7f, 0.7f);

				// 최소한의 충돌 회피 시도
				for (int i = 0; i < 10; i++) {
					bool hasCollision = false;
					for (int j = 0; j < index; j++) {
						if (targets[j].exist) {
							rect tempRect = *this;
							tempRect.exist = true;
							if (tempRect.isOverlapping(targets[j])) {
								hasCollision = true;
								break;
							}
						}
					}
					if (!hasCollision) break;

					x = getRandomfloat(-0.9f, -0.2f);
					y = getRandomfloat(-0.7f, 0.7f);
				}
			}
		}

		exist = true;
		isTarget = true;
		isMatched = false;
	}
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

	void makePlayerRectangle(float targetW, float targetH, int index) {
		color[0] = getRandomcolor();
		color[1] = getRandomcolor();
		color[2] = getRandomcolor();
		color[3] = 1.0f;

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

	bool isNear(const rect& other, float tolerance = 0.01f) {
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
int currentTargetCount = 0; // 실제 목표 블록 개수


bool autoMatching = false;
int currentAutoMatchIndex = 0;
int autoMatchTimer = 0;
const int autoMatchDelay = 30;

void generateTargetPattern() {
	printf("🎯 목표 패턴 생성 중...\n");

	for (int i = 0; i < maxrectcount; i++) {
		targets[i].init();
	}

	currentTargetCount = 5 + (rand() % 4);
	printf("목표 블록 개수: %d개\n", currentTargetCount);

	// 첫 번째 블록 - 랜덤 크기로 시작
	targets[0].x = -0.7f;
	targets[0].y = 0.0f;
	targets[0].w = getRandomfloat(0.08f, 0.16f);
	targets[0].h = getRandomfloat(0.08f, 0.16f);
	targets[0].color[0] = 0.3f;
	targets[0].color[1] = 0.3f;
	targets[0].color[2] = 0.3f;
	targets[0].color[3] = 1.0f;
	targets[0].exist = true;
	targets[0].isTarget = true;

	// 나머지 블록들을 순차적으로 인접 배치
	for (int i = 1; i < currentTargetCount; i++) {
		// 각 블록마다 랜덤 크기 설정
		targets[i].w = getRandomfloat(0.08f, 0.16f);
		targets[i].h = getRandomfloat(0.08f, 0.16f);

		bool placed = false;
		int attempts = 0;

		while (!placed && attempts < 50) {
			int baseIndex = rand() % i;

			int direction = rand() % 4;

			float newX, newY;
			switch (direction) {
			case 0: // 오른쪽
				newX = targets[baseIndex].x + targets[baseIndex].w;
				newY = targets[baseIndex].y;
				break;
			case 1: // 왼쪽
				newX = targets[baseIndex].x - targets[i].w;
				newY = targets[baseIndex].y;
				break;
			case 2: // 위
				newX = targets[baseIndex].x;
				newY = targets[baseIndex].y + targets[baseIndex].h;
				break;
			case 3: // 아래
				newX = targets[baseIndex].x;
				newY = targets[baseIndex].y - targets[i].h;
				break;
			}

			// 경계 체크 - 각 블록의 실제 크기 사용
			if (newX < -0.95f || newX + targets[i].w > -0.05f ||
				newY < -0.8f || newY + targets[i].h > 0.8f) {
				attempts++;
				continue;
			}

			// 충돌 체크 - 각 블록의 실제 크기 사용
			bool collision = false;
			rect tempBlock;
			tempBlock.x = newX;
			tempBlock.y = newY;
			tempBlock.w = targets[i].w;
			tempBlock.h = targets[i].h;
			tempBlock.exist = true;

			for (int j = 0; j < i; j++) {
				if (targets[j].isOverlapping(tempBlock)) {
					collision = true;
					break;
				}
			}

			if (!collision) {
				targets[i].x = newX;
				targets[i].y = newY;
				targets[i].color[0] = 0.3f;
				targets[i].color[1] = 0.3f;
				targets[i].color[2] = 0.3f;
				targets[i].color[3] = 1.0f;
				targets[i].exist = true;
				targets[i].isTarget = true;
				placed = true;
				printf("블록 %d 배치: (%.2f, %.2f) 크기(%.2f, %.2f)\n", i, newX, newY, targets[i].w, targets[i].h);
			}

			attempts++;
		}

		// 배치 실패 시 강제 배치
		if (!placed) {
			targets[i].x = -0.8f + (i * 0.13f);
			targets[i].y = -0.3f + ((i % 3) * 0.13f);
			targets[i].color[0] = 0.3f;
			targets[i].color[1] = 0.3f;
			targets[i].color[2] = 0.3f;
			targets[i].color[3] = 1.0f;
			targets[i].exist = true;
			targets[i].isTarget = true;
			printf("블록 %d 강제 배치: (%.2f, %.2f) 크기(%.2f, %.2f)\n", i, targets[i].x, targets[i].y, targets[i].w, targets[i].h);
		}
	}

	printf("✅ 목표 패턴 생성 완료!\n");
}

// 자동 매칭 시작 함수
void startAutoMatching() {
	if (autoMatching) return; // 이미 진행 중이면 무시

	autoMatching = true;
	currentAutoMatchIndex = 0;
	autoMatchTimer = 0;
	printf("🚀 자동 매칭 시작!\n");
}

// 다음 매치되지 않은 블록 찾기
int findNextUnmatchedBlock() {
	for (int i = currentAutoMatchIndex; i < currentTargetCount; i++) {
		if (!playerBlocks[i].isMatched && !playerBlocks[i].isMoving) {
			return i;
		}
	}
	return -1; // 매치되지 않은 블록이 없음
}

// 매칭 확인 함수
void checkMatching() {
	matchedCount = 0;

	for (int i = 0; i < currentTargetCount; i++) {
		playerBlocks[i].isMatched = false;
	}

	for (int i = 0; i < currentTargetCount; i++) {
		if (!playerBlocks[i].exist) continue;
		if (playerBlocks[i].isNear(targets[i])) {
			playerBlocks[i].isMatched = true;
			// 정확한 위치에 스냅
			playerBlocks[i].x = targets[i].x;
			playerBlocks[i].y = targets[i].y;
			matchedCount++;
		}
	}

	gameComplete = (matchedCount == currentTargetCount);
	if (gameComplete) {
		printf("🎉 축하합니다! 모든 블록을 맞췄습니다! 🎉\n");
	}
}

void main(int argc, char** argv)
{
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

	// 초기화
	for (int i = 0; i < maxrectcount; i++) {
		targets[i].init();
		playerBlocks[i].init();
	}

	// 목표 패턴 생성
	generateTargetPattern();

	// 플레이어 블록들 생성
	for (int i = 0; i < currentTargetCount; i++) {
		playerBlocks[i].makePlayerRectangle(targets[i].w, targets[i].h, i);
	}

	printf("🎮 게임 시작!\n");
	printf("우측 블록들을 드래그해서 좌측 목표 위치에 맞춰보세요!\n");
	printf("r: 새 패턴, q: 종료\n");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(16, TimerFunction, 1);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	// 좌측 영역 (연한 파란색)
	glColor4f(0.8f, 0.9f, 1.0f, 0.3f);
	glRectf(-1.0f, -1.0f, 0.0f, 1.0f);

	// 우측 영역 (연한 빨간색)
	glColor4f(1.0f, 0.9f, 0.8f, 0.3f);
	glRectf(0.0f, -1.0f, 1.0f, 1.0f);


	// 목표 블록들 그리기 (좌측)
	for (int i = 0; i < currentTargetCount; i++) {
		if (targets[i].exist) {
			glColor4f(targets[i].color[0], targets[i].color[1], targets[i].color[2], targets[i].color[3]);
			glRectf(targets[i].x, targets[i].y, targets[i].x + targets[i].w, targets[i].y + targets[i].h);
		}
	}

	// 플레이어 블록들 그리기 (우측)
	for (int i = 0; i < currentTargetCount; i++) {
		if (playerBlocks[i].exist) {
			if (playerBlocks[i].isMatched) {
				glColor4f(0.0f, 0.9f, 0.0f, 0.9f); // 초록색
			}
			else {
				glColor4f(playerBlocks[i].color[0], playerBlocks[i].color[1], playerBlocks[i].color[2], playerBlocks[i].color[3]);
			}

			glRectf(playerBlocks[i].x, playerBlocks[i].y,
				playerBlocks[i].x + playerBlocks[i].w, playerBlocks[i].y + playerBlocks[i].h);
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
	case 'a': { // 자동 매칭
		if (!autoMatching) {
			startAutoMatching();
		}
		break;
	}
	case 'r': { // 새 게임
		printf("🔄 새로운 패턴 생성...\n");
		gameComplete = false;
		matchedCount = 0;
		autoMatching = false; // 자동 매칭 중단

		// 초기화
		for (int i = 0; i < maxrectcount; i++) {
			targets[i].init();
			playerBlocks[i].init();
		}

		// 새 패턴 생성
		generateTargetPattern();

		// 플레이어 블록 재생성
		for (int i = 0; i < currentTargetCount; i++) {
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
	// 애니메이션 업데이트
	for (int i = 0; i < currentTargetCount; i++) {
		playerBlocks[i].updateAnimation();
	}

	// 자동 매칭 로직
	if (autoMatching) {
		autoMatchTimer++;

		if (autoMatchTimer >= autoMatchDelay) {
			int nextBlock = findNextUnmatchedBlock();

			if (nextBlock != -1) {
				// 다음 블록을 목표 위치로 이동 시작
				playerBlocks[nextBlock].startMoveAnimation(targets[nextBlock].x, targets[nextBlock].y);
				printf("블록 %d 자동 이동 시작!\n", nextBlock);
				currentAutoMatchIndex = nextBlock + 1;
				autoMatchTimer = 0;
			}
			else {
				// 모든 블록이 매치되었거나 이동 중
				autoMatching = false;
				printf("자동 매칭 완료!\n");
			}
		}
	}
	checkMatching();
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1);
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float x_ndc = (1.0f * x / 400 - 1.0f);
	float y_ndc = -(1.0f * y / 300 - 1.0f);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		draggingRect = -1;
		for (int i = 0; i < currentTargetCount; i++) {
			if (playerBlocks[i].exist &&
				x_ndc >= playerBlocks[i].x && x_ndc <= playerBlocks[i].x + playerBlocks[i].w &&
				y_ndc >= playerBlocks[i].y && y_ndc <= playerBlocks[i].y + playerBlocks[i].h &&
				!playerBlocks[i].isMatched) {

				draggingRect = i;
				isDragging = true;
				lastX = x_ndc;
				lastY = y_ndc;
				printf("🔷 블록 %d 드래그 시작\n", i);
				break;
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (draggingRect != -1) {
			printf("🔷 블록 %d 드래그 종료\n", draggingRect);
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

		if (!playerBlocks[draggingRect].isMatched) {
			playerBlocks[draggingRect].x += deltaX;
			playerBlocks[draggingRect].y += deltaY;

			// 화면 경계 제한
			if (playerBlocks[draggingRect].x < -0.95f)
				playerBlocks[draggingRect].x = -0.95f;
			if (playerBlocks[draggingRect].x + playerBlocks[draggingRect].w > 0.95f)
				playerBlocks[draggingRect].x = 0.95f - playerBlocks[draggingRect].w;
			if (playerBlocks[draggingRect].y < -0.95f)
				playerBlocks[draggingRect].y = -0.95f;
			if (playerBlocks[draggingRect].y + playerBlocks[draggingRect].h > 0.85f)
				playerBlocks[draggingRect].y = 0.85f - playerBlocks[draggingRect].h;
		}

		lastX = x_ndc;
		lastY = y_ndc;
		glutPostRedisplay();
	}
}