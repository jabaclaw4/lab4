#include <iostream>

using namespace std;

void run_test_lazy_sequence();
void run_test_streams();
void run_test_heap_sorter();

void run_all_tests() {
    cout << "\n=== RUNNING ALL TESTS FOR LAB 4 ===" << endl;

    cout << "\n[1/3] LazySequence tests:" << endl;
    run_test_lazy_sequence();

    cout << "\n[2/3] Stream tests:" << endl;
    run_test_streams();

    cout << "\n[3/3] HeapSorter tests + O(n log n) check:" << endl;
    run_test_heap_sorter();
}