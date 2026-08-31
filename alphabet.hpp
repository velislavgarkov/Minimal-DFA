#ifndef ALPHABET
#define ALPHABET

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

int utf8_len(unsigned char lead) {
    if ((lead & 0x80) == 0x00) return 1; // 0xxxxxxx  -> ASCII
    if ((lead & 0xE0) == 0xC0) return 2; // 110xxxxx
    if ((lead & 0xF0) == 0xE0) return 3; // 1110xxxx
    if ((lead & 0xF8) == 0xF0) return 4; // 11110xxx
    return 1;
}

vector <char> specialCharacters = {'(', ')', '|', '.', '*', '&', '!', '~', '-', '+'};
bool isCharacter(unsigned char ch) {
	if (utf8_len(ch) != 1) return true;
	
	for (auto i : specialCharacters) {
		if (ch == i) return false;
	}
	return true;
}

struct Character {
	string x;
	Character (string c) : x(c) {}
};
	
struct Sigma {
	unordered_map <string, int> characterNum;
	unordered_map <int, string> fromNumToChar;
	int size = 0;

	string readChar(string &expr, int &i) {
		unsigned char lead = expr[i];
		int len = utf8_len(lead);
		
		string res;
		for (int j = 0; j < len && i < (int)expr.size(); j++) res += expr[i++];
		return res;
	}

	int toAutCharacter(Character ch) {
		auto it = characterNum.find(ch.x);
		int res;
		if (it == characterNum.end()) {
			//cout << "Sigma " << ch.x << ' ' << sigmaSize << endl;
			res = characterNum[ch.x] = size;
			fromNumToChar[size] = ch.x;
			size++;
		}
		else res = it->second;
		return res;
	}
	string toChar(int t) { return fromNumToChar[t]; }
};

vector <int> readWord(string &expr, Sigma &sigma) {
	vector <int> res;
	int i = 0;
	while (i < (int)expr.size()) {
		res.push_back(sigma.toAutCharacter(sigma.readChar(expr, i)));
	}
	return res;
}
#endif
