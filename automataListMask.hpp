#ifndef AUTOMATA_LIST
#define AUTOMATA_LIST

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
using namespace std;
using Clock = chrono::high_resolution_clock;

struct Transition{
	int dest;
	int ch;
};

struct BitTrans {
	vector <uint64_t> v;
	int ch;
	bool isTF;
};
struct TempTable{
	int p;
	int ch;
};

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
	
	MinimizeTools(vector <bool> &isF, vector <FullTransition> &tr) : C(sigmaSize), I(sigmaSize, -1), B(isF.size()), idxBlock(isF.size()), whereInMem(isF.size()), state(isF.size()), realSz(2), cntCh(0) {
		for (int i = 0; i < (int)isF.size(); i++) {
			state[i].b = isF[i];
			B[state[i].b].l.push_back(i);
			state[i].pos = --B[state[i].b].l.end();
			state[i].idx1 = -1; state[i].idx2 = -2;
			idxBlock[i] = whereInMem[i] = i;
		}
		
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
		//cout << "currently processing block: " << t << endl;
		//printBlocks();
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
		size_t operator()(const vector<uint64_t>& v) const {
		    size_t seed = 0;

		    for (uint64_t x : v) {
		        seed ^= hash<uint64_t>{}(x) + 0x9e3779b97f4a7c15ULL
		                + (seed << 6)
		                + (seed >> 2);
		    }

		    return seed;
		}
	};

	unordered_map <vector <uint64_t> , int, VectorHash> SetToId;
	vector <vector <uint64_t> > s;
	bool type;
	
	void add(vector <uint64_t> &v) {
		SetToId[v] = s.size();
		s.push_back(v);
	}
	int findVector(vector <uint64_t> &v) {
		auto it = SetToId.find(v);
		if (it == SetToId.end()) return -1;
		return it->second;
	}
};

void OrBit(vector<uint64_t> &v, int idx) {
    int ost  = idx & 63;
    int word = idx >> 6;
    v[word] |= (1ULL << ost);
}
void OrVector (vector <uint64_t> &res, vector <uint64_t> mask) {
	for (int i = 0; i < (int)res.size(); i++) {
		res[i] |= mask[i];
	}
}

struct AutomatonList {
	vector <vector <Transition> > trans;
	vector <int> init;
	vector <bool> isF;
	vector <TempTable> t;
	bool flag = false;
	bool det = false;
	
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
	void print() {
		cout << "is epsilon in " << flag << endl;
		cout << isF.size() << endl;
		cout << "initial "; for (auto i : init) cout << i << ' ';
		cout << endl;
		for (int i = 0; i < (int)trans.size(); i++) {
			cout << "transitions from " << i << " and its finality is ";
			if (i < (int)isF.size()) cout << isF[i] << endl;
			else cout << "nothing" << endl;
			for (auto tr : trans[i]) cout << i << '-' << tr.ch << "->" << tr.dest << endl;
		}
		//cout << "Table:" << endl;
		//for (auto i : t) cout << i.p << ' ' << i.ch << endl;
	}
	
	void Determinize(bool addingEpsilon = false) {
		if (det && !addingEpsilon) return;
		
		if (addingEpsilon) {
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
		vector <vector <int> > chars(trans.size());
		
		int n = numStates();
		int maskSz = (n >> 6) + ((n & 63) != 0);
		StateHash h;
		h.type = true;
		h.SetToId.reserve(200000);
		h.s.reserve(200000);
		queue <int> q;
		q.push(0);
		
		vector <uint64_t> curMask(maskSz);
		vector <vector <BitTrans> > bitT(n);
		for (int i = 0; i < n; i++) {
			sort(trans[i].begin(), trans[i].end(), [&](Transition& a, Transition& b) { return a.ch < b.ch; });
			for (int j = 0; j < (int)trans[i].size(); j++) {
				int c = trans[i][j].ch;
				chars[i].push_back(c);
				bool isTF = false;
				
				while (j < (int)trans[i].size() && trans[i][j].ch == c) {
					if (h.type) {
						OrBit(curMask, trans[i][j].dest);
						isTF = (isTF || isF[trans[i][j].dest]);
					}
					j++;
				}
				j--;
				
				if (h.type) {
					bitT[i].push_back({curMask, c, isTF});
					for (int k = 0; k < maskSz; k++) curMask[k] = 0;
				}
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
		int szInit = init.size();
		if (h.type) szInit = maskSz;
		vector <uint64_t> init64(szInit);
		
		sort(init.begin(), init.end());
		for (int i = 0; i < (int)init.size(); i++) {
			if (!h.type) init64[i] = init[i];
			else OrBit(init64, init[i]);
		}
		h.add(init64);
		init.clear();
		init.push_back(0);

		while (!q.empty()) {
			auto p = q.front(); q.pop();
			
			set<int> chT;
			int idxEl = 0;
			for (auto el : h.s[p]) {
				int curS;
				if (!h.type) {
					curS = el;
					for (auto ch : chars[curS]) chT.insert(ch);
				} else {
					while (el) {
						curS = (idxEl << 6) + __builtin_ctzll(el);
						for (auto ch : chars[curS]) chT.insert(ch);
						el = (el & (el - 1));
					}
					idxEl++;
				}
			}
			
			int br = 0;
			for (auto ch : chT) {
				realCh[br] = ch;
				chIdx[ch] = br++;
			}
			
			vector<vector<uint64_t> > newS(br);
			vector<bool> curF(br);
			
			if (h.type) {
				for (int i = 0; i < br; i++) newS[i].resize(maskSz);
			}
			idxEl = 0;
			for (auto el : h.s[p]) {
				int curS;
				if (!h.type) {
					curS = el;
					for (auto [dest, ch] : trans[curS]) {
						newS[chIdx[ch]].push_back(dest);
						curF[chIdx[ch]] = curF[chIdx[ch]] || isF[dest];
					}
				} else {
					while (el) {
						curS = (idxEl << 6) + __builtin_ctzll(el);
						for (int i = 0; i < (int)bitT[curS].size(); i++) {
							auto [dest, ch, isTF] = bitT[curS][i];
							OrVector(newS[chIdx[ch]], dest);
							curF[chIdx[ch]] = (curF[chIdx[ch]] || isTF);
						}
						el = (el & (el - 1));
					}
					idxEl++;
				}
			}
			
			for (int a = 0; a < br; a++) {
				if (!h.type) {
					sort(newS[a].begin(), newS[a].end());
					auto it = unique(newS[a].begin(), newS[a].end());
					newS[a].erase(it, newS[a].end());
				}
				
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
	
	void Minimize() {
		Determinize();
		
		vector <FullTransition> tr;
		tr.reserve(trans.size());
		for (int i = 0; i < numStates(); i++) {
			for (auto [dest, ch] : trans[i]) tr.push_back({i, ch, dest});
		}
		countSortTrans(tr, 1, sigmaSize);
		countSortTrans(tr, 2, trans.size());
		
		MinimizeTools m(isF, tr);
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
};

// (a)|((b).(c))|((d)*)
// !((abc)|(def))
// (((a).(b))|((a).(c)))&(((a).(b))|((a).(d)))
// ((a)*)&((aa)*)
// (aac)|(abd)|(bad)|(bbc)
#endif
