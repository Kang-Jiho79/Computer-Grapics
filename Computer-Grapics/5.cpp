#include <iostream>
#include <Windows.h>
using namespace std;

#define MaxBoardSize 40

struct Board {
	int value; // 0 : empty, 1 : X, 2 : O, 3 : #
};

struct Rect {
	int x, y;
	int width, height;
};

void checkboard(Board board[MaxBoardSize][MaxBoardSize], Rect rect1, Rect rect2, int boardWidth, int boardHeight) {
	for (int i = 0; i < boardHeight; ++i) {
		for (int j = 0; j < boardWidth; ++j) {
			board[i][j].value = 0;
		}
	}
	for (int i = rect1.x; i < rect1.x + rect1.width; ++i) {
		for (int j = rect1.y; j < rect1.y + rect1.height; ++j) {
			int x = (boardWidth + i) % boardWidth;
			int y = (boardHeight + j) % boardHeight;
			board[y][x].value += 1;
		}
	}
	for (int i = rect2.x; i < rect2.x + rect2.width; ++i) {
		for (int j = rect2.y; j < rect2.y + rect2.height; ++j) {
			int x = (boardWidth + i) % boardWidth;
			int y = (boardHeight + j) % boardHeight;
			board[y][x].value += 2;
		}
	}
}

void printboard(Board board[MaxBoardSize][MaxBoardSize], Rect rect1, Rect rect2, int boardWidth, int boardHeight) {
	checkboard(board, rect1, rect2, boardWidth, boardHeight);
	for (int i = 0; i < boardHeight; ++i) {
		for (int j = 0; j < boardWidth; ++j) {
			if (board[i][j].value == 0) cout << ". ";
			else if (board[i][j].value == 1) cout << "X ";
			else if (board[i][j].value == 2) cout << "O ";
			else if (board[i][j].value == 3) { 
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
				cout << "# "; 
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
		}
		cout << endl;
	}
}

void setrect(Rect* rect) {
	int tempx1, tempy1;
	int tempx2, tempy2;
	while (true) {
		cout << "좌측 상단 좌표 입력 : ";
		cin >> tempx1 >> tempy1;
		cout << "우측 하단 좌표 입력 : ";
		cin >> tempx2 >> tempy2;
		if (tempx1 > tempx2 || tempy1 > tempy2) {
			cout << "잘못된 좌표입니다. 다시 입력하세요." << endl;
			continue;
		}
		else break;
	}
	rect->x = tempx1;
	rect->y = tempy1;
	rect->width = tempx2 - tempx1 + 1;
	rect->height = tempy2 - tempy1 + 1;
}

int main() {
	Board board[MaxBoardSize][MaxBoardSize];
	Rect rect1, rect2;
	int tempx, tempy;
	int boardWidth = 30, boardHeight = 30;

	cout << "첫번째 사각형" << endl;
	setrect(&rect1);
	cout << "두번째 사각형" << endl;
	setrect(&rect2);
	system("cls");
	while (true) {
		string command;
		printboard(board, rect1, rect2, boardWidth, boardHeight);
		cout << "명령어 입력 : ";
		cin >> command;
		if (command == "q") break;
		else if (command == "1" || command == "2") {
			Rect* rect = (command == "1") ? &rect1 : &rect2;
			string shapecommand;
			cout << "도형 명령어 입력 : ";
			cin >> shapecommand;
			if (shapecommand == "x") {
				rect->x = (rect->x - 1);
			}
			else if (shapecommand == "X") {
				rect->x = (rect->x + 1);
			}
			else if (shapecommand == "y") {
				rect->y = (rect->y - 1);
			}
			else if (shapecommand == "Y") {
				rect->y = (rect->y + 1);
			}
			else if (shapecommand == "s") {
				if (rect->width > 1)
					rect->width = rect->width - 1;
				if (rect->height > 1)
					rect->height = rect->height - 1;
			}
			else if (shapecommand == "S") {
				if (rect->width < boardWidth)
					rect->width = rect->width + 1;
				if (rect->height < boardHeight)
					rect->height = rect->height + 1;
			}
			else if (shapecommand == "i") {
				if (rect->width > 1)
					rect->width = rect->width - 1;
			}
			else if (shapecommand == "I") {
				if (rect->width < boardWidth)
					rect->width = rect->width + 1;
			}
			else if (shapecommand == "j") {
				if (rect->height > 1)
					rect->height = rect->height - 1;
			}
			else if (shapecommand == "J") {
				if (rect->height < boardHeight)
					rect->height = rect->height + 1;
			}
			else if (shapecommand == "a") {
				if (rect->width < boardWidth)
					rect->width = rect->width + 1;
				if (rect->height > 1)
					rect->height = rect->height - 1;
			}
			else if (shapecommand == "A") {
				if (rect->width > 1)
					rect->width = rect->width - 1;
				if (rect->height < boardHeight)
					rect->height = rect->height + 1;
			}
			else if (shapecommand == "b") {
				cout << "면적 : " << rect->width * rect->height << endl;
				system("pause");
			}
			else {
				cout << "잘못된 명령어입니다." << endl;
				system("pause");
			}
		}
		else if (command == "c") {
			if (boardHeight >= MaxBoardSize || boardWidth >= MaxBoardSize) {
				cout << "더 이상 보드 크기를 늘릴 수 없습니다." << endl;
				system("pause");
				continue;
			}
			boardHeight = boardHeight + 1;
			boardWidth = boardWidth + 1;
		}
		else if (command == "d") {
			if (boardHeight <= 1 || boardWidth <= 1) {
				cout << "더 이상 보드 크기를 줄일 수 없습니다." << endl;
				system("pause");
				continue;
			}
			boardHeight = boardHeight - 1;
			boardWidth = boardWidth - 1;

		}
		else if (command == "r") {
			cout << "첫번째 사각형" << endl;
			setrect(&rect1);
			cout << "두번째 사각형" << endl;
			setrect(&rect2);
		}
		else {
			cout << "잘못된 명령어입니다." << endl;
			system("pause");
		}
		system("cls");
	}
}