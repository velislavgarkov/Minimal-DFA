#include <iostream>
#include <string>
using namespace std;

void convert(string &str) {
	for (int i = 0; i < (int)str.size(); i++) {
		if (str[i] == '\\') str[i] = '-';
		//else if (str[i] == '+') str[i] = '|';
	}
}
