#include <iostream>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <string>
#include <chrono>
#include <fstream>

// Структуры для НКА
struct NFA {
    std::vector<std::map<char, std::set<int>>> transition;
    std::vector<bool> isTerminal;
    int startState;
};

// Структуры для ДКА
struct DFA {
    std::vector<std::map<char, int>> transition;
    std::vector<bool> isTerminal;
    int startState;
};

// Функция для создания нового состояния НКА
int newState(NFA& nfa) {
    int newState = nfa.transition.size();
    nfa.transition.push_back(std::map<char, std::set<int>>());
    nfa.isTerminal.push_back(false);
    return newState;
}

// Алгоритм Томпсона для построения НКА
NFA thompsonConstruction(const std::string& regex) {
    std::stack<NFA> nfaStack;

    for (char c : regex) {
        NFA nfa;
        int start = newState(nfa);
        int end = newState(nfa);

        if (c == 'a' || c == 'b') { // Простая альтернатива
            nfa.transition[start][c].insert(end);
        } else if (c == '*') { // Звездочка Клини
            nfa.transition[start]['\0'].insert(end);  // ε-переход
            nfa.transition[end]['\0'].insert(start);  // цикл
            nfa.transition[start]['\0'].insert(end);  // для пустой строки
        }

        nfaStack.push(nfa);
    }

    NFA nfa = nfaStack.top();
    nfa.startState = 0;
    return nfa;
}

// Детерминизация НКА в ДКА
DFA determinize(NFA& nfa) {
    DFA dfa;
    std::map<std::set<int>, int> stateMap;

    std::set<int> startSet = {nfa.startState};
    stateMap[startSet] = 0;

    dfa.startState = 0;

    for (auto& entry : stateMap) {
        std::set<int> currentSet = entry.first;
        for (char c = 'a'; c <= 'z'; ++c) {
            std::set<int> nextSet;
            for (int state : currentSet) {
                if (nfa.transition[state].find(c) != nfa.transition[state].end()) {
                    nextSet.insert(nfa.transition[state][c].begin(), nfa.transition[state][c].end());
                }
            }

            if (!nextSet.empty() && stateMap.find(nextSet) == stateMap.end()) {
                int newState = stateMap.size();
                stateMap[nextSet] = newState;
                dfa.transition.push_back(std::map<char, int>());
                dfa.isTerminal.push_back(false);
            }

            if (!nextSet.empty()) {
                dfa.transition[entry.second][c] = stateMap[nextSet];
            }
        }
    }

    return dfa;
}

// Минимизация ДКА
void minimizeDFA(DFA& dfa) {
    std::vector<std::vector<bool>> table(dfa.transition.size(), std::vector<bool>(dfa.transition.size(), false));

    for (int i = 0; i < dfa.transition.size(); ++i) {
        for (int j = i + 1; j < dfa.transition.size(); ++j) {
            if (dfa.isTerminal[i] != dfa.isTerminal[j]) {
                table[i][j] = true;
            }
        }
    }

    for (int k = 0; k < dfa.transition.size(); ++k) {
        for (int i = 0; i < dfa.transition.size(); ++i) {
            for (int j = i + 1; j < dfa.transition.size(); ++j) {
                if (!table[i][j]) {
                    for (char c = 'a'; c <= 'z'; ++c) {
                        if (dfa.transition[i].find(c) != dfa.transition[i].end() &&
                            dfa.transition[j].find(c) != dfa.transition[j].end()) {
                            if (table[dfa.transition[i][c]][dfa.transition[j][c]]) {
                                table[i][j] = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<int> mapping(dfa.transition.size(), -1);
    int newState = 0;
    for (int i = 0; i < dfa.transition.size(); ++i) {
        if (mapping[i] == -1) {
            mapping[i] = newState++;
        }
        for (int j = i + 1; j < dfa.transition.size(); ++j) {
            if (!table[i][j] && mapping[j] == -1) {
                mapping[j] = mapping[i];
            }
        }
    }

    std::vector<std::map<char, int>> newTransition(newState);
    std::vector<bool> newIsTerminal(newState, false);
    for (int i = 0; i < dfa.transition.size(); ++i) {
        for (auto& entry : dfa.transition[i]) {
            newTransition[mapping[i]][entry.first] = mapping[entry.second];
        }
        newIsTerminal[mapping[i]] = dfa.isTerminal[i];
    }

    dfa.transition = newTransition;
    dfa.isTerminal = newIsTerminal;
}

// Генерация DOT файла для НКА
void generateNfaGraph(const NFA& nfa) {
    std::ofstream outFile("nfa_graph.dot");

    outFile << "digraph NFA {" << std::endl;
    outFile << "    rankdir=LR;" << std::endl;  // Левый направленный граф (по умолчанию сверху вниз)

    // Пример нумерации состояний, создаем уникальные имена для состояний
    for (int i = 0; i < nfa.transition.size(); ++i) {
        if (nfa.isTerminal[i]) {
            outFile << "    " << i << " [shape=doublecircle];" << std::endl; // Двойной круг для терминальных состояний
        } else {
            outFile << "    " << i << " [shape=circle];" << std::endl; // Обычный круг для нетерминальных состояний
        }
    }

    // Генерация переходов
    for (int i = 0; i < nfa.transition.size(); ++i) {
        for (const auto& entry : nfa.transition[i]) {
            for (int state : entry.second) {
                outFile << "    " << i << " -> " << state << " [label=\"" << entry.first << "\"];" << std::endl;
            }
        }
    }

    outFile << "}" << std::endl;
    outFile.close();
}

// Генерация DOT файла для ДКА
void generateDfaGraph(const DFA& dfa) {
    std::ofstream outFile("dfa_graph.dot");

    outFile << "digraph DFA {" << std::endl;
    outFile << "    rankdir=LR;" << std::endl;  // Левый направленный граф (по умолчанию сверху вниз)

    // Пример нумерации состояний, создаем уникальные имена для состояний
    for (int i = 0; i < dfa.transition.size(); ++i) {
        if (dfa.isTerminal[i]) {
            outFile << "    " << i << " [shape=doublecircle];" << std::endl; // Двойной круг для терминальных состояний
        } else {
            outFile << "    " << i << " [shape=circle];" << std::endl; // Обычный круг для нетерминальных состояний
        }
    }

    // Генерация переходов
    for (int i = 0; i < dfa.transition.size(); ++i) {
        for (const auto& entry : dfa.transition[i]) {
            outFile << "    " << i << " -> " << entry.second << " [label=\"" << entry.first << "\"];" << std::endl;
        }
    }

    outFile << "}" << std::endl;
    outFile.close();
}

// Основной тестовый блок для одного регулярного выражения
void runTest(const std::string& regex) {
    std::cout << "Тест для регулярного выражения: " << regex << std::endl;

    // Засекаем время для построения НКА
    auto start = std::chrono::high_resolution_clock::now();
    NFA nfa = thompsonConstruction(regex);  // Строим НКА
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для построения НКА: " << duration.count() << " микросекунд." << std::endl;

    // Генерируем граф для НКА
    generateNfaGraph(nfa);

    // Засекаем время для детерминизации НКА в ДКА
    start = std::chrono::high_resolution_clock::now();
    DFA dfa = determinize(nfa);  // Детерминизация НКА в ДКА
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для детерминизации НКА в ДКА: " << duration.count() << " микросекунд." << std::endl;

    // Генерируем граф для ДКА
    generateDfaGraph(dfa);

    // Засекаем время для минимизации ДКА
    start = std::chrono::high_resolution_clock::now();
    minimizeDFA(dfa);  // Минимизация ДКА
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для минимизации ДКА: " << duration.count() << " микросекунд." << std::endl;
}

int main() {
    // Пример регулярного выражения
    std::string regex = "(a|b)*"; // Вы можете заменить на любое регулярное выражение

    // Запуск теста
    runTest(regex);

    std::cout << "Генерация графов завершена. Для визуализации используйте Graphviz." << std::endl;

    return 0;
}
