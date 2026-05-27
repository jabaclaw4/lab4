#include <iostream>
#include "../src/LazySequence.h"
#include "../src/Generator.h"
#include "../src/test_utils.h"

using namespace std;

void run_test_lazy_sequence() {
    reset_counters();
    cout << "=== LazySequence Tests ===" << endl;

    //создание из массива
    int arr[] = {10, 20, 30, 40, 50};
    LazySequence<int> seq(arr, 5);

    check(seq.Get(0) == 10, "array ctor: Get(0) = 10");
    check(seq.Get(4) == 50, "array ctor: Get(4) = 50");
    check(seq.GetLength() == 5, "array ctor: length = 5");
    check(seq.GetFirst() == 10, "array ctor: GetFirst = 10");
    check(seq.GetLast() == 50,  "array ctor: GetLast = 50");

    //после Get(4) все 5 элементов должны быть материализованы
    check(seq.GetMaterializedCount() == 5, "memoization: materialized count = 5 after Get(4)");

    //повторный Get не должен менять счётчик (берётся из кэша)
    seq.Get(2);
    check(seq.GetMaterializedCount() == 5, "memoization: count stays 5 after repeated Get(2)");

    //бесконечная последовательность числа Фибоначчи
    auto fibRule = [](const MutableArraySequence<int>& cache) -> int {
        int n = cache.GetLength();
        if (n == 0) return 1;
        if (n == 1) return 1;
        return cache.Get(n - 1) + cache.Get(n - 2);
    };

    LazySequence<int> fib(Generator<int>(fibRule), true);

    check(fib.Get(0) == 1,  "fibonacci: fib(0) = 1");
    check(fib.Get(1) == 1,  "fibonacci: fib(1) = 1");
    check(fib.Get(2) == 2,  "fibonacci: fib(2) = 2");
    check(fib.Get(3) == 3,  "fibonacci: fib(3) = 3");
    check(fib.Get(4) == 5,  "fibonacci: fib(4) = 5");
    check(fib.Get(9) == 55, "fibonacci: fib(9) = 55");
    check(fib.IsInfinite(),  "fibonacci: IsInfinite = true");

    //бесконечная факториалы

    auto factRule = [](const MutableArraySequence<int>& cache) -> int {
        int n = cache.GetLength(); // n = индекс следующего элемента
        if (n == 0) return 1;     // 0! = 1
        return cache.Get(n - 1) * (n);
    };

    LazySequence<int> fact(Generator<int>(factRule), true);

    check(fact.Get(0) == 1,   "factorial: 0! = 1");
    check(fact.Get(1) == 1,   "factorial: 1! = 1");
    check(fact.Get(2) == 2,   "factorial: 2! = 2");
    check(fact.Get(3) == 6,   "factorial: 3! = 6");
    check(fact.Get(4) == 24,  "factorial: 4! = 24");
    check(fact.Get(5) == 120, "factorial: 5! = 120");
    Cardinal finiteCar = seq.GetCardinalLength();
    check(!finiteCar.IsInfinite(),    "cardinal: finite seq is not infinite");
    check(finiteCar.GetValue() == 5,  "cardinal: finite seq cardinal = 5");
    Cardinal infCar = fib.GetCardinalLength();
    check(infCar.IsInfinite(), "cardinal: infinite seq cardinal = ∞");

    Sequence<int>* sub = seq.GetSubsequence(1, 3);
    check(sub->GetLength() == 3, "subsequence: length = 3");
    check(sub->Get(0) == 20,     "subsequence: [0] = 20");
    check(sub->Get(1) == 30,     "subsequence: [1] = 30");
    check(sub->Get(2) == 40,     "subsequence: [2] = 40");
    delete sub;

    Sequence<int>* doubled = seq.Map([](int x) { return x * 2; });
    check(doubled->Get(0) == 20,  "map: [0] = 20");
    check(doubled->Get(2) == 60,  "map: [2] = 60");
    check(doubled->Get(4) == 100, "map: [4] = 100");
    delete doubled;

    //Map не должен менять оригинал
    check(seq.Get(0) == 10, "map: original[0] still 10");
    Sequence<int>* evens = seq.Where([](int x) { return x % 20 == 0; });
    // из {10,20,30,40,50} чётные десятки кратные 20: 20, 40
    check(evens->Get(0) == 20, "where: [0] = 20");
    check(evens->Get(1) == 40, "where: [1] = 40");
    delete evens;

    int sum = seq.Reduce([](int a, int b) { return a + b; }, 0);
    check(sum == 150, "reduce: sum {10..50} = 150");

    int product = seq.Reduce([](int a, int b) { return a * b; }, 1);
    check(product == 12000000, "reduce: product {10..50} = 12000000");

    //Reduce с лимитом на бесконечной
    int fibSum5 = fib.Reduce([](int a, int b) { return a + b; }, 0, 5);
    // fib: 1+1+2+3+5 = 12
    check(fibSum5 == 12, "reduce(limit=5): fib sum = 12");

    //Concat конечных

    int arr2[] = {60, 70};
    LazySequence<int> seq2(arr2, 2);
    Sequence<int>* cat = seq.Concat(&seq2);

    check(cat->Get(0) == 10, "concat: [0] = 10");
    check(cat->Get(4) == 50, "concat: [4] = 50");
    check(cat->Get(5) == 60, "concat: [5] = 60");
    check(cat->Get(6) == 70, "concat: [6] = 70");
    delete cat;

    //Append Prepend  InsertAt (конечная)

    LazySequence<int> mut;
    mut.Append(1);
    mut.Append(2);
    mut.Append(3);
    check(mut.GetLength() == 3, "append: length = 3");
    check(mut.Get(2) == 3,      "append: [2] = 3");

    mut.Prepend(0);
    check(mut.GetLength() == 4, "prepend: length = 4");
    check(mut.Get(0) == 0,      "prepend: [0] = 0");

    mut.InsertAt(99, 2);
    check(mut.GetLength() == 5, "insertAt: length = 5");
    check(mut.Get(2) == 99,     "insertAt: [2] = 99");

    //граничные

    bool caught = false;
    try { seq.Get(-1); }
    catch (const std::out_of_range&) { caught = true; }
    check(caught, "exception: Get(-1) throws out_of_range");


    caught = false;
    try { fib.Append(999); }
    catch (const std::logic_error&) { caught = true; }
    check(caught, "exception: Append to infinite throws logic_error");

    caught = false;
    try { fib.GetLast(); }
    catch (const std::logic_error&) { caught = true; }
    check(caught, "exception: GetLast on infinite throws logic_error");

    print_results();
}