#include <iostream>
#include <fstream>
#include "../src/ReadOnlyStream.h"
#include "../src/WriteOnlyStream.h"
#include "../src/MutableArraySequence.h"
#include "../src/LazySequence.h"
#include "../src/test_utils.h"

using namespace std;

void run_test_streams() {
    reset_counters();
    cout << "=== Stream Tests ===" << endl;


    int arr[] = {1, 2, 3, 4, 5};
    MutableArraySequence<int> baseSeq;
    for (int x : arr) baseSeq.Append(x);

    ReadOnlyStream<int> rs(&baseSeq);
    rs.Open();

    check(!rs.IsEndOfStream(),   "read stream: not end at start");
    check(rs.GetPosition() == 0, "read stream: position = 0 at start");

    int first = rs.Read();
    check(first == 1,            "read stream: Read() = 1");
    check(rs.GetPosition() == 1, "read stream: position = 1 after read");

    rs.Read(); rs.Read(); rs.Read();
    int last = rs.Read();
    check(last == 5,             "read stream: Read() 5th = 5");
    check(rs.IsEndOfStream(),    "read stream: end of stream after 5 reads");

    //исключение при чтении за концом

    bool caught = false;
    try { rs.Read(); }
    catch (const std::out_of_range&) { caught = true; }
    check(caught, "read stream: Read() past end throws out_of_range");

    rs.Close();

    //Read без Open
    ReadOnlyStream<int> rs2(&baseSeq);
    caught = false;
    try { rs2.Read(); }
    catch (const std::runtime_error&) { caught = true; }
    check(caught, "read stream: Read() without Open throws runtime_error");



    ReadOnlyStream<int> rs3(&baseSeq);
    rs3.Open();
    rs3.Seek(3);
    check(rs3.GetPosition() == 3, "seek: position = 3 after Seek(3)");
    int val = rs3.Read();
    check(val == 4, "seek: Read() after Seek(3) = 4");
    rs3.Close();

    ReadOnlyStream<int> rs4(&baseSeq);
    check(rs4.IsCanSeek(), "seek: IsCanSeek() = true for Sequence source");

    //ReadOnlyStream из LazySequence
    auto fibRule = [](const MutableArraySequence<int>& c) -> int {
        int n = c.GetLength();
        if (n == 0) return 1;
        if (n == 1) return 1;
        return c.Get(n-1) + c.Get(n-2);
    };
    LazySequence<int> fib(Generator<int>(fibRule), true);

    //читаем первые 6 чисел Фибоначчи: 1 1 2 3 5 8
    ReadOnlyStream<int> fibStream(&fib, 6);
    fibStream.Open();
    int fibVals[6];
    for (int i = 0; i < 6; i++) fibVals[i] = fibStream.Read();
    fibStream.Close();

    check(fibVals[0] == 1, "lazy stream: fib[0] = 1");
    check(fibVals[1] == 1, "lazy stream: fib[1] = 1");
    check(fibVals[2] == 2, "lazy stream: fib[2] = 2");
    check(fibVals[4] == 5, "lazy stream: fib[4] = 5");
    check(fibVals[5] == 8, "lazy stream: fib[5] = 8");

    MutableArraySequence<int> target;
    WriteOnlyStream<int> ws(&target);
    ws.Open();

    int pos1 = ws.Write(10);
    int pos2 = ws.Write(20);
    int pos3 = ws.Write(30);

    check(pos1 == 1,               "write stream: Write returns 1");
    check(pos2 == 2,               "write stream: Write returns 2");
    check(pos3 == 3,               "write stream: Write returns 3");
    check(ws.GetPosition() == 3,   "write stream: position = 3");
    check(target.GetLength() == 3, "write stream: target length = 3");
    check(target.Get(0) == 10,     "write stream: target[0] = 10");
    check(target.Get(2) == 30,     "write stream: target[2] = 30");

    ws.Close();

    //Write без Open
    MutableArraySequence<int> target2;
    WriteOnlyStream<int> ws2(&target2);
    caught = false;
    try { ws2.Write(42); }
    catch (const std::runtime_error&) { caught = true; }
    check(caught, "write stream: Write() without Open throws runtime_error");

    //двойной Open
    ReadOnlyStream<int> rs5(&baseSeq);
    rs5.Open();
    caught = false;
    try { rs5.Open(); }
    catch (const std::runtime_error&) { caught = true; }
    check(caught, "read stream: double Open throws runtime_error");
    rs5.Close();

    {
        WriteOnlyStream<int> wsFile(
                "test_output.txt",
                [](const int& x) { return to_string(x); }
        );
        wsFile.Open();
        wsFile.Write(100);
        wsFile.Write(200);
        wsFile.Write(300);
        wsFile.Close();
    }

    //читаем обратно через ReadOnlyStream
    {
        ReadOnlyStream<int> rsFile(
                "test_output.txt",
                [](const string& s) { return stoi(s); }
        );
        rsFile.Open();
        int a = rsFile.Read();
        int b = rsFile.Read();
        int c = rsFile.Read();
        rsFile.Close();

        check(a == 100, "file round-trip: [0] = 100");
        check(b == 200, "file round-trip: [1] = 200");
        check(c == 300, "file round-trip: [2] = 300");
    }

    print_results();
}