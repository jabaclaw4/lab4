#ifndef HEAP_SORTER_H
#define HEAP_SORTER_H

#include "BinaryHeap.h"
#include "ReadOnlyStream.h"
#include "WriteOnlyStream.h"
#include <functional>
#include <stdexcept>

//сортировка потока данных через бинарную кучу
//читает из ReadOnlyStream, сортирует, пишет в WriteOnlyStream
//сложность: O(n log n) по времени, O(n) по памяти
template <class T>
class HeapSorter {
private:
    //компаратор: возвращает true если a "меньше" b
    std::function<bool(const T&, const T&)> less;

public:
    //конструктор по умолчанию — сортировка по возрастанию
    HeapSorter()
            : less([](const T& a, const T& b) { return a < b; }) {}

    //конструктор с пользовательским компаратором
    //пример для сортировки по убыванию:
    //  HeapSorter<int>([](const int& a, const int& b) { return a > b; })
    explicit HeapSorter(std::function<bool(const T&, const T&)> comparator)
            : less(comparator) {}

    //отсортировать поток
    //input  — источник данных (уже открытый)
    //output — куда писать результат (уже открытый)
    //maxElements — сколько элементов читать (-1 = до конца потока)
    void Sort(ReadOnlyStream<T>& input,
              WriteOnlyStream<T>& output,
              int maxElements = -1) {

        BinaryHeap<T> heap(less);

        //фаза 1: читаем всё из потока в кучу
        int read = 0;
        while (!input.IsEndOfStream()) {
            if (maxElements != -1 && read >= maxElements) {
                break;
            }
            heap.Insert(input.Read());
            read++;
        }

        if (read == 0) {
            throw std::runtime_error("input stream is empty");
        }

        //фаза 2: извлекаем из кучи в выходной поток
        //каждый ExtractMin даёт следующий минимум -> результат отсортирован
        while (!heap.IsEmpty()) {
            output.Write(heap.ExtractMin());
        }
    }

    //удобный вариант: сортировать Sequence -> возвращает новую Sequence
    //создаёт потоки внутри сам
    MutableArraySequence<T>* SortSequence(const Sequence<T>* input) {
        //поток чтения из входной последовательности
        ReadOnlyStream<T> inStream(input);
        MutableArraySequence<T>* result = new MutableArraySequence<T>();
        WriteOnlyStream<T> outStream(result);

        inStream.Open();
        outStream.Open();

        Sort(inStream, outStream);

        inStream.Close();
        outStream.Close();

        return result;
    }

    //сортировка LazySequence первых count элементов
    MutableArraySequence<T>* SortLazy(LazySequence<T>* input, int count) {
        if (count <= 0) {
            throw std::invalid_argument("count must be positive");
        }

        ReadOnlyStream<T> inStream(input, count);
        MutableArraySequence<T>* result = new MutableArraySequence<T>();
        WriteOnlyStream<T> outStream(result);

        inStream.Open();
        outStream.Open();

        Sort(inStream, outStream, count);

        inStream.Close();
        outStream.Close();

        return result;
    }
};

#endif