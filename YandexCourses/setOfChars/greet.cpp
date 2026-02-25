#include <iostream>
#include <string>

using namespace std;

int main() {
    string name = "";
    cout << "Hello! What's your name, dear?" << endl;
    //cin >> name;
    getline(cin, name);
    cout << "Nice to meet you, " << name << "! :)" << endl;
}