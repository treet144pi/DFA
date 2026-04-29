#include "NFA_DFA.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono;

string random_regex(int target_length) {
    static mt19937 rng(42);
    string alphabet = "ab";
    uniform_int_distribution<int> coin(0, 2);
    uniform_int_distribution<int> letter(0, alphabet.size()-1);

    string s;
    while ((int)s.size() < target_length) {
        int choice = coin(rng);
        if (choice == 0) {
            s += alphabet[letter(rng)];
        } else if (choice == 1) {
            s += alphabet[letter(rng)];
            s += '*';
        } else {
            s += "(";
            s += alphabet[letter(rng)];
            s += "|";
            s += alphabet[letter(rng)];
            s += ")";
        }
        if (s.size() + 1 < target_length && coin(rng) == 0) {
            s += "|";
        }
    }
    if ((int)s.size() > target_length) {
        s.erase(target_length);
        while (!s.empty() && (s.back() == '|' || s.back() == '(')) s.pop_back();
    }
    if (s.empty()) s = "a";
    return s;
}

struct BenchResult {
    int length;
    int nfa_states;
    int dfa_states;
    int mindfa_states;
    double nfa_time_ms;
    double dfa_time_ms;
    double min_time_ms;
    double total_time_ms;
};

BenchResult run_benchmark(const string& regex, int length) {

    auto t1 = high_resolution_clock::now();
    string withDot = addConcatenation(regex);
    vector<char> postfix = infixToPostfix(withDot);
    NFA nfa = buildNFAFromPostfix(postfix);
    auto t2 = high_resolution_clock::now();

    set<int> nfa_states_set;
    nfa_states_set.insert(nfa.start);
    for (auto& tr : nfa.transitions) {
        nfa_states_set.insert(tr.from);
        nfa_states_set.insert(tr.to);
    }
    int nfa_states = nfa_states_set.size();

    DFA dfa = nfaToDfa(nfa);
    auto t3 = high_resolution_clock::now();

    set<int> dfa_states_set;
    dfa_states_set.insert(dfa.start);
    for (auto& tr : dfa.transitions) {
        dfa_states_set.insert(tr.first.first);
        dfa_states_set.insert(tr.second);
    }
    int dfa_states = dfa_states_set.size();

    DFA minDfa = minimizeDFA(dfa);
    auto t4 = high_resolution_clock::now();

    set<int> min_states_set;
    min_states_set.insert(minDfa.start);
    for (auto& tr : minDfa.transitions) {
        min_states_set.insert(tr.first.first);
        min_states_set.insert(tr.second);
    }
    int mindfa_states = min_states_set.size();

    double nfa_time = duration<double, milli>(t2 - t1).count();
    double dfa_time = duration<double, milli>(t3 - t2).count();
    double min_time = duration<double, milli>(t4 - t3).count();

    return {length, nfa_states, dfa_states, mindfa_states,
            nfa_time, dfa_time, min_time, nfa_time + dfa_time + min_time};
}

int main() {
    cout << fixed << setprecision(3);

    vector<int> lengths = {10, 100,500,200,300, 1000,2000,3000,4000,5000,6000,10000,20000,100000};
    int repeats = 5;

    cout << "Benchmark results (averaged over " << repeats << " runs)\n";
    cout << "Length | NFA states | DFA states | MinDFA states | "
         << "NFA time(ms) | DFA time(ms) | Min time(ms) | Total time(ms)\n";
    cout << string(100, '-') << '\n';

    for (int L : lengths) {
        double sum_nfa = 0, sum_dfa = 0, sum_min = 0, sum_total = 0;
        int avg_nfa_st = 0, avg_dfa_st = 0, avg_min_st = 0;

        for (int r = 0; r < repeats; ++r) {
            string re = random_regex(L);
            BenchResult res = run_benchmark(re, L);

            sum_nfa += res.nfa_time_ms;
            sum_dfa += res.dfa_time_ms;
            sum_min += res.min_time_ms;
            sum_total += res.total_time_ms;
            avg_nfa_st += res.nfa_states;
            avg_dfa_st += res.dfa_states;
            avg_min_st += res.mindfa_states;
        }

        avg_nfa_st /= repeats;
        avg_dfa_st /= repeats;
        avg_min_st /= repeats;

        cout << setw(6)  << L << " | "
             << setw(10) << avg_nfa_st << " | "
             << setw(10) << avg_dfa_st << " | "
             << setw(13) << avg_min_st << " | "
             << setw(12) << sum_nfa/repeats << " | "
             << setw(12) << sum_dfa/repeats << " | "
             << setw(12) << sum_min/repeats << " | "
             << setw(14) << sum_total/repeats << "\n";
    }

    return 0;
}
