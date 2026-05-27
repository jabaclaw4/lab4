#include <iostream>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include "../src/HeapSorter.h"
#include "../src/ReadOnlyStream.h"
#include "../src/WriteOnlyStream.h"
#include "../src/MutableArraySequence.h"
#include "../src/test_utils.h"

using namespace std;
using namespace std::chrono;

static MutableArraySequence<int> makeRandom(int n) {
    MutableArraySequence<int> seq;
    for (int i = 0; i < n; i++) {
        seq.Append(rand() % 1000000);
    }
    return seq;
}

static bool isSorted(const MutableArraySequence<int>& seq) {
    for (int i = 1; i < seq.GetLength(); i++) {
        if (seq.Get(i) < seq.Get(i - 1)) return false;
    }
    return true;
}

void run_test_heap_sorter() {
    reset_counters();
    cout << "=== HeapSorter Tests ===" << endl;

    //пустая последовательность
    MutableArraySequence<int> empty;
    ReadOnlyStream<int> emptyIn(&empty);
    MutableArraySequence<int> emptyOut;
    WriteOnlyStream<int> emptyWs(&emptyOut);

    emptyIn.Open(); emptyWs.Open();
    bool threw = false;
    try {
        HeapSorter<int> emptySorter([](int a, int b){ return a < b; });
        emptySorter.Sort(emptyIn, emptyWs);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    emptyIn.Close(); emptyWs.Close();

    check(threw, "sort empty: throws runtime_error");

    //один элемент

    MutableArraySequence<int> single;
    single.Append(42);
    ReadOnlyStream<int> singleIn(&single);
    MutableArraySequence<int> singleOut;
    WriteOnlyStream<int> singleWs(&singleOut);

    singleIn.Open(); singleWs.Open();
    HeapSorter<int> singleSorter([](int a, int b){ return a < b; });
    singleSorter.Sort(singleIn, singleWs);
    singleIn.Close(); singleWs.Close();

    check(singleOut.GetLength() == 1,  "sort single: length = 1");
    check(singleOut.Get(0) == 42,      "sort single: [0] = 42");

    //уже отсортированный
    int sorted[] = {1, 2, 3, 4, 5};
    MutableArraySequence<int> sortedSeq;
    for (int x : sorted) sortedSeq.Append(x);

    ReadOnlyStream<int> sortedIn(&sortedSeq);
    MutableArraySequence<int> sortedOut;
    WriteOnlyStream<int> sortedWs(&sortedOut);

    sortedIn.Open(); sortedWs.Open();
    HeapSorter<int> sortedSorter([](int a, int b){ return a < b; });
    sortedSorter.Sort(sortedIn, sortedWs);
    sortedIn.Close(); sortedWs.Close();

    check(isSorted(sortedOut),         "sort already sorted: output is sorted");
    check(sortedOut.GetLength() == 5,  "sort already sorted: length = 5");

    //обратный порядок
    int rev[] = {5, 4, 3, 2, 1};
    MutableArraySequence<int> revSeq;
    for (int x : rev) revSeq.Append(x);

    ReadOnlyStream<int> revIn(&revSeq);
    MutableArraySequence<int> revOut;
    WriteOnlyStream<int> revWs(&revOut);

    revIn.Open(); revWs.Open();
    HeapSorter<int> revSorter([](int a, int b){ return a < b; });
    revSorter.Sort(revIn, revWs);
    revIn.Close(); revWs.Close();

    check(isSorted(revOut),      "sort reversed: output is sorted");
    check(revOut.Get(0) == 1,    "sort reversed: [0] = 1");
    check(revOut.Get(4) == 5,    "sort reversed: [4] = 5");

    //дубликаты
    int dups[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    MutableArraySequence<int> dupSeq;
    for (int x : dups) dupSeq.Append(x);

    ReadOnlyStream<int> dupIn(&dupSeq);
    MutableArraySequence<int> dupOut;
    WriteOnlyStream<int> dupWs(&dupOut);

    dupIn.Open(); dupWs.Open();
    HeapSorter<int> dupSorter([](int a, int b){ return a < b; });
    dupSorter.Sort(dupIn, dupWs);
    dupIn.Close(); dupWs.Close();

    check(isSorted(dupOut),        "sort with dups: output is sorted");
    check(dupOut.GetLength() == 10,"sort with dups: length = 10");
    check(dupOut.Get(0) == 1,      "sort with dups: min = 1");
    check(dupOut.Get(9) == 9,      "sort with dups: max = 9");

    //сортировка по убыванию
    int arr[] = {3, 1, 4, 1, 5};
    MutableArraySequence<int> descSeq;
    for (int x : arr) descSeq.Append(x);

    ReadOnlyStream<int> descIn(&descSeq);
    MutableArraySequence<int> descOut;
    WriteOnlyStream<int> descWs(&descOut);

    descIn.Open(); descWs.Open();
    HeapSorter<int> descSorter([](int a, int b){ return a > b; });
    descSorter.Sort(descIn, descWs);
    descIn.Close(); descWs.Close();

    check(descOut.Get(0) >= descOut.Get(1), "sort desc: [0] >= [1]");
    check(descOut.Get(0) == 5,              "sort desc: max first = 5");
    check(descOut.Get(4) == 1,              "sort desc: min last = 1");

    //средний случай: 1000 случайных
    srand(42);
    MutableArraySequence<int> rand1k = makeRandom(1000);
    ReadOnlyStream<int> rand1kIn(&rand1k);
    MutableArraySequence<int> rand1kOut;
    WriteOnlyStream<int> rand1kWs(&rand1kOut);

    rand1kIn.Open(); rand1kWs.Open();
    HeapSorter<int> rand1kSorter([](int a, int b){ return a < b; });
    rand1kSorter.Sort(rand1kIn, rand1kWs);
    rand1kIn.Close(); rand1kWs.Close();

    check(isSorted(rand1kOut),          "sort 1000 random: output is sorted");
    check(rand1kOut.GetLength() == 1000,"sort 1000 random: length = 1000");

    print_results();

    //замер времени

    cout << "\n=== Complexity Check ===" << endl;

    int sizes[] = {1000, 10000, 100000, 1000000};
    double times[4];

    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        srand(12345);
        MutableArraySequence<int> input = makeRandom(n);
        MutableArraySequence<int> output;
        ReadOnlyStream<int> in(&input);
        WriteOnlyStream<int> out(&output);

        in.Open(); out.Open();
        HeapSorter<int> perfSorter([](int a, int b){ return a < b; });
        auto t1 = high_resolution_clock::now();
        perfSorter.Sort(in, out);
        auto t2 = high_resolution_clock::now();
        in.Close(); out.Close();

        times[s] = duration<double, milli>(t2 - t1).count();
        cout << "  n = " << n
             << "\t time = " << times[s] << " ms" << endl;
    }

    // проверяем что соотношения времён соответствуют O(n log n) те T(10n) / T(n) должно быть близко к 10 * log2(10n) / log2(n)
    cout << "\n  Ratio check (actual vs expected ~O(n log n)):" << endl;
    for (int s = 1; s < 4; s++) {
        double n0 = sizes[s - 1];
        double n1 = sizes[s];
        double actualRatio   = times[s] / times[s - 1];
        double expectedRatio = (n1 * log2(n1)) / (n0 * log2(n0));
        double deviation     = fabs(actualRatio - expectedRatio) / expectedRatio;

        cout << "  n=" << (int)n0 << " -> n=" << (int)n1
             << ": actual=" << actualRatio
             << " expected=" << expectedRatio
             << " deviation=" << (deviation * 100) << "%" << endl;

        check(deviation < 0.5,
              "O(n log n): ratio within 50% of expected");
    }
}