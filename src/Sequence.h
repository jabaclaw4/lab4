#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "ResultInfo.h"

//напоминалки
//базовый абстрактный класс для всех последовательностей (полиморфизм работаем с seq не зная что внутри)
//virtual = функция может работать по-разному в зависимости от реального типа объекта
//= 0 означает "чисто виртуальная" - должна быть реализована в наследниках
//const после функции = функция не изменяет объект

//правильная архитектура чтобы был полиморфизм (спокойно добавить новый тд) распределяем обязанности:
//DynamicArray and LinkedList для работы с данными (чисто хранение)
//ListSeqBase and ArraySeqBase для методов чтения (чтобы не писать одни и те же методы для мьютабл и иммьютабл)
//сам абстрактный класс Sequence высокий уровень организует логику через нижний уровень (выше озвученные классы)
template <class T> //щаблон = один код для всех типов
class Sequence {
protected:
    //чисто виртуальный - возвращает this или копию
    virtual Sequence<T>* instance() = 0;

    //чисто виртуальные - реализация добавления
    virtual Sequence<T>* appendImpl(const T& elem) = 0;
    virtual Sequence<T>* prependImpl(const T& elem) = 0;
    virtual Sequence<T>* insertAtImpl(const T& elem, int index) = 0;

public:
    virtual ~Sequence() {}

//методы чтения
    //получить первый элемент (бросает исключение если пусто)
    virtual T GetFirst() const = 0;

    //получить последний элемент (бросает исключение если пусто)
    virtual T GetLast() const = 0;

    //получить элемент по индексу (бросает исключение если неверный индекс)
    virtual T Get(int index) const = 0;

    //получить длину последовательности
    virtual int GetLength() const = 0;

    //получить подпоследовательность от startIndex до endIndex (включительно)
    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) const = 0;

//методы изменения
    //добавить элемент в конец
    virtual Sequence<T>* Append(const T& elem) {
        return instance()->appendImpl(elem);
    }

    //добавить элемент в начало
    virtual Sequence<T>* Prepend(const T& elem) {
        return instance()->prependImpl(elem);
    }

    //вставить элемент на позицию index
    virtual Sequence<T>* InsertAt(const T& elem, int index) {
        return instance()->insertAtImpl(elem, index);
    }

    //объединить с другой последовательностью
    virtual Sequence<T>* Concat(const Sequence<T>* other) const = 0;

//методы для результ инфо (try)
    //попытка получить первый элемент (без исключения если пусто)
    virtual ResultInfo<T> TryGetFirst() const {
        if (this->GetLength() == 0) {
            return ResultInfo<T>::Failure("Sequence is empty");
        }
        return ResultInfo<T>::Success(this->GetFirst());
    }

    //попытка получить последний элемент (без исключения если пусто)
    virtual ResultInfo<T> TryGetLast() const {
        if (this->GetLength() == 0) {
            return ResultInfo<T>::Failure("Sequence is empty");
        }
        return ResultInfo<T>::Success(this->GetLast());
    }

    //попытка получить элемент по индексу (без исключения если неверный индекс)
    virtual ResultInfo<T> TryGet(int index) const {
        if (index < 0 || index >= this->GetLength()) {
            return ResultInfo<T>::Failure("Index out of range");
        }
        return ResultInfo<T>::Success(this->Get(index));
    }

    //найти первый элемент удовлетворяющий условию (без исключения если не найден)
    virtual ResultInfo<T> TryFind(bool (*predicate)(T)) const {
        for (int i = 0; i < this->GetLength(); i++) {
            T item = this->Get(i);
            if (predicate(item)) {
                return ResultInfo<T>::Success(item);
            }
        }
        return ResultInfo<T>::Failure("Element not found");
    }

    //найти индекс первого элемента, удовлетворяющего условию (без исключения если не найден)
    virtual ResultInfo<int> TryFindIndex(bool (*predicate)(T)) const {
        for (int i = 0; i < this->GetLength(); i++) {
            if (predicate(this->Get(i))) {
                return ResultInfo<int>::Success(i);
            }
        }
        return ResultInfo<int>::Failure("Element not found");
    }

    //seq[i]
    //позволяет писать seq[5] вместо seq->Get(5)
    T operator[](int index) const {
        return this->Get(index);
    }

    //seq1 == seq2
    bool operator==(const Sequence<T>& other) const {
        //разная длина -> не равны
        if (this->GetLength() != other.GetLength()) {
            return false;
        }
        //сравниваем каждый элемент
        for (int i = 0; i < this->GetLength(); i++) {
            if (this->Get(i) != other.Get(i)) {
                return false;
            }
        }
        return true;
    }

    //seq1 != seq2
    bool operator!=(const Sequence<T>& other) const {
        return !(*this == other);
    }

    //seq1 + seq2
    //возвращает новую последовательность [элементы seq1, элементы seq2]
    Sequence<T>* operator+(const Sequence<T>& other) const {
        return this->Concat(&other);
    }

    virtual Sequence<T>* Where(bool (*predicate)(T)) const = 0;
    virtual Sequence<T>* Map(T (*func)(T)) const = 0;
    virtual T Reduce(T (*func)(T, T), T initial) const = 0;
};

#endif