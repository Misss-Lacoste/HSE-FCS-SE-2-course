#include <iostream>
#include <cmath>

using namespace std;

int main() {
    char one;
    cin >> one;
    char two;
    cin >> two;
    char medium = char (((int(two) + int(one)) / 2));
    int distance = abs(int (two) - int (one));
    cout << "Начальный символ: " << one << endl;
    cout << "Конечный символ: " << two << endl;
    cout << "---" << endl;
    cout << "Центральный символ: " << medium << endl;
    cout << "Дистанция: " << distance << endl;
}