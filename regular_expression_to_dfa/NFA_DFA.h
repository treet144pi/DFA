#ifndef NFA_DFA_H
#define NFA_DFA_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <algorithm>

using namespace std;

struct Transition {
    int from;
    char symbol;
    int to;
};

struct NFA {
    int start;
    set<int> finals;
    vector<Transition> transitions;
};

struct DFA {
    int start;
    set<int> finals;
    map<pair<int, char>, int> transitions;
};

NFA createBasicNFA(char symbol);
NFA concatNFA(const NFA& a, const NFA& b);
NFA unionNFA(const NFA& a, const NFA& b);
NFA kleeneStar(const NFA& nfa);

string addConcatenation(const string& regex);
vector<char> infixToPostfix(const string& regex);

NFA buildNFAFromPostfix(const vector<char>& postfix);

DFA nfaToDfa(const NFA& nfa);

DFA minimizeDFA(const DFA& dfa);

void printDot(const DFA& dfa, const string& title = "DFA");
void printDot(const NFA& nfa, const string& title = "NFA");

#endif
