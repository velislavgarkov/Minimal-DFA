#ifndef REGULAR_EXPRESSION
#define REGULAR_EXPRESSION

#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "alphabet.hpp"
#include "operations.hpp"
using namespace std;

struct ExprTree {
	struct Parent {
		int vert;
		bool dir;
	};
	
	vector <array<int, 2> > adj;
	vector <Parent> par;
	vector <char> type;
	vector <vector <int> > str;
	int start;
	
	void addVertex() {
		adj.push_back({-1, -1});
		par.push_back({-1, 0});
		type.push_back(0);
		str.push_back(vector <int> ());
	}
	
	string addBracketsAround(string &expr, char ch1 = 0, char ch2 = 0) {
		string ans="";
		stack <int> st;
		vector <int> pos;
		int lastClosed;
		for (int i = 0; i < (int)expr.size(); i++) {
			ans += expr[i];
			if (expr[i] == ch1 || expr[i] == ch2) {
				pos.push_back(lastClosed);
				ans += ')';
			} else if (expr[i] == '(') {
				st.push(ans.size() - 1);
			} else if (expr[i] == ')') {
				if (st.empty()) {
					cout << "Bracket at position " << i << " doesn't have a matching bracket!" << endl;
					cout << ans << endl;
					return "()";
				}
				lastClosed = st.top();
				st.pop();
			}
		}
		
		sort(pos.begin(), pos.end());
		
		swap(expr, ans);
		ans="";
		int j = 0;
		for (int i = 0; i < (int)expr.size(); i++) {
			while (j < (int)pos.size() && pos[j] == i) {
				ans += '(';
				j++;
			}
			ans += expr[i];
		}
		return ans;
	}
	
	string reverseExpr(string expr) {
		for (int i = 0; i < (int)expr.size() / 2; i++) {
			swap(expr[i], expr[expr.size() - 1 - i]);	
		}
		for (int i = 0; i < (int)expr.size(); i++) {
			if (expr[i] == '(') expr[i] = ')';
			else if (expr[i] == ')') expr[i] = '(';
		}
		return expr;
	}
	
	string convertExpr(string &expr, Sigma &sigma) {
		string ans="";
		for (int i = 0; i < (int)expr.size(); i++) {
			if (isCharacter(expr[i])) {
				ans += '(';
				while (i < (int)expr.size() && isCharacter(expr[i])) {
					ans += sigma.readChar(expr, i);
				}
				ans += ')';
				i--;
			} else ans += expr[i];
		}
		
		//cout << ans << endl;
		ans = addBracketsAround(ans, '*', '+');
		//ans = reverseExpr(ans);
		ans = addBracketsAround(ans, '~' /*, '!'*/);
		//ans = reverseExpr(ans);
		
		swap(ans, expr);
		ans = "";
		for (int i = 0; i < (int)expr.size(); i++) {
			ans += expr[i];
			if (expr[i] == ')' && i < (int)expr.size() - 1 && expr[i + 1] == '(') {
				ans += '.';
			}
		}
		return ans;
	}
	
	void buildFromExpr(string expr, Sigma& sigma) {
		expr = convertExpr(expr, sigma);
		//cout << "result " << expr << endl;
		int curNode = 0, cnt = 1;
		stack <int> st;
		st.push(0);
		start = 0;
		addVertex();
		
		for (int i = 0; i < (int)expr.size(); i++) {
			/*cout << expr[i] << ' ' << st.size() << ' ' << curNode << ' ' << cnt << ' ' << par[curNode].vert << ' ';
			if (!type[curNode]) cout << 0;
			else cout << type[curNode];
			cout << ' ' << start << endl;*/
			
			if (expr[i] == '(') {
				addVertex();

				par[cnt].vert = curNode;				
				if (adj[curNode][0] == -1) {
					adj[curNode][0] = cnt;
					par[cnt].dir = 0;
				} else {
					adj[curNode][1] = cnt;
					par[cnt].dir = 1;
				}
				
				curNode = cnt;
				st.push(cnt++);
				
			} else if (expr[i] == ')') {
				st.pop();
				if (!st.empty()) curNode = st.top();
				else {
					cout << "at symbol " << i << "is an extra ')'" << endl;
					assert((1 == 0));
					return;
				}
			} else if (expr[i] == '|' || expr[i] == '.' || expr[i] == '&' || expr[i] == '-') {
				if (type[curNode] != 0) {
					addVertex();
					
					par[cnt] = par[curNode];
					if (par[curNode].vert != -1) {
						adj[par[curNode].vert][par[curNode].dir] = cnt;
					} else {
						start = cnt;
						par[curNode] = {cnt, 0};
					}
					
					adj[cnt][0] = curNode;
					curNode = cnt++;
					st.pop();
					st.push(curNode);
				}
				type[curNode] = expr[i];
				
			} else if (expr[i] == '~' /*|| expr[i] == '!'*/ || expr[i] == '*' || expr[i] == '+') {
				type[curNode] = expr[i];
			} else {
				while (i < (int)expr.size() && isCharacter(expr[i]) == 1) {
					str[curNode].push_back(sigma.toAutCharacter(sigma.readChar(expr, i)));
				}
				i--;
			}
		}
	}
	
	void print() {
		cout << adj.size() << endl;
		for (int i = 0; i < (int)adj.size(); i++) {
			cout << "node " << i << " type ";
			if (type[i] == 0) cout << 0;
			else cout << type[i];
			cout << " left child " << adj[i][0] << " right child " << adj[i][1] << endl;
		}
		cout << start << endl;
	}
	
	int size() { return adj.size(); }
};

Automata traverseExprTree(ExprTree &exTree, Sigma &sigma) {
	int start = exTree.start;
	stack <int> st;
	st.push(start);
	
	vector <Automata> aut;
	aut.reserve(exTree.size());
	for (int i = 0; i < (int)exTree.size(); i++) aut.emplace_back(sigma.size);
	vector <int> ptrChild(exTree.size());
	
	while (!st.empty()) {
		int node = st.top();
		if (exTree.adj[node][0] == -1) {
			AutomataWord(aut[node], exTree.str[node]);
			st.pop();
			continue;	
		}
		
		if (ptrChild[node] == 0) {
			st.push(exTree.adj[node][0]);
			ptrChild[node] = 1;
		} else if (ptrChild[node] == 1 && exTree.adj[node][1] != -1) {
			st.push(exTree.adj[node][1]);
			ptrChild[node] = 2;
		} else {
			bool curFl;
			int l = exTree.adj[node][0], r = exTree.adj[node][1];
			switch (exTree.type[node]) {
				case '|' :
					assert(l != -1 && r != -1);
					Union(aut[node], aut[l], aut[r]);
					break;
				case '.' :
					assert(l != -1 && r != -1);
					Concat(aut[node], aut[l], aut[r]);
					break;
				case '*' :
					assert(l != -1); 
					Star(aut[node], aut[l]);
					break;
				case '+' :
					assert(l != -1);
					curFl = aut[l].flag;
					Star(aut[node], aut[l]);
					aut[node].flag = curFl;
					break;
				case '~' :
					assert(l != -1);
					Reverse(aut[node], aut[l]);
					break;
				/*case '!' :
					assert(l != -1);
					swap(aut[node], aut[l]);
					aut[node].Complement();
					break;*/
				case '&' :
					assert(l != -1 && r != -1);
					Intersect(aut[node], aut[l], aut[r]);
					break;
				case '-' :
					assert(l != -1 && r != -1);
					Subtract(aut[node], aut[l], aut[r]);
					break;
				case 0 : 
					assert(l != -1);
					swap(aut[node], aut[l]);
					break;
			}
			/*if (exTree.type[node] != 0) {
				cout << "finished node " << node << " with type " << exTree.type[node] << " and its automata is:" << endl;
				aut[node].print(sigma);
			}*/
			st.pop();
		}
	}
	
	return aut[start];
}

#endif
