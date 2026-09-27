#ifndef AUTOMATA
#define AUTOMATA

#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <array>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>
#include "alphabet.hpp"
#include "MinimizeTools.hpp"
using namespace std;
using Clock = chrono::high_resolution_clock;

struct Transition{
	int dest;
	int ch;
};
struct TempTable{
	int p;
	int ch;
};

void countSortTrans(vector <FullTransition>& tr, int type, int k) {
	vector <int> cnt(k);
	for (auto i : tr) cnt[i.t[type]]++;
	for (int i = 1; i < k; i++) cnt[i] = cnt[i - 1] + cnt[i];
	
	vector <FullTransition> temp(tr.size());
	for (int i = tr.size() - 1; i >= 0; i--) {
		auto cur = tr[i];
		temp[--cnt[cur.t[type]]] = cur;
	}
	swap(temp, tr);
}

struct StateHash {
	struct VectorHash {
		size_t operator()(const vector<int>& v) const {
		    size_t seed = 0;
		    for (int x : v) {
		        seed ^= hash<int>{}(x) + 0x9e3779b97f4a7c15ULL
		                + (seed << 6)
		                + (seed >> 2);
		    }

		    return seed;
		}
	};

	unordered_map <vector <int> , int, VectorHash> SetToId;
	vector <vector <int> > s;
	bool type;
	
	void add(vector <int> &v) {
		SetToId[v] = s.size();
		s.push_back(v);
	}
	int findVector(vector <int> &v) {
		auto it = SetToId.find(v);
		if (it == SetToId.end()) return -1;
		return it->second;
	}
};

struct Automata {
	vector <unordered_map <int, int> > transUM;
	vector <vector <Transition> > trans;
	vector <int> init;
	vector <bool> isF;
	vector <TempTable> t;
	int sigmaSize;
	bool flag = false;
	bool det = false;
	
	Automata(int sigmaSz) : sigmaSize(sigmaSz) {}
	
	int size() { return trans.size() + init.size() + isF.size() + t.size(); }
	int numStates() { return trans.size(); }
	void shift(int n) {
		for (int i = 0; i < (int)init.size(); i++) init[i] += n;
		for (auto &v : trans) {
			for (int i = 0; i < (int)v.size(); i++) {
				v[i].dest += n;
			}
		}
		for (int i = 0; i < (int)t.size(); i++) t[i].p += n;
	}
	void addNewNode() {
		trans.push_back(vector <Transition> ());
		isF.push_back(false);
	}
	void clear() {
		trans.clear();
		init.clear();
		isF.clear();
		t.clear();
	}
	
	void Determinize() {
		if (det) return;
		
		if (flag) {
			int newNode = isF.size();
			init.push_back(newNode);
			trans.push_back(vector <Transition> ());
			isF.push_back(true);
		}
		
		vector <vector <Transition> > tempTrans;
		vector <TempTable> tempT;
		vector <bool> tempIsF;
		
		vector <int> realCh(sigmaSize);
		vector <int> chIdx(sigmaSize);
		vector <unordered_set <int> > chars(trans.size());
		
		int n = numStates();
		StateHash h;
		h.SetToId.reserve(200000);
		h.s.reserve(200000);
		queue <int> q;
		q.push(0);
		
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < (int)trans[i].size(); j++) {
				int c = trans[i][j].ch;
				chars[i].insert(c);
			}
		}
		
		tempTrans.push_back(vector <Transition> ());
		tempIsF.push_back(false);
		for (auto i : init) {
			if (isF[i]) {
				tempIsF[0] = true;
				break;
			}
		}
		
		sort(init.begin(), init.end());
		h.add(init);
		init.clear(); init.push_back(0);

		while (!q.empty()) {
			auto p = q.front(); q.pop();
			
			set<int> chT;
			for (auto curS : h.s[p]) {
				for (auto ch : chars[curS]) chT.insert(ch);
			}
			
			int br = 0;
			for (auto ch : chT) {
				realCh[br] = ch;
				chIdx[ch] = br++;
			}
			
			vector<vector<int> > newS(br);
			vector<bool> curF(br);
			for (auto curS : h.s[p]) {
				for (auto [dest, ch] : trans[curS]) {
					newS[chIdx[ch]].push_back(dest);
					curF[chIdx[ch]] = curF[chIdx[ch]] || isF[dest];
				}
			}
			
			for (int a = 0; a < br; a++) {
				sort(newS[a].begin(), newS[a].end());
				auto it = unique(newS[a].begin(), newS[a].end());
				newS[a].erase(it, newS[a].end());
				
				int hashNum = h.findVector(newS[a]);
				if (hashNum == -1) {
				    h.add(newS[a]);
				    hashNum = h.s.size() - 1;
				    q.push(hashNum);
				    
				    tempTrans.push_back(vector<Transition>());
				    tempIsF.push_back(curF[a]);
				}
				
				tempTrans[p].push_back({hashNum, realCh[a]});
				if (curF[a]) tempT.push_back({p, realCh[a]});
			}
		}
		
		swap(trans, tempTrans);
		swap(isF, tempIsF);
		swap(t, tempT);
		det = true;
	}
	
	void addDummyState() {
		int n = numStates();
		vector <Transition> temp(sigmaSize);
		for (int i = 0; i < n; i++) {
			for (int c = 0; c < sigmaSize; c++) temp[c] = {n, c};
			for (auto t : trans[i]) temp[t.ch] = t;
			trans[i] = temp;
		}
		
		addNewNode();
		trans[n].resize(sigmaSize);
		for (int i = 0; i < sigmaSize; i++) {
			trans[n][i] = {n, i};
		}
	}
	
	void Complement() {
		Determinize();
		addDummyState();
		
		for (int i = 0; i < numStates(); i++) isF[i] = !isF[i];
		flag = !flag;
	}
	
	void Trim() {
		queue <int> q;
		vector <bool> reachableI(numStates());
		vector <bool> reachableF(numStates());
		vector <vector <int> > opDir(numStates());
		for (auto i : init) {
			q.push(i);
			reachableI[i] = true;
		}
		
		while (!q.empty()) {
			int x = q.front();
			q.pop();
			for (int i = 0; i < (int)trans[x].size(); i++) {
				int dest = trans[x][i].dest;
				opDir[dest].push_back(x);
				if (!reachableI[dest]) {
					reachableI[dest] = true;
					q.push(dest);
				}
			}
		}
		
		for (int i = 0; i < numStates(); i++) {
			if (reachableI[i] && isF[i]) {
				q.push(i);
				reachableF[i] = true;
			}
		}
		
		while (!q.empty()) {
			int x = q.front();
			q.pop();
			for (auto i : opDir[x]) {
				if (!reachableF[i]) {
					q.push(i);
					reachableF[i] = true;
				}
			}
		}
		vector <int> newIdx(numStates());
		int cnt = 0;
		for (int i = 0; i < numStates(); i++) {
			if (reachableI[i] && reachableF[i]) newIdx[i] = cnt++;
			else newIdx[i] = -1;
		}
		
		vector <vector <Transition> > tempTrans(cnt);
		vector <TempTable> tempT;
		vector <bool> tempIsF(cnt);
		vector <int> tempI;
		for (auto i : init) {
			if (newIdx[i] != -1) tempI.push_back(newIdx[i]);
		}
		swap(init, tempI);
		
		for (int i = 0; i < numStates(); i++) {
			if (newIdx[i] == -1) continue;
			if (isF[i]) tempIsF[newIdx[i]] = true;
			
			for (auto [dest, ch] : trans[i]) {
				if (newIdx[dest] != -1) {
					tempTrans[newIdx[i]].push_back({newIdx[dest], ch});
					if (isF[dest]) tempT.push_back({newIdx[i], ch});
				}
			}
		}
		swap(tempIsF, isF);
		swap(tempTrans, trans);
		swap(t, tempT);
	}
	
	void Minimize() {
		Determinize();
		Trim();
		
		bool EmptyEps = true;
		for (int i = 0; i < numStates(); i++) {
			if (!trans[i].empty()) {
				EmptyEps = false;
				break;
			}
		}
		//cout << "EmptyEps " << EmptyEps << endl;
		if (EmptyEps) {
			if (numStates() == 0) {
				init.push_back(0);
				trans.push_back(vector <Transition> ());
				isF.push_back(false);
			}
			return;
		}
		
		vector <FullTransition> tr;
		tr.reserve(trans.size());
		for (int i = 0; i < numStates(); i++) {
			for (auto [dest, ch] : trans[i]) tr.push_back({i, ch, dest});
		}
		countSortTrans(tr, 1, sigmaSize);
		countSortTrans(tr, 2, trans.size());
		
		MinimizeTools m(isF, tr, sigmaSize);
		int curStep = 0;
		while (curStep < m.realSz) m.Step(tr, curStep++);
		//m.printBlocks();
		
		vector <int> newIdx(trans.size());
		vector <bool> tempIsF;
		int n = 0;
		for (int i = 0; i < m.realSz; i++) {
			tempIsF.push_back(false);
			for (auto q : m.B[i].l) {
				newIdx[q] = n;
				tempIsF.back() = (tempIsF.back() || isF[q]);
			}
			n++;
		}
		init[0] = newIdx[init[0]];
		
		for (int i = 0; i < (int)tr.size(); i++) {
			auto [p, a, q] = tr[i].t;
			tr[i].t = {newIdx[p], a, newIdx[q]};
		}
		countSortTrans(tr, 2, n);
		countSortTrans(tr, 1, sigmaSize);
		countSortTrans(tr, 0, n);
		auto it = unique(tr.begin(), tr.end());
		tr.erase(it, tr.end());
		
		trans.clear(); t.clear();
		swap(tempIsF, isF);
		trans.resize(n);
		for (int i = 0; i < (int)tr.size(); i++) {
			auto [p, a, q] = tr[i].t;
			trans[p].push_back({q, a});
			if (isF[q]) t.push_back({p, a});
		}
	}
	
	void MakeTraversable() {
		transUM.resize(numStates());
		for (int i = 0; i < numStates(); i++) {
			for (auto [dest, ch] : trans[i]) {
				transUM[i][ch] = dest;
			}
		}
	}
	
	bool Traverse(vector <int> word) {
		int curS = init[0];
		for (auto ch : word) {
			auto it = transUM[curS].find(ch);
			if (it == transUM[curS].end()) return false;
			curS = it -> second;
		}
		return isF[curS];
	}
	
	void print(Sigma &sigma) {
		cout << "is epsilon in " << flag << endl;
		cout << "Number of States: " << isF.size() << endl;
		int numT = 0, numF = 0;
		for (int i = 0; i < (int)trans.size(); i++) {
			numT += trans[i].size();
			numF += isF[i];	
		}
		cout << "Number of Transitions: " << numT << endl;
		cout << "Number of Final: " << numF << endl;
		/*cout << "digraph automaton {\n";
		cout << "node [shape=none]; start" << endl;
		cout << "node [shape=doublecircle]; ";
		for (int i = 0; i < numStates(); i++) {
			if (isF[i]) cout << i << ' ';
		}
		cout << ";\n";
		
		cout << "node [shape=circle]; ";
		for (int i = 0; i < numStates(); i++) {
			if (!isF[i]) cout << i << ' ';
		}
		cout << ";\n";
		
		for (auto i : init) cout << "start -> " << i << endl;
		
		for (int i = 0; i < (int)trans.size(); i++) {
			for (auto tr : trans[i]) cout << i << " -> " << tr.dest << " [label=\"" << sigma.toChar(tr.ch) << "\"];" << endl;
		}
		cout << "}\n";
		//cout << "Table:" << endl;
		//for (auto i : t) cout << i.p << ' ' << i.ch << endl;*/
	}
};

#endif
