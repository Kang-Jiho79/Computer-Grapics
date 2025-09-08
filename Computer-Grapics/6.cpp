#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <conio.h>
using namespace std;

#define MaxBoardSize 30

struct Board {
	int value;
	int type; // 0 : empty, 1 : player, 2 : obstacle
};

void setting(Board board[MaxBoardSize][MaxBoardSize]) {
	srand((unsigned int)time(0));
	int num_obstacles = 0;
	for (int i = 0; i < MaxBoardSize; ++i) {
		for (int j = 0; j < MaxBoardSize; ++j) {
			board[i][j].value = 0;
			board[i][j].type = 0;
		}
	}
	board[0][0].type = 1;
	while (true) {
		int obsx = rand() % MaxBoardSize;
		int obsy = rand() % MaxBoardSize;
		if (board[obsy][obsx].value == 0) {
			board[obsy][obsx].value = -1;
			board[obsy][obsx].type = 2;
			num_obstacles++;
		}
		if (num_obstacles == 4) break;
	}
}

void printboard(Board board[MaxBoardSize][MaxBoardSize], int boardWidth, int boardHeight) {
	for (int i = 0; i < boardHeight; ++i) {
		for (int j = 0; j < boardWidth; ++j) {
			if (board[i][j].type == 1) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
				cout << "P ";
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else if (board[i][j].type == 2) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
				cout << "X ";
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else if (board[i][j].value > 0) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 1);
				cout << board[i][j].value << " ";
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else {
				cout << "0 ";
			}
		}
		cout << endl;
	}
}

void moveplayer (Board board[MaxBoardSize][MaxBoardSize], int x, int y, int playerx, int playery) {
	int newx = playerx + x;
	int newy = playery + y;
	if (newx < 0 || newx >= MaxBoardSize || newy < 0 || newy >= MaxBoardSize) {
		return;
	}
	if (board[newy][newx].type == 2) {
		return;
	}
	board[playery][playerx].type = 0;
	board[playery][playerx].value++;
	board[newy][newx].type = 1;
}

int main()
{
	Board board[MaxBoardSize][MaxBoardSize];
	int playerx = 0, playery = 0;
	setting(board);
	while (true) {
		char command;
		printboard(board, MaxBoardSize, MaxBoardSize);
		cout << "명령어 입력 (w/a/s/d : 이동, q : 종료) : ";
		command = _getch();
		if (command == 'q') break;
		else if (command == 'w') moveplayer(board, 0, -1, playerx, playery);
		else if (command == 'a') moveplayer(board, -1, 0, playerx, playery);
		else if (command == 's') moveplayer(board, 0, 1, playerx, playery);
		else if (command == 'd') moveplayer(board, 1, 0, playerx, playery);
		else if (command == '\n') {
			setting(board);
			playerx = 0, playery = 0;
		}
	}
}