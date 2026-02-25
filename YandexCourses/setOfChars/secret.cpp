#include <iostream>
#include <string>

using namespace std;

int main() {
    string word = "", secret = "";
    int length;
    cout << "Enter any word: " << endl;
    cin >> word;
    length = word.length();
    cout << "The length jf the word is - " << length << endl;
    secret += word[0];
    secret += word[length -1];
    secret += word[length/2];
    cout << "Encrypted word: " << secret << endl;
}