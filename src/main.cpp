#include <iostream>
#include <fstream>
#include "regularExpression.hpp"
#include "translate.cpp"
#include <time.h>
using namespace std;

int main () {
	clock_t tStart = clock();
	string expr, s;
	
	while (getline(cin, s)) expr += s;
	//getline(cin, expr);
	convert(expr);
	
	ExprTree exTree;
	Sigma sigma;
	exTree.buildFromExpr(expr, sigma);
	Automata aut = traverseExprTree(exTree, sigma);

	cout << "finished expression" << endl;
	cout << "sigmasize is : " << sigma.size << endl;
	aut.Determinize();
	aut.Minimize();
	aut.MakeTraversable();
	
	aut.print(sigma);
	
	printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
	
	/*
	do {
		cout << "Input a word: ";
		string word;
		
		if (getline(cin, word)) {
			if (aut.Traverse(readWord(word, sigma))) cout << "The word is in the language." << endl;
			else cout << "The word is NOT in the language." << endl;
		} else break;
	} while (true);*/
}
