#include <iostream>
#include <string>
#include <chrono>
#include "automation.cpp"

void runTest(const std::string& regex) {
    std::cout << "Тест для выражения: " << regex << std::endl;

    // построения НКА
    auto start = std::chrono::high_resolution_clock::now();
    NFA nfa = thompsonConstruction(regex);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для построения НКА: " << duration.count() << " микросекунд." << std::endl;

    // НКА в ДКА
    start = std::chrono::high_resolution_clock::now();
    DFA dfa = determinize(nfa);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для детерминизации НКА в ДКА: " << duration.count() << " микросекунд." << std::endl;

    // минимизации ДКА
    start = std::chrono::high_resolution_clock::now();
    minimizeDFA(dfa);
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Время для минимизации ДКА: " << duration.count() << " микросекунд." << std::endl;
}

int main() {
    // Тестирование различных регулярных выражений
    std::string regex1 = "(a|b)*";
    std::string regex2 = "ab(a|b)*c";
    std::string regex3 = "(a|b|c)*";
    std::string regex4 = "(a|b)(a|c)*(a|b)";
    std::string regex5 = "(a|b)(a|b|c)*ab";

    runTest(regex1);
    runTest(regex2);
    runTest(regex3);
    runTest(regex4);
    runTest(regex5);

    return 0;
}
