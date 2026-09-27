#ifndef OPERATIONS
#define OPERATIONS

#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "Automata.hpp"
using namespace std;

void AutomataWord(Automata &result, vector <int> &word) {
	result.addNewNode();
	result.init.push_back(0);
	
	for (int i = 0; i < (int)word.size(); i++) {
		int curCh = word[i];
		result.addNewNode();
		result.trans[i].push_back({i + 1, curCh});
		if (i == (int)word.size() - 1) {
			result.isF[i + 1] = true;
			result.t.push_back({i, curCh});	
		}
	}
	result.det = true;
}

void Union(Automata &result, Automata &l, Automata &r) {
	assert(&l != &r);
    assert(&result != &l);
    assert(&result != &r);
    
	if (r.size() > l.size()) swap(l, r);
	swap(l, result);
	r.shift(result.numStates());
	
	result.flag |= r.flag;
	result.trans.insert(result.trans.end(), make_move_iterator(r.trans.begin()), make_move_iterator(r.trans.end()));
	result.init.insert(result.init.end(), r.init.begin(), r.init.end());
	result.isF.insert(result.isF.end(), r.isF.begin(), r.isF.end());
	result.t.insert(result.t.end(), r.t.begin(), r.t.end());
	result.det = false;
}

void Concat(Automata &result, Automata &l, Automata &r) {
	assert(&l != &r);
    assert(&result != &l);
    assert(&result != &r);

    bool unionMode = true;
    int lefNumS = l.numStates();
    int riNumS = r.numStates();
    if (l.size() > r.size()) r.shift(lefNumS);
    else {
        l.shift(riNumS);
        unionMode = false;
    }

    if (!unionMode) swap(l.trans, r.trans);
    swap(result.trans, l.trans);
    result.trans.insert(result.trans.end(), make_move_iterator(r.trans.begin()), make_move_iterator(r.trans.end()));

    for (auto i : l.t) {
        for (auto j : r.init) {
            result.trans[i.p].push_back({j, i.ch});
        }
    }

    if (!l.flag) swap(result.init, l.init);
    else {
        if (!unionMode) swap(l.init, r.init);
        swap(result.init, l.init);
        result.init.insert(result.init.end(), r.init.begin(), r.init.end());
    }

    if (!r.flag) {
        swap(result.t, r.t);
        if (unionMode) {
            result.isF = vector<bool>(lefNumS, false);
            result.isF.insert(result.isF.end(), r.isF.begin(), r.isF.begin() + riNumS);
        } else {
            swap(result.isF, r.isF);
            result.isF.resize(lefNumS + riNumS);
        }
    } else {
        if (!unionMode) {
            swap(l.isF, r.isF);
            swap(l.t, r.t);
        }
        swap(result.isF, l.isF);
        swap(result.t, l.t);
        result.isF.insert(result.isF.end(), r.isF.begin(), r.isF.end());
        result.t.insert(result.t.end(), r.t.begin(), r.t.end());
    }

    result.flag = (l.flag && r.flag);
    result.det = false;
}

void Star(Automata &result, Automata &src) {
    swap(result, src);
    for (auto i : result.t) {
        for (auto j : result.init) {
            result.trans[i.p].push_back({j, i.ch});
        }
    }
    result.flag = true;
    result.det = false;
}

void Reverse(Automata &result, Automata &src) {
    swap(result, src);
    int sz = result.numStates();

    vector<int> tempI;
    for (int i = 0; i < sz; i++) {
        if (result.isF[i]) tempI.push_back(i);
        result.isF[i] = false;
    }
    for (auto i : result.init) result.isF[i] = true;
    result.init = tempI;

    vector<vector<Transition>> temp(sz);
    vector<TempTable> tempT;
    for (int d = 0; d < sz; d++) {
        for (auto i : result.trans[d]) {
            temp[i.dest].push_back({d, i.ch});
            if (result.isF[d]) tempT.push_back({i.dest, i.ch});
        }
    }
    result.trans = move(temp);
    result.t = move(tempT);
    result.det = false;
}

struct SquaredHash {
	unordered_map<long long, int> um;
	long long val;
	int sz;
	
	SquaredHash(int Val) : val(Val), sz(0) {}
	int getRealState(long long p, long long q) {
		auto it = um.find(p * val + q);
		if (it == um.end()) return -1;
		return it->second;
	}
	int addSquaredState(long long p, long long q) {
		return um[p * val + q] = sz++;
	}
};

void Intersect(Automata &result, Automata &l, Automata &r) {
	assert(&l != &r);
    assert(&result != &l);
    assert(&result != &r);

    l.Determinize();
    r.Determinize();
    //l.Minimize(); r.Minimize();

    SquaredHash h(r.numStates());
    queue<pair<int, int>> q;
    q.push({l.init[0], r.init[0]});
    h.addSquaredState(l.init[0], r.init[0]);
    
    result.addNewNode();
    result.init.push_back(0);
    if (l.isF[l.init[0]] && r.isF[r.init[0]]) result.isF[0] = true;

    while (!q.empty()) {
        auto [p1, p2] = q.front();
        q.pop();
        int curState = h.getRealState(p1, p2);

        int i1 = 0, i2 = 0;
        while (i1 < (int)l.trans[p1].size() && i2 < (int)r.trans[p2].size()) {
            if (l.trans[p1][i1].ch != r.trans[p2][i2].ch) {
                if (l.trans[p1][i1].ch < r.trans[p2][i2].ch) i1++;
                else i2++;
            } else {
                int d1 = l.trans[p1][i1].dest, d2 = r.trans[p2][i2].dest;
                int ch = l.trans[p1][i1].ch;

                int dest = h.getRealState(d1, d2);
                if (dest == -1) {
                    result.addNewNode();
                    q.push({d1, d2});
                    dest = h.addSquaredState(d1, d2);
                }

                result.trans[curState].push_back({dest, ch});
                if (l.isF[d1] && r.isF[d2]) {
                    result.isF[dest] = true;
                    result.t.push_back({curState, ch});
                }
                i1++; i2++;
            }
        }
    }

    result.flag = (l.flag && r.flag);
    result.det = true;
}

void Subtract(Automata &result, Automata &l, Automata &r) {
	assert(&l != &r);
    assert(&result != &l);
    assert(&result != &r);
    
    r.Complement();
    Intersect(result, l, r);
}

#endif
