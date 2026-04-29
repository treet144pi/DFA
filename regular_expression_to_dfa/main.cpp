#include "NFA_DFA.h"

int main() {
    string regex;
    cout << "Введите регулярное выражение (eng lowercase): ";
    getline(cin, regex);
    if (regex.empty()) regex = "(a|b)*abb";
    string withDot = addConcatenation(regex);
    cout << "Выражение с точками: " << withDot << endl;

    vector<char> postfix = infixToPostfix(withDot);
    cout << "Постфикс: ";
    for (char c : postfix) cout << c;
    cout << endl;

    NFA nfa = buildNFAFromPostfix(postfix);
    cout << "\n===== NFA =====\n";
    printDot(nfa, "NFA");


    DFA dfa = nfaToDfa(nfa);
    cout << "\n===== DFA =====\n";
    printDot(dfa, "DFA");

    DFA minDfa = minimizeDFA(dfa);
    cout << "\n===== Minimized DFA =====\n";
    printDot(minDfa, "MinDFA");

    return 0;
}
