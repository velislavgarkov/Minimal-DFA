#ifndef MINIMIZE_TOOLS
#define MINIMIZE_TOOLS

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

struct FullTransition{
	array<int, 3> t;
	
	bool operator ==(const FullTransition& o) {
		return t == o.t;
	}
	int from() { return t[0]; }
	int ch() { return t[1]; }
	int to() { return t[2]; }
};

struct MinimizeTools {
	struct Block {
		list <int> l;
		int count = 0;
		int newBlockIdx = 0;
	};
	struct StateInfo {
		int idx1, idx2, b;
		list<int>::iterator pos;
	};
	
	vector <vector<int> > C;
	vector <int> I;
	vector <Block> B;
	vector <int> idxBlock;
	vector <int> whereInMem;
	vector <StateInfo> state;
	vector <int> myC;
	int realSz;
	int cntCh;
	
	MinimizeTools(vector <bool> &isF, vector <FullTransition> &tr, int sigmaSize) : C(sigmaSize), I(sigmaSize, -1), B(isF.size()), idxBlock(isF.size()), whereInMem(isF.size()), state(isF.size()), realSz(2), cntCh(0) {
		for (int i = 0; i < (int)isF.size(); i++) {
			state[i].b = !isF[i];
			B[state[i].b].l.push_back(i);
			state[i].pos = --B[state[i].b].l.end();
			state[i].idx1 = -1; state[i].idx2 = -2;
			idxBlock[i] = whereInMem[i] = i;
		}
		if (B[1].l.empty()) realSz--;
		
		for (int i = 0; i < (int)tr.size(); i++) {
			int q = tr[i].to();
			if (state[q].idx1 == -1) state[q].idx1 = i;
			state[q].idx2 = i;
		}
	}
	
	void CollectC(vector <FullTransition> &tr, int i) {
		cntCh = 0;
		for (auto q : B[i].l) {
			for (int idxT = state[q].idx1; idxT <= state[q].idx2; idxT++) {
				int p = tr[idxT].from(), a = tr[idxT].ch();
				if (I[a] == -1) {
					I[a] = cntCh++;
					myC.push_back(a);	
				}
				C[I[a]].push_back(p);
			}
		}
	}
	
	void CleanC(vector <FullTransition> &tr, list <int>& curL) {
		for (auto q : curL) {
			for (int idxT = state[q].idx1; idxT <= state[q].idx2; idxT++) {
				int a = tr[idxT].ch();
				if (I[a] != -1) {
					C[I[a]].clear();
					I[a] = -1;
				}
			}
		}
		cntCh = 0;
	}
	
	void Mark(vector <int> &c) {
		for (auto q : c) {
			int i = state[q].b;
			B[i].count++;
		}
	}
	
	void MoveSingleState(int q, int i, int j) {
		B[i].l.erase(state[q].pos);
		B[i].count--;
		
		B[j].l.push_back(q);
		state[q].pos = --B[j].l.end();
		state[q].b = j;
	}
	
	void Move(vector <int> &c) {
		for (auto q : c) {
			int i = state[q].b;
			if (B[i].count >= (int)B[i].l.size()) B[i].count = 0;
			
			if (B[i].count != 0) {
				int idx = B[i].newBlockIdx;
				if (idx == 0) {
					B[i].newBlockIdx = idx = realSz++;
				}
				
				MoveSingleState(q, i, idx);
				if (B[i].count == 0) {
					B[i].newBlockIdx = 0;
					if (B[i].l.size() < B[idx].l.size()) {
						swap(whereInMem[idxBlock[i]], whereInMem[idxBlock[idx]]);
						swap(idxBlock[i], idxBlock[idx]);
					}
				}
			}
		}
	}
	
	void Step(vector <FullTransition> &tr, int t) {
		t = whereInMem[t];
		CollectC(tr, t);
		auto curL = B[t].l;
		for (int i = 0; i < cntCh; i++) {
			Mark(C[i]);
			Move(C[i]);
		}
		myC.clear();
		CleanC(tr, curL);
	}
	
	void printBlock(int t) {
		cout << "Block " << t << " should be at " << idxBlock[t] << ":\n";
		for (auto q : B[t].l) cout << q << ' ';
		cout << endl;
	}
	void printBlocks() {
		cout << "Whole structure before this process:\n";
		for (int i = 0; i < realSz; i++) printBlock(i);
		cout << "indexes:\n";
		for (int i = 0; i < (int)state.size(); i++) cout << "state " << i << " at block " << state[i].b << endl;
	}
};

#endif
