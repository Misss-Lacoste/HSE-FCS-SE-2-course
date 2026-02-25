#include <iostream>
#include <string>

using namespace std;

int main() {
    string name ="";
    getline(cin, name);
    cout << "Активация... Профиль создан." << endl;
    cout << "Добро пожаловать, " << name << "!" << endl;
    //string alias = "";
    int length = name.length();
	string alias1, alias2, alias3;
    alias1 += name[0];
    alias2 += name[length -1];
    alias3 += to_string(length);
    cout << "Ваш личный позывной: " << alias1  << "-" << alias2 << "-" << alias3 << endl;
}