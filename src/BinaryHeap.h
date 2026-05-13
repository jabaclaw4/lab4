#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include "MutableArraySequence.h"
#include <functional>
#include <stdexcept>
//сама сортировочка
//минимальная бинарная куча (min-heap) родитель всегда <= детей значит минимум всегда на вершине
template <class T>
class BinaryHeap {
private:
    MutableArraySequence<T> data;
    //логический размер кучи элементы за этой границей игнорируются
    int heapSize; //физически массив не уменьшается просто колво которое видит куча
    std::function<bool(const T&, const T&)> less;//определяет че значит меньше

    int parentIndex(int i) const {
        return (i - 1) / 2;
    }

    int leftChild(int i) const {
        return 2 * i + 1;
    }

    int rightChild(int i) const {
        return 2 * i + 2;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = parentIndex(i);
            if (less(data.Get(i), data.Get(parent))) {
                swapAt(i, parent);
                i = parent;
            } else {
                break;
            }
        }
    }

    //теперь siftDown использует heapSize а не data.GetLength()
    //именно поэтому ExtractMin больше не пересоздаёт массив —
    //просто уменьшаем heapSize и элемент "исчезает" логически
    void siftDown(int i) {
        while (true) {
            int left = leftChild(i);
            int right = rightChild(i);
            int smallest = i;

            if (left < heapSize && less(data.Get(left), data.Get(smallest))) {
                smallest = left;
            }
            if (right < heapSize && less(data.Get(right), data.Get(smallest))) {
                smallest = right;
            }

            if (smallest == i) {
                break;
            }

            swapAt(i, smallest);
            i = smallest;
        }
    }

    void swapAt(int i, int j) {
        T temp = data.Get(i);
        data.Set(i, data.Get(j));
        data.Set(j, temp);
    }

public:
    BinaryHeap()
            : heapSize(0),
              less([](const T& a, const T& b) { return a < b; }) {}

    explicit BinaryHeap(std::function<bool(const T&, const T&)> comparator)
            : heapSize(0), less(comparator) {}

    //добавить элемент
    //O(log n)
    void Insert(const T& item) {
        if (heapSize < data.GetLength()) {
            //есть место в уже выделенном массиве просто записываем
            data.Set(heapSize, item);
        } else {
            //массив заполнен расширяемс
            data.Append(item);
        }
        siftUp(heapSize);
        heapSize++;
    }

    //посмотреть минимум без удаления
    //O(1)
    T GetMin() const {
        if (heapSize == 0) {
            throw std::out_of_range("heap is empty");
        }
        return data.Get(0);
    }

    //O(log n) без пересоздания массива
    T ExtractMin() {
        if (heapSize == 0) {
            throw std::out_of_range("heap is empty");
        }

        T minVal = data.Get(0);

        //ставим последний элемент на вершину
        heapSize--;
        if (heapSize > 0) {
            data.Set(0, data.Get(heapSize));
            siftDown(0);
        }

        return minVal;
    }

    int GetSize() const {
        return heapSize;
    }

    bool IsEmpty() const {
        return heapSize == 0;
    }

    //сбросить кучу (не освобождает память то есть для переиспользования) типа память уже выделена повторно нен надо
    void Clear() {
        heapSize = 0;
    }
};

#endif