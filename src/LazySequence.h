#ifndef LAZY_SEQUENCE_H
#define LAZY_SEQUENCE_H

#include "Sequence.h"
#include "MutableArraySequence.h"
#include "Generator.h"
#include "Cardinal.h"
#include <stdexcept>
#include <functional>

//ленивая последовательность вычисляет элементы по требованию
//наследуется от Sequence чтобы быть взаимозаменяемой с другими последовательностями
//операции Map Where Concat не материализуют данные возвращают новую LazySequence с генератором
template <class T>
class LazySequence : public Sequence<T> {
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

protected:
    //instance возвращает this тк LazySequence мьютабл работает на месте
    Sequence<T>* instance() override {
        return this;
    }

    Sequence<T>* appendImpl(const T& elem) override {
        if (isInfinite) {
            throw std::logic_error("cannot append to infinite sequence");
        }
        cache.Append(elem);
        return this;
    }

    Sequence<T>* prependImpl(const T& elem) override {
        if (isInfinite) {
            throw std::logic_error("cannot prepend to infinite sequence");
        }
        cache.Prepend(elem);
        return this;
    }

    Sequence<T>* insertAtImpl(const T& elem, int index) override {
        if (isInfinite) {
            throw std::logic_error("cannot insert into infinite sequence");
        }
        cache.InsertAt(elem, index);
        return this;
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

    //реализация Sequence<T>
    T GetFirst() const override {
        return Get(0);
    }

    T GetLast() const override {
        if (isInfinite) {
            throw std::logic_error("cannot get last element of infinite sequence");
        }
        if (cache.GetLength() == 0) {
            throw std::out_of_range("sequence is empty");
        }
        return cache.Get(cache.GetLength() - 1);
    }

    T Get(int index) const override {
        if (index < 0) {
            throw std::out_of_range("index cannot be negative");
        }
        //логически объект не меняется( кэш это деталь реализации)
        const_cast<LazySequence<T>*>(this)->materializeUpTo(index);
        return cache.Get(index);
    }

    //GetLength для совместимости с Sequence для бесконечной бросает исключение
    //для получения кардинал длины используемс GetCardinalLength()
    int GetLength() const override {
        if (isInfinite) {
            throw std::logic_error("length of infinite sequence is undefined — use GetCardinalLength()");
        }
        return cache.GetLength();
    }

    //Кардинал-версия длины умеет возвращать бесконечность
    Cardinal GetCardinalLength() const {
        if (isInfinite) {
            return Cardinal::Infinity();
        }
        return Cardinal(cache.GetLength());
    }

    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override {
        if (startIndex < 0 || startIndex > endIndex) {
            throw std::out_of_range("invalid indices");
        }
        const_cast<LazySequence<T>*>(this)->materializeUpTo(endIndex);
        LazySequence<T>* result = new LazySequence<T>();
        for (int i = startIndex; i <= endIndex; i++) {
            result->cache.Append(cache.Get(i));
        }
        return result;
    }

    //новая лэзи секанс с генератором который знает про обе
    //ничего не материализует пока не запросят конкретный элемент
    Sequence<T>* Concat(const Sequence<T>* other) const override {
        LazySequence<T>* self = const_cast<LazySequence<T>*>(this);
        const Sequence<T>* otherSeq = other;
        bool selfInfinite = isInfinite;

        //определяем бесконечность результата
        bool otherInfinite = false;
        const LazySequence<T>* otherLazy = dynamic_cast<const LazySequence<T>*>(other);
        if (otherLazy) {
            otherInfinite = otherLazy->IsInfinite();
        }

        //генератор типа i-й элемент результата из первой или второй последовательности
        auto indexFn = [self, otherSeq, selfInfinite](int i) -> T {
            if (selfInfinite) {
                //первая бесконечная а вторая недостижима
                return self->Get(i);
            }
            int selfSize = self->GetLength();
            if (i < selfSize) {
                return self->Get(i);
            }
            return otherSeq->Get(i - selfSize);
        };

        bool resultInfinite = selfInfinite || otherInfinite;
        return new LazySequence<T>(
                Generator<T>::FromIndexFunction(indexFn),
                resultInfinite
        );
    }

    //тоже новая LazySequence применяет func к каждому элементу без материализации
    Sequence<T>* Map(T (*func)(T)) const override {
        LazySequence<T>* self = const_cast<LazySequence<T>*>(this);
        auto indexFn = [self, func](int i) -> T {
            return func(self->Get(i));
        };
        return new LazySequence<T>(
                Generator<T>::FromIndexFunction(indexFn),
                isInfinite
        );
    }

    //аналогичноо новая LazySequence генератор ищет следующий подходящий элемент
    //каждый Get(i) ищет iй элемент удовлетворяющий предикату в исходной последовательности
    Sequence<T>* Where(bool (*predicate)(T)) const override {
        LazySequence<T>* self = const_cast<LazySequence<T>*>(this);
        //правило длина resultCache равно сколько подходящих уже нашли
        //ищем следующий подходящий перебирая исходную с нуля
        auto rule = [self, predicate](const MutableArraySequence<T>& resultCache) -> T {
            int need = resultCache.GetLength();
            int found = 0;
            int sourceIdx = 0;
            while (true) {
                T item = self->Get(sourceIdx);
                if (predicate(item)) {
                    if (found == need) { return item; }
                    found++;
                }
                sourceIdx++;
            }
        };

        return new LazySequence<T>(Generator<T>(rule), isInfinite);
    }

    //только для конечных
    T Reduce(T (*func)(T, T), T initial) const override {
        if (isInfinite) {
            throw std::logic_error("cannot reduce infinite sequence — use Reduce(func, initial, count)");
        }
        int len = cache.GetLength();
        T result = initial;
        for (int i = 0; i < len; i++) {
            result = func(result, cache.Get(i));
        }
        return result;
    }

    //с лимиторм для бесконечных и конечных
    T Reduce(T (*func)(T, T), T initial, int count) const {
        T result = initial;
        for (int i = 0; i < count; i++) {
            result = func(result, Get(i));
        }
        return result;
    }


    //для совместимости со старым кодом
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

    int GetMaterializedCount() const {
        return cache.GetLength();
    }

    bool IsInfinite() const {
        return isInfinite;
    }
};

#endif