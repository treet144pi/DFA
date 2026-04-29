#include "NFA_DFA.h"

static int nextState = 0;

void resetStateCounter() { nextState = 0; }

NFA createBasicNFA(char symbol) {
    NFA nfa;
    int s0 = nextState++;
    int s1 = nextState++;
    nfa.start = s0;
    nfa.finals = {s1};
    nfa.transitions.push_back({s0, symbol, s1});
    return nfa;
}

NFA concatNFA(const NFA& a, const NFA& b) {
    NFA res;
    res.start = a.start;
    res.finals = b.finals;

    res.transitions = a.transitions;
    res.transitions.insert(res.transitions.end(),
                           b.transitions.begin(), b.transitions.end());

    for (int f : a.finals)
        res.transitions.push_back({f, 0, b.start});

    return res;
}

NFA unionNFA(const NFA& a, const NFA& b) {
    NFA res;
    int newStart = nextState++;
    int newFinal = nextState++;
    res.start = newStart;
    res.finals = {newFinal};

    res.transitions = a.transitions;
    res.transitions.insert(res.transitions.end(),
                           b.transitions.begin(), b.transitions.end());

    res.transitions.push_back({newStart, 0, a.start});
    res.transitions.push_back({newStart, 0, b.start});

    for (int f : a.finals)
        res.transitions.push_back({f, 0, newFinal});
    for (int f : b.finals)
        res.transitions.push_back({f, 0, newFinal});

    return res;
}

NFA kleeneStar(const NFA& nfa) {
    NFA res;
    int newStart = nextState++;
    int newFinal = nextState++;
    res.start = newStart;
    res.finals = {newFinal};

    res.transitions = nfa.transitions;

    res.transitions.push_back({newStart, 0, nfa.start});  // вход
    res.transitions.push_back({newStart, 0, newFinal});   // пустая цепочка

    for (int f : nfa.finals) {
        res.transitions.push_back({f, 0, nfa.start});
        res.transitions.push_back({f, 0, newFinal});
    }

    return res;
}

string addConcatenation(const string& re) {
    string res;
    for (size_t i = 0; i < re.size(); ++i) {
        char c = re[i];
        res.push_back(c);
        if (i + 1 < re.size()) {
            char next = re[i+1];
            if (c != '(' && c != '|' && next != ')' && next != '|' && next != '*') {
                res.push_back('.');
            }
        }
    }
    return res;
}

vector<char> infixToPostfix(const string& regex) {

    map<char, int> prec = { {'*', 3}, {'.', 2}, {'|', 1} };
    stack<char> ops;
    vector<char> postfix;

    for (char c : regex) {
        if (isalnum(c)) {
            postfix.push_back(c);
        } else if (c == '(') {
            ops.push(c);
        } else if (c == ')') {
            while (!ops.empty() && ops.top() != '(') {
                postfix.push_back(ops.top());
                ops.pop();
            }
            if (!ops.empty()) ops.pop();
        } else {
            while (!ops.empty() && ops.top() != '(' &&
                   prec[ops.top()] >= prec[c]) {
                postfix.push_back(ops.top());
                ops.pop();
            }
            ops.push(c);
        }
    }
    while (!ops.empty()) {
        postfix.push_back(ops.top());
        ops.pop();
    }
    return postfix;
}

NFA buildNFAFromPostfix(const vector<char>& postfix) {
    stack<NFA> st;
    for (char token : postfix) {
        if (isalnum(token)) {
            st.push(createBasicNFA(token));
        } else if (token == '.') {        // конкатенация
            NFA b = st.top(); st.pop();
            NFA a = st.top(); st.pop();
            st.push(concatNFA(a, b));
        } else if (token == '|') {
            NFA b = st.top(); st.pop();
            NFA a = st.top(); st.pop();
            st.push(unionNFA(a, b));
        } else if (token == '*') {
            NFA a = st.top(); st.pop();
            st.push(kleeneStar(a));
        }
    }
    return st.top();
}

set<int> epsilonClosure(const NFA& nfa, const set<int>& states) {
    set<int> closure = states;
    stack<int> stk;
    for (int s : states) stk.push(s);
    while (!stk.empty()) {
        int s = stk.top(); stk.pop();
        for (auto& t : nfa.transitions) {
            if (t.from == s && t.symbol == 0) {
                if (closure.insert(t.to).second) {
                    stk.push(t.to);
                }
            }
        }
    }
    return closure;
}

set<int> move(const NFA& nfa, const set<int>& states, char symbol) {
    set<int> result;
    for (int s : states) {
        for (auto& t : nfa.transitions) {
            if (t.from == s && t.symbol == symbol) {
                result.insert(t.to);
            }
        }
    }
    return result;
}

DFA nfaToDfa(const NFA& nfa) {
    DFA dfa;
    map<set<int>, int> stateId;
    queue<set<int>> q;

    set<int> startSet = epsilonClosure(nfa, {nfa.start});
    q.push(startSet);
    stateId[startSet] = 0;
    dfa.start = 0;
    int nextId = 1;

    set<char> alphabet;
    for (auto& t : nfa.transitions)
        if (t.symbol != 0) alphabet.insert(t.symbol);

    while (!q.empty()) {
        set<int> cur = q.front(); q.pop();
        int curId = stateId[cur];

        for (int f : nfa.finals)
            if (cur.count(f)) {
                dfa.finals.insert(curId);
                break;
            }

        for (char sym : alphabet) {
            set<int> nxt = epsilonClosure(nfa, move(nfa, cur, sym));
            if (nxt.empty()) continue;
            int nxtId;
            if (stateId.find(nxt) == stateId.end()) {
                nxtId = nextId++;
                stateId[nxt] = nxtId;
                q.push(nxt);
            } else {
                nxtId = stateId[nxt];
            }
            dfa.transitions[{curId, sym}] = nxtId;
        }
    }
    return dfa;
}

DFA minimizeDFA(const DFA& dfa) {
    set<int> reachable = {dfa.start};
    stack<int> stk; stk.push(dfa.start);
    while (!stk.empty()) {
        int s = stk.top(); stk.pop();
        for (auto& t : dfa.transitions) {
            if (t.first.first == s && reachable.insert(t.second).second)
                stk.push(t.second);
        }
    }

    DFA clean;
    clean.start = dfa.start;
    map<int,int> oldToNew;
    int newId = 0;
    for (int s : reachable) oldToNew[s] = newId++;
    for (int s : reachable) {
        for (auto& t : dfa.transitions) {
            if (t.first.first == s) {
                clean.transitions[{oldToNew[s], t.first.second}] = oldToNew[t.second];
            }
        }
        if (dfa.finals.count(s)) clean.finals.insert(oldToNew[s]);
    }

    set<int> allStates;
    for (int i = 0; i < newId; ++i) allStates.insert(i);
    set<int> F = clean.finals;
    set<int> QminusF;
    for (int s : allStates) if (!F.count(s)) QminusF.insert(s);

    vector<set<int>> P;
    if (!F.empty()) P.push_back(F);
    if (!QminusF.empty()) P.push_back(QminusF);

    set<char> alphabet;
    for (auto& t : clean.transitions) alphabet.insert(t.first.second);

    bool changed = true;
    while (changed) {
        changed = false;
        for (char c : alphabet) {
            vector<set<int>> newP;
            for (auto& block : P) {
                map<size_t, set<int>> split;
                for (int s : block) {
                    int target = -1;
                    auto it = clean.transitions.find({s, c});
                    if (it != clean.transitions.end()) target = it->second;
                    size_t classIdx = (size_t)-1;
                    if (target != -1) {
                        for (size_t i = 0; i < P.size(); ++i) {
                            if (P[i].count(target)) { classIdx = i; break; }
                        }
                    }
                    split[classIdx].insert(s);
                }
                if (split.size() > 1) changed = true;
                for (auto& kv : split) newP.push_back(kv.second);
            }
            P = newP;
        }
    }

    DFA minDfa;
    map<int,int> classRep;
    for (size_t i = 0; i < P.size(); ++i)
        for (int s : P[i]) classRep[s] = (int)i;

    minDfa.start = classRep[clean.start];
    for (auto& t : clean.transitions) {
        minDfa.transitions[{classRep[t.first.first], t.first.second}] = classRep[t.second];
    }
    for (int f : clean.finals)
        minDfa.finals.insert(classRep[f]);

    return minDfa;
}

void printDot(const DFA& dfa, const string& title) {
    cout << "digraph " << title << " {\n";
    cout << "  rankdir=LR;\n";
    cout << "  node [shape=circle];\n";
    for (auto& t : dfa.transitions) {
        cout << "  " << t.first.first << " -> " << t.second
             << " [label=\"" << t.first.second << "\"];\n";
    }
    for (int f : dfa.finals) {
        cout << "  " << f << " [shape=doublecircle];\n";
    }
    cout << "}\n";
}

void printDot(const NFA& nfa, const string& title) {
    cout << "digraph " << title << " {\n";
    cout << "  rankdir=LR;\n";
    cout << "  node [shape=circle];\n";
    for (auto& t : nfa.transitions) {
        string label = (t.symbol == 0) ? "ε" : string(1, t.symbol);
        cout << "  " << t.from << " -> " << t.to
             << " [label=\"" << label << "\"];\n";
    }
    for (int f : nfa.finals) {
        cout << "  " << f << " [shape=doublecircle];\n";
    }
    cout << "}\n";
}
