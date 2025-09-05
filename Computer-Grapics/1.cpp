#include <iostream>
#include <random>
using namespace std;

//��� 4*4�� ���� ������ ä���
void makeprocession(int p[][4])
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			p[i][j] = rand() % 9 + 1;
		}
	}
}

void printprocession(int p[][4])
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cout << p[i][j] << " ";
		}
		cout << endl;
	}
}

void multiplyprocession(int p1[][4], int p2[][4])
{
	int result[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				result[i][j] += p1[i][k] * p2[k][j];
			}
		}
	}
	cout << "���� ���" << endl;
	printprocession(result);
}

void addprocession(int p1[][4], int p2[][4])
{
	int result[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = p1[i][j] + p2[i][j];
		}
	}
	cout << "���� ���" << endl;
	printprocession(result);
}

void subtractprocession(int p1[][4], int p2[][4])
{
	int result[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = p1[i][j] - p2[i][j];
		}
	}
	cout << "���� ���" << endl;
	printprocession(result);
}

void determinantprocession(int p[][4])
{
	int det = 0;
	for (int i = 0; i < 4; i++) {
		int temp[3][3];
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				temp[j][k] = p[j + 1][(i + k + 1) % 4];
			}
		}
		int subdet = 0;
		for (int j = 0; j < 3; j++) {
			subdet += temp[0][j] * (temp[1][(j + 1) % 3] * temp[2][(j + 2) % 3]
				- temp[1][(j + 2) % 3] * temp[2][(j + 1) % 3]);
		}
		det += p[0][i] * subdet * (i % 2 == 0 ? 1 : -1);
	}
	cout << "��Ľ��� ��: " << det << endl;
}

void findminmax(int p1[][4], int p2[][4], int& max, int& min)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (p1[i][j] > max) {
				max = p1[i][j];
			}
			if (p1[i][j] < min) {
				min = p1[i][j];
			}
			if (p2[i][j] > max) {
				max = p2[i][j];
			}
			if (p2[i][j] < min) {
				min = p2[i][j];
			}
		}
	}
}

void transposeprocession(int p[][4])
{
	int result[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[j][i] = p[i][j];
		}
	}
	cout << "��ġ��� ���" << endl;
	printprocession(result);
}

int main()
{
	srand((unsigned)time(NULL));

	int p1[4][4], p2[4][4];
	int max = 0, min = 0;
	bool submin = false, addmax = false;

	makeprocession(p1);
	makeprocession(p2);

	while (true) {
		char command;
		cout << "ù ��° ���"<<endl;
		printprocession(p1);
		cout << endl << "�� ��° ���" <<endl;
		printprocession(p2);
		cout << "m : ����, a : ����, d : ����, r : ��Ľ��� ��, t : ��ġ���, e : �ּҰ����� ����, f : �ִ밪 ���ϱ�, ���� : �Է��� ������ ����� ���, s : ��� �� �ٽ� ����, q : ����";
		cout << "��ɾ �Է��ϼ��� : ";
		cin >> command;
		if (command == 'm') {
			multiplyprocession(p1, p2);
		}
		else if (command == 'a') {
			addprocession(p1, p2);
		}
		else if (command == 'd') {
			subtractprocession(p1, p2);
		}
		else if (command == 'r') {
			cout << "ù ��° ����� ";
			determinantprocession(p1);
			cout << endl;
			cout << "�� ��° ����� ";
			determinantprocession(p2);
		}
		else if (command == 't') {
		}
		else if (command == 'e') {
		}
		else if (command == 'f') {
		}
		else if (command >= '0' && command <= '9') {
			int num = command - '0';
		}
		else if (command == 's') {
			makeprocession(p1);
			makeprocession(p2);
		}
		else if (command == 'q') {
			break;
		}
		else {
			cout << "�߸��� ��ɾ��Դϴ�." << endl;
		}
		system("pause");
		system("cls");
	}
}