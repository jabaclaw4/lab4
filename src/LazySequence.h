#ifndef LAZY_SEQUENCE_H
#define LAZY_SEQUENCE_H

#include "MutableArraySequence.h"
#include "Generator.h"
#include <stdexcept>

template <class T>
class LazySequence {
private:
    MutableArraySequence<T> cache;//уже вычисл
    Generator<T> generator;//праивло вычисл следщ
    bool isInfinite; //беск или нет

    void materializeUpTo(int index) {
        while (cache.GetLength() <= index) {
            if (!generator.HasRule()) {
                throw std::out_of_range("index out of range: sequence is finite");
            }
            T next = generator.GetNext(cache);
            cache.Append(next);
        }
    }

public:
    //пустая конечная последовательность
    LazySequence() : isInfinite(false) {}

    //из массива
    LazySequence(T* items, int count) : isInfinite(false) {
        for (int i = 0; i < count; i++) {
            cache.Append(items[i]);
        }
    }

    //из Sequence
    explicit LazySequence(const Sequence<T>* seq) : isInfinite(false) {
        for (int i = 0; i < seq->GetLength(); i++) {
            cache.Append(seq->Get(i));
        }
    }

    //бесконечная с генератором (весь кэш)
    explicit LazySequence(Generator<T> gen, bool infinite = true)
            : generator(gen), isInfinite(infinite) {}

    //бесконечная с генератором и начальными элементами
    LazySequence(Generator<T> gen, T* seedItems, int seedCount,
                 bool infinite = true)
            : generator(gen), isInfinite(infinite) {
        for (int i = 0; i < seedCount; i++) {
            cache.Append(seedItems[i]);
        }
    }

    T Get(int index) {
        if (index < 0) {
            throw std::out_of_range("index cannot be negative");
        }
        materializeUpTo(index);
        return cache.Get(index);
    }

    T GetFirst() {
        return Get(0);
    }

    int GetMaterializedCount() const {
        return cache.GetLength();
    }

    bool IsInfinite() const {
        return isInfinite;
    }

    LazySequence<T>* GetSubsequence(int startIndex, int endIndex) {
        if (startIndex < 0 || startIndex > endIndex) {
            throw std::out_of_range("invalid indices");
        }
        materializeUpTo(endIndex);
        LazySequence<T>* result = new LazySequence<T>();
        for (int i = startIndex; i <= endIndex; i++) {
            result->cache.Append(cache.Get(i));
        }
        return result;
    }

    LazySequence<T>* Where(std::function<bool(T)> predicate, int count) {
        LazySequence<T>* result = new LazySequence<T>();
        for (int i = 0; i < count; i++) {
            T item = Get(i);
            if (predicate(item)) {
                result->cache.Append(item);
            }
        }
        return result;
    }

    T Reduce(std::function<T(T, T)> func, T initial, int count) {//сворачивает список в одно значение по правилу например сумма 5 сисео первых

        T result = initial;
        for (int i = 0; i < count; i++) {
            result = func(result, Get(i));
        }
        return result;
    }
};

#endif