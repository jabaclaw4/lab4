#include <iostream>
#include <limits>
#include <cstdlib>
#include <ctime>
#include "src/LazySequence.h"
#include "src/Generator.h"
#include "src/BinaryHeap.h"
#include "src/HeapSorter.h"
#include "src/ReadOnlyStream.h"
#include "src/WriteOnlyStream.h"
#include "src/MutableArraySequence.h"
#include "src/Cardinal.h"
#include "src/all_tests.h"

using namespace std;

LazySequence<int>*    g_lazy    = nullptr;
BinaryHeap<int>*      g_heap    = nullptr;

enum ActiveType { NONE, LAZY_FINITE, LAZY_INFINITE };
ActiveType g_active = NONE;


void clear_lazy() {
    if (g_lazy != nullptr) { delete g_lazy; g_lazy = nullptr; }
    g_active = NONE;
}

void clear_heap() {
    if (g_heap != nullptr) { delete g_heap; g_heap = nullptr; }
}

void print_lazy_n(LazySequence<int>* seq, int n) {
    cout << "[ ";
    for (int i = 0; i < n; i++) {
        cout << seq->Get(i);
        if (i < n - 1) cout << ", ";
    }
    cout << " ]" << endl;
}

void print_heap_sorted(BinaryHeap<int>& heap) {
    // печатаем не разрушая оригинал те копируем
    BinaryHeap<int> copy = heap;
    cout << "[ ";
    bool first = true;
    while (!copy.IsEmpty()) {
        if (!first) cout << ", ";
        cout << copy.ExtractMin();
        first = false;
    }
    cout << " ]" << endl;
}


void print_menu() {
    cout << "\n========================================" << endl;
    cout << "          LAB 4 - MENU" << endl;
    cout << "========================================" << endl;

    cout << "\n=== LAZY SEQUENCE ===" << endl;
    cout << "1  - Create finite LazySequence (from input)" << endl;
    cout << "2  - Create infinite: Fibonacci" << endl;
    cout << "3  - Create infinite: Natural numbers (1,2,3,...)" << endl;
    cout << "4  - Create infinite: Powers of 2" << endl;
    cout << "5  - Get element by index" << endl;
    cout << "6  - Print first N elements" << endl;
    cout << "7  - GetSubsequence [start, end]" << endl;
    cout << "8  - Map (* scalar)" << endl;
    cout << "9  - Where (filter even/odd)" << endl;
    cout << "10 - Concat two lazy sequences" << endl;
    cout << "11 - Reduce (sum, first N elements)" << endl;
    cout << "12 - GetCardinalLength" << endl;
    cout << "13 - GetMaterializedCount" << endl;

    cout << "\n=== BINARY HEAP ===" << endl;
    cout << "14 - Create empty BinaryHeap" << endl;
    cout << "15 - Insert element" << endl;
    cout << "16 - GetMin" << endl;
    cout << "17 - ExtractMin" << endl;
    cout << "18 - Print heap (sorted order)" << endl;
    cout << "19 - GetSize / IsEmpty" << endl;
    cout << "20 - Clear heap" << endl;

    cout << "\n=== HEAP SORTER (streams) ===" << endl;
    cout << "21 - Sort finite LazySequence via HeapSorter" << endl;
    cout << "22 - Sort from input array via HeapSorter" << endl;
    cout << "23 - Sort descending from input array" << endl;

    cout << "\n=== STREAMS ===" << endl;
    cout << "24 - ReadOnlyStream: read N elements from active LazySequence" << endl;
    cout << "25 - WriteOnlyStream: write N random ints, print result" << endl;

    cout << "\n=== CARDINAL ===" << endl;
    cout << "26 - Cardinal demo (finite vs infinite)" << endl;

    cout << "\n=== TESTS ===" << endl;
    cout << "30 - Run ALL tests" << endl;

    cout << "\n0  - Exit" << endl;
    cout << "========================================" << endl;
    cout << "Choice: ";
}


int main() {
    srand((unsigned)time(nullptr));

    cout << "\n========================================" << endl;
    cout << "       LABORATORY WORK #4" << endl;
    cout << "  LazySequence, Heap, Streams, Sort" << endl;
    cout << "========================================\n" << endl;

    int choice;
    while (true) {
        print_menu();

        if (!(cin >> choice)) {
            cout << "\nERROR: Invalid input!" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 0) {
            cout << "\nExiting... Goodbye!" << endl;
            break;
        }

        switch (choice) {

            case 1: {
                clear_lazy();
                int size;
                cout << "Enter size: ";
                if (!(cin >> size) || size < 0) {
                    cout << "ERROR: Invalid size!" << endl;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                int* arr = new int[size];
                cout << "Enter " << size << " integers:" << endl;
                for (int i = 0; i < size; i++) {
                    cout << "  [" << i << "]: ";
                    if (!(cin >> arr[i])) {
                        cout << "ERROR: Invalid input!" << endl;
                        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        delete[] arr;
                        goto end_case_1;
                    }
                }
                g_lazy = new LazySequence<int>(arr, size);
                delete[] arr;
                g_active = LAZY_FINITE;
                cout << "OK: Created finite LazySequence, length = " << size << endl;
                end_case_1:
                break;
            }

            case 2: {
                clear_lazy();
                int seed[] = {0, 1};
                auto fib_rule = [](const MutableArraySequence<int>& cache) -> int {
                    int n = cache.GetLength();
                    return cache.Get(n - 1) + cache.Get(n - 2);
                };
                g_lazy = new LazySequence<int>(
                        Generator<int>(fib_rule, 2), seed, 2, true
                );
                g_active = LAZY_INFINITE;
                cout << "OK: Created infinite Fibonacci LazySequence" << endl;
                cout << "First 10: "; print_lazy_n(g_lazy, 10);
                break;
            }

            case 3: {
                clear_lazy();
                //натуральные числа
                auto nat_rule = [](const MutableArraySequence<int>& cache) -> int {
                    return cache.GetLength() + 1;
                };
                g_lazy = new LazySequence<int>(Generator<int>(nat_rule), true);
                g_active = LAZY_INFINITE;
                cout << "OK: Created infinite Natural numbers LazySequence" << endl;
                cout << "First 10: "; print_lazy_n(g_lazy, 10);
                break;
            }

            case 4: {
                clear_lazy();
                //степени двойки
                auto pow2_rule = [](const MutableArraySequence<int>& cache) -> int {
                    int n = cache.GetLength();
                    return cache.Get(n - 1) * 2;
                };
                int seed[] = {1};
                g_lazy = new LazySequence<int>(
                        Generator<int>(pow2_rule, 1), seed, 1, true
                );
                g_active = LAZY_INFINITE;
                cout << "OK: Created infinite Powers-of-2 LazySequence" << endl;
                cout << "First 10: "; print_lazy_n(g_lazy, 10);
                break;
            }

            case 5: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int idx;
                cout << "Enter index: ";
                if (!(cin >> idx) || idx < 0) {
                    cout << "ERROR: Invalid index!" << endl;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                try {
                    cout << "Get(" << idx << ") = " << g_lazy->Get(idx) << endl;
                } catch (const exception& e) {
                    cout << "ERROR: " << e.what() << endl;
                }
                break;
            }

            case 6: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int n;
                cout << "Print first N elements, enter N: ";
                if (!(cin >> n) || n <= 0) {
                    cout << "ERROR: Invalid N!" << endl;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                print_lazy_n(g_lazy, n);
                cout << "Materialized count: " << g_lazy->GetMaterializedCount() << endl;
                break;
            }

            case 7: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int s, e;
                cout << "Enter start index: "; cin >> s;
                cout << "Enter end index:   "; cin >> e;
                try {
                    Sequence<int>* sub = g_lazy->GetSubsequence(s, e);
                    cout << "Subsequence [" << s << ".." << e << "]: [ ";
                    for (int i = 0; i < sub->GetLength(); i++) {
                        cout << sub->Get(i);
                        if (i < sub->GetLength() - 1) cout << ", ";
                    }
                    cout << " ]" << endl;
                    delete sub;
                } catch (const exception& ex) {
                    cout << "ERROR: " << ex.what() << endl;
                }
                break;
            }

            case 8: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int scalar, n;
                cout << "Enter scalar: "; cin >> scalar;
                cout << "Print first N elements after Map: "; cin >> n;
                int sc = scalar;
                Sequence<int>* mapped = g_lazy->Map([](int x) -> int { return x; }); // placeholder
                //Map через лямбду нельзя напрямую поэтому используем свой вариант
                //и создаём новую LazySequence вручную через генератор
                delete mapped;
                LazySequence<int>* src = g_lazy;
                auto map_rule = [src, sc](int i) -> int {
                    return src->Get(i) * sc;
                };
                LazySequence<int>* result = new LazySequence<int>(
                        Generator<int>::FromIndexFunction(map_rule),
                        g_active == LAZY_INFINITE
                );
                cout << "Map(x * " << scalar << "), first " << n << ": ";
                print_lazy_n(result, n);
                delete result;
                break;
            }

            case 9: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int choice2, n;
                cout << "Filter: 1 - even, 2 - odd: "; cin >> choice2;
                cout << "How many matching elements to print: "; cin >> n;
                try {
                    LazySequence<int>* filtered;
                    if (choice2 == 1) {
                        filtered = g_lazy->Where([](int x) { return x % 2 == 0; }, n * 10);
                    } else {
                        filtered = g_lazy->Where([](int x) { return x % 2 != 0; }, n * 10);
                    }
                    int len = filtered->GetLength();
                    int print_count = (len < n) ? len : n;
                    cout << "First " << print_count << " filtered: ";
                    print_lazy_n(filtered, print_count);
                    delete filtered;
                } catch (const exception& ex) {
                    cout << "ERROR: " << ex.what() << endl;
                }
                break;
            }

            case 10: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int size2;
                cout << "Enter size of second sequence: "; cin >> size2;
                int* arr2 = new int[size2];
                for (int i = 0; i < size2; i++) {
                    cout << "  [" << i << "]: "; cin >> arr2[i];
                }
                LazySequence<int> second(arr2, size2);
                delete[] arr2;
                int n;
                cout << "Print first N elements of concat: "; cin >> n;
                try {
                    Sequence<int>* cat = g_lazy->Concat(&second);
                    LazySequence<int>* lazyCat = dynamic_cast<LazySequence<int>*>(cat);
                    if (lazyCat) {
                        print_lazy_n(lazyCat, n);
                    } else {
                        cout << "[ ";
                        for (int i = 0; i < n && i < cat->GetLength(); i++) {
                            cout << cat->Get(i);
                            if (i < n - 1) cout << ", ";
                        }
                        cout << " ]" << endl;
                    }
                    delete cat;
                } catch (const exception& ex) {
                    cout << "ERROR: " << ex.what() << endl;
                }
                break;
            }

            case 11: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int n;
                cout << "Reduce (sum) first N elements, enter N: "; cin >> n;
                try {
                    int sum = g_lazy->Reduce([](int a, int b) { return a + b; }, 0, n);
                    cout << "Sum of first " << n << " elements = " << sum << endl;
                } catch (const exception& ex) {
                    cout << "ERROR: " << ex.what() << endl;
                }
                break;
            }

            case 12: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                Cardinal card = g_lazy->GetCardinalLength();
                cout << "CardinalLength = " << card.ToString() << endl;
                break;
            }

            case 13: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                cout << "MaterializedCount = " << g_lazy->GetMaterializedCount() << endl;
                break;
            }

            case 14: {
                clear_heap();
                g_heap = new BinaryHeap<int>();
                cout << "OK: Created empty BinaryHeap<int> (min-heap)" << endl;
                break;
            }

            case 15: {
                if (g_heap == nullptr) { cout << "ERROR: No active heap! Create one first (14)." << endl; break; }
                int val;
                cout << "Enter value to insert: ";
                if (!(cin >> val)) {
                    cout << "ERROR: Invalid input!" << endl;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                g_heap->Insert(val);
                cout << "OK: Inserted " << val << ", heap size = " << g_heap->GetSize() << endl;
                break;
            }

            case 16: {
                if (g_heap == nullptr || g_heap->IsEmpty()) {
                    cout << "ERROR: Heap is empty or not created!" << endl; break;
                }
                cout << "GetMin() = " << g_heap->GetMin() << endl;
                break;
            }

            case 17: {
                if (g_heap == nullptr || g_heap->IsEmpty()) {
                    cout << "ERROR: Heap is empty or not created!" << endl; break;
                }
                int val = g_heap->ExtractMin();
                cout << "ExtractMin() = " << val << ", remaining size = " << g_heap->GetSize() << endl;
                break;
            }

            case 18: {
                if (g_heap == nullptr || g_heap->IsEmpty()) {
                    cout << "ERROR: Heap is empty or not created!" << endl; break;
                }
                cout << "Heap (sorted order): ";
                print_heap_sorted(*g_heap);
                break;
            }

            case 19: {
                if (g_heap == nullptr) { cout << "ERROR: No active heap!" << endl; break; }
                cout << "Size    = " << g_heap->GetSize() << endl;
                cout << "IsEmpty = " << (g_heap->IsEmpty() ? "true" : "false") << endl;
                break;
            }

            case 20: {
                if (g_heap == nullptr) { cout << "ERROR: No active heap!" << endl; break; }
                g_heap->Clear();
                cout << "OK: Heap cleared (size = 0)" << endl;
                break;
            }

            case 21: {
                if (g_lazy == nullptr || g_active != LAZY_FINITE) {
                    cout << "ERROR: Need active finite LazySequence (create with option 1)!" << endl;
                    break;
                }
                try {
                    int len = g_lazy->GetLength();
                    MutableArraySequence<int> output;
                    ReadOnlyStream<int>  in(g_lazy, len);
                    WriteOnlyStream<int> out(&output);
                    in.Open(); out.Open();
                    HeapSorter<int> sorter([](const int& a, const int& b){ return a < b; });
                    sorter.Sort(in, out);
                    in.Close(); out.Close();
                    cout << "Sorted: [ ";
                    for (int i = 0; i < output.GetLength(); i++) {
                        cout << output.Get(i);
                        if (i < output.GetLength() - 1) cout << ", ";
                    }
                    cout << " ]" << endl;
                } catch (const exception& ex) {
                    cout << "ERROR: " << ex.what() << endl;
                }
                break;
            }

            case 22:
            case 23: {
                int size;
                cout << "Enter array size: ";
                if (!(cin >> size) || size <= 0) {
                    cout << "ERROR: Invalid size!" << endl;
                    cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                MutableArraySequence<int> input;
                cout << "Enter " << size << " integers:" << endl;
                for (int i = 0; i < size; i++) {
                    int v; cout << "  [" << i << "]: "; cin >> v;
                    input.Append(v);
                }
                MutableArraySequence<int> output;
                ReadOnlyStream<int>  in(&input);
                WriteOnlyStream<int> out(&output);
                in.Open(); out.Open();
                bool asc = (choice == 22);
                if (asc) {
                    HeapSorter<int> sorter([](const int& a, const int& b){ return a < b; });
                    sorter.Sort(in, out);
                } else {
                    HeapSorter<int> sorter([](const int& a, const int& b){ return a > b; });
                    sorter.Sort(in, out);
                }
                in.Close(); out.Close();
                cout << "Sorted " << (asc ? "ascending" : "descending") << ": [ ";
                for (int i = 0; i < output.GetLength(); i++) {
                    cout << output.Get(i);
                    if (i < output.GetLength() - 1) cout << ", ";
                }
                cout << " ]" << endl;
                break;
            }

            case 24: {
                if (g_lazy == nullptr) { cout << "ERROR: No active LazySequence!" << endl; break; }
                int n;
                cout << "How many elements to read from stream: "; cin >> n;
                ReadOnlyStream<int> in(g_lazy, n);
                in.Open();
                cout << "Reading " << n << " elements: [ ";
                for (int i = 0; i < n; i++) {
                    cout << in.Read();
                    if (i < n - 1) cout << ", ";
                }
                cout << " ]" << endl;
                cout << "Stream position: " << in.GetPosition() << endl;
                in.Close();
                break;
            }

            case 25: {
                int n;
                cout << "How many random ints to write: "; cin >> n;
                MutableArraySequence<int> buf;
                WriteOnlyStream<int> out(&buf);
                out.Open();
                for (int i = 0; i < n; i++) {
                    out.Write(rand() % 100);
                }
                out.Close();
                cout << "Written " << buf.GetLength() << " elements: [ ";
                for (int i = 0; i < buf.GetLength(); i++) {
                    cout << buf.Get(i);
                    if (i < buf.GetLength() - 1) cout << ", ";
                }
                cout << " ]" << endl;
                break;
            }

            case 26: {
                cout << "\n=== Cardinal Demo ===" << endl;
                Cardinal c5(5);
                Cardinal c10(10);
                Cardinal inf = Cardinal::Infinity();
                cout << "c5  = " << c5.ToString()  << endl;
                cout << "c10 = " << c10.ToString() << endl;
                cout << "inf = " << inf.ToString()  << endl;
                cout << "c5 < c10  : " << (c5 < c10  ? "true" : "false") << endl;
                cout << "c10 < inf : " << (c10 < inf  ? "true" : "false") << endl;
                cout << "inf < c5  : " << (inf < c5   ? "true" : "false") << endl;
                cout << "inf == inf: " << (inf == Cardinal::Infinity() ? "true" : "false") << endl;
                break;
            }

            case 30: {
                run_all_tests();
                break;
            }

            default:
                cout << "ERROR: Unknown option!" << endl;
                break;
        }
    }

    clear_lazy();
    clear_heap();
    return 0;
}