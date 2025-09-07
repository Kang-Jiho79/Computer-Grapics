#include <iostream>
using namespace std;

struct Dot
{
	int x, y, z;
	bool exist;
};

void addDot(Dot dot[], int& dot_count, int x, int y, int z)
{
	if (dot_count >= 10) {
		cout << "더 이상 점을 추가할 수 없습니다." << endl;
		return;
	}
	for (int i = 0; i < 10; i++) {
		if (!dot[i].exist) {
			dot[i].x = x;
			dot[i].y = y;
			dot[i].z = z;
			dot[i].exist = true;
			dot_count++;
			return;
		}
	}
}

void removeDot(Dot dot[], int& dot_count)
{
	if (dot_count <= 0) {
		cout << "제거할 점이 없습니다." << endl;
		return;
	}
	for (int i = 9; i >= 0; i--) {
		if(dot[i].exist) {
			dot[i].exist = false;
			dot_count--;
			return;
		}
	}
}

void addbottomDot(Dot dot[], int& dot_count, int x, int y, int z)
{
	if (dot_count >= 10) {
		cout << "더 이상 점을 추가할 수 없습니다." << endl;
		return;
	}
	for (int i = 8; i >= 0; i--) {
		if (dot[i].exist && !dot[i+1].exist) {
			dot[i+1].x = dot[i].x;
			dot[i+1].y = dot[i].y;
			dot[i+1].z = dot[i].z;
			dot[i+1].exist = true;
			dot[i].exist = false;
		}
	}
	dot[0].x = x;
	dot[0].y = y;
	dot[0].z = z;
	dot[0].exist = true;
	dot_count++;
}

void removebottomDot(Dot dot[], int& dot_count)
{
	if (dot_count <= 0) {
		cout << "제거할 점이 없습니다." << endl;
		return;
	}
	for (int i = 0; i < 9; i++) {
		if (dot[i].exist) {
			dot[i].exist = false;
			dot_count--;
			return;
		}
	}
}

void moveDot(Dot dot[], int dot_count)
{
	Dot temp = dot[0];
	for (int i = 1; i < 10; ++i) {
		dot[i - 1] = dot[i];
	}
	dot[9] = temp;
}

void clearDot(Dot dot[], int& dot_count)
{
	for (int i = 0; i < 10; i++) {
		dot[i].exist = false;
		dot[i].x = -1;
		dot[i].y = -1;
		dot[i].z = -1;
	}
	dot_count = 0;
}

void downDot(Dot dot[], int dot_count)
{
	Dot temp[10];
	int idx = 0;
	for (int i = 0; i < 10; i++) {
		if (dot[i].exist) {
			temp[idx++] = dot[i];
		}
	}
	for (int i = 0; i < 10; i++) {
		if (temp[i].exist == true) {
			dot[i] = temp[i];
		}
		else {
			dot[i].exist = false;
		}
	}
}

void sortbydistance(Dot dot[], int dot_count)
{
	downDot(dot, dot_count);
	for (int i = 0; i < dot_count-1; i++) {
		for (int j = 0; j < dot_count - 1; j++) {
			int dist1 = dot[j].x * dot[j].x + dot[j].y * dot[j].y + dot[j].z * dot[j].z;
			int dist2 = dot[j + 1].x * dot[j + 1].x + dot[j + 1].y * dot[j + 1].y + dot[j + 1].z * dot[j + 1].z;
			if (dist1 > dist2) {
				Dot temp = dot[j];
				dot[j] = dot[j + 1];
				dot[j + 1] = temp;
			}
		}
	}
}

int main()
{
	Dot dot[10];
	for (int i = 0; i < 10; i++) {
		dot[i].exist = false;
	}
	int dot_count = 0;
	while (true) {
		char command;
		for (int i = 9; i >= 0; i--) {
			cout << i;
			if (dot[i].exist) {
				cout << " : (" << dot[i].x << ", " << dot[i].y << ", " << dot[i].z << ")" << endl;
			}
			else {
				cout << " : ( , , )" << endl;
			}
		}
		cout << "명령어 : ";
		cin >> command;
		if (command == '+') {
			cout << "좌표입력 : ";
			int x, y, z;
			cin >> x >> y >> z;
			addDot(dot, dot_count, x, y, z);
		}
		else if (command == '-') {
			removeDot(dot, dot_count);
		}
		else if (command == 'e') {
			cout << "좌표입력 : ";
			int x, y, z;
			cin >> x >> y >> z;
			addbottomDot(dot, dot_count, x, y, z);
		}
		else if (command == 'd') {
			removebottomDot(dot, dot_count);
		}
		else if (command == 'a') {
			cout << "점 갯수 : " << dot_count << endl;
		}
		else if (command == 'b') {
			moveDot(dot, dot_count);
		}
		else if (command == 'c') {
			clearDot(dot, dot_count);
		}
		else if (command == 'f') {
			sortbydistance(dot, dot_count);
		}
		else if (command == 'q') {
			break;
		}
		else {
			cout << "잘못된 명령어입니다." << endl;
		}
		system("pause");
		system("cls");
	}
}