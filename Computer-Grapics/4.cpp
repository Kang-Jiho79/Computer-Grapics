#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <windows.h>
using namespace std;

struct Board {
    char value;
    bool matched;
};

void fillBoard(Board board[5][5], int& count, int& score) {
    char pool[25];
    int idx = 0;
    count = 20;
	score = 0;

    for (char c = 'a'; c <= 'l'; ++c) {
        pool[idx++] = c;
        pool[idx++] = c;
    }
    pool[idx++] = '@';

    srand((unsigned int)time(0));
    for (int i = 24; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(pool[i], pool[j]);
    }

    // 2차원 배열에 배치
    idx = 0;
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j) {
            board[i][j].value = pool[idx++];
			board[i][j].matched = false;
        }
}

void printBoard(Board board[5][5]) {
	cout << "  " << "a b c d e" << endl;
    for (int i = 0; i < 5; ++i) {
		cout << i + 1 << ' ';
        for (int j = 0; j < 5; ++j) {
            if (board[i][j].matched) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
                cout << board[i][j].value << ' ';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
			else
                cout << "+ ";
        }
        cout << endl;
    }
}

void hintBoard(Board board[5][5]) {
    cout << "  " << "a b c d e" << endl;
    for (int i = 0; i < 5; ++i) {
        cout << i + 1 << ' ';
        for (int j = 0; j < 5; ++j) {
            cout << board[i][j].value << ' ';
        }
        cout << endl;
    }
}

void checkMatch(Board board[5][5], char pos1[],char pos2[],int& score) {
	int x1 = pos1[0] - 'a';
	int y1 = pos1[1] - '1';
	int x2 = pos2[0] - 'a';
	int y2 = pos2[1] - '1';
    if (board[y1][x1].value == board[y2][x2].value) {
        board[y1][x1].matched = true;
		board[y1][x1].value = toupper(board[y1][x1].value);
        board[y2][x2].matched = true;
        board[y2][x2].value = toupper(board[y2][x2].value);
        score++;
    }
    else if (board[y1][x1].value == '@') {
        board[y1][x1].matched = true;
		board[y2][x2].matched = true;
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 5; ++j)
                if (board[i][j].value == board[y2][x2].value && (i != y2 && j != x2)) {
                    board[i][j].matched = true;
                    board[i][j].value = toupper(board[i][j].value);
                    board[y2][x2].value = toupper(board[y2][x2].value);
                }
        score++;
    }
    else if (board[y2][x2].value == '@') {
        board[y1][x1].matched = true;
        board[y2][x2].matched = true;
        for (int i = 0; i < 5; ++i)
            for (int j = 0; j < 5; ++j)
                if (board[i][j].value == board[y1][x1].value) {
                    board[i][j].matched = true;
                    board[i][j].value = toupper(board[i][j].value);
                    board[y1][x1].value = toupper(board[y1][x1].value);
                }
        score++;
    }
}

void checkBoard(Board board[5][5], char pos1[], char pos2[],int& count,int& score) {
    int x1 = pos1[0] - 'a';
    int y1 = pos1[1] - '1';
    int x2 = pos2[0] - 'a';
    int y2 = pos2[1] - '1';
    if (x1 < 0 || x1 >= 5 || y1 < 0 || y1 >= 5 || x2 < 0 || x2 >= 5 || y2 < 0 || y2 >= 5) {
        cout << "잘못된 좌표입니다." << endl;
        return;
	}
    cout << "  " << "a b c d e" << endl;
    for (int i = 0; i < 5; ++i) {
        cout << i + 1 << ' ';
        for (int j = 0; j < 5; ++j) {
            if (board[i][j].matched) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
                cout << board[i][j].value << ' ';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
            else if(i == y1 && j == x1) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
                cout << board[i][j].value << ' ';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }
            else if (i == y2 && j == x2) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
                cout << board[i][j].value << ' ';
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
            else
                cout << "+ ";
        }
        cout << endl;
    }
	checkMatch(board, pos1, pos2, score);
    count--;
}

int main() {
	Board board[5][5];
    int count = 0, score = 0;
    fillBoard(board, count, score);
    while (true) {
		string command;
		cout << "남은 기회 : " << count << ", 점수 : " << score << endl;
        printBoard(board);
		cout << "명령어 : ";
		cin >> command;
		if (command == "q") break;
        else if (command == "c") {
            string s1, s2;
            cout << "좌표입력 : ";
            cin >> s1 >> s2;
            if (s1.size() != 2 || s2.size() != 2 ||
                s1[0] < 'a' || s1[0] > 'e' || s1[1] < '1' || s1[1] > '5' ||
                s2[0] < 'a' || s2[0] > 'e' || s2[1] < '1' || s2[1] > '5') {
                cout << "잘못된 좌표입니다." << endl;
                continue;
            }
            char pos1[2] = { s1[0], s1[1] };
            char pos2[2] = { s2[0], s2[1] };
            checkBoard(board, pos1, pos2, count, score);
        }
        else if (command == "r") {
            fillBoard(board, count, score);
        }
        else if (command == "h") {
			hintBoard(board);
        }
        else {
			cout << "잘못된 명령어입니다." << endl;
        }
        if (count == 0) {
            printBoard(board);
            cout << "게임 종료!" << endl;
            break;
		}
		system("pause");
		system("cls");
    }
	return 0;
}
