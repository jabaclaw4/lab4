#ifndef GENERATOR_H
#define GENERATOR_H

#include "MutableArraySequence.h"
#include <functional>
#include <stdexcept>

//генератор элементов для LazySequence хранит одно состояние тк если бы генераторов было несколько они бы хранили разные окна и запутались кто где находится
//инкапсулирует правило появления нового элемента и существует в единственном экземпляре для каждого LazySequence
template <class T>
class Generator {
public:
    // режим работы генератора — определяет какое правило используется
    enum class Mode { FromCache, FromIndex, FromVoid };

private:
    std::function<T(const MutableArraySequence<T>&)> rule; //функция которая смотрит на предыдущие элементы и возвращает следующий ну типа само правило
    MutableArraySequence<T> window; //не храним всю историю только сколько нужно для следующего шага
    int windowSize; //размер окна
    bool hasRule; //мб нет правила
    Mode mode;

public:
    //генератор без правила для конечных последовательностей
    Generator() : windowSize(0), hasRule(false), mode(Mode::FromCache) {}

    //генератор с правилом ( весь кэш)
    explicit Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule) //запрещает неявное преобразование при создании объекта:
            : rule(generatorRule), windowSize(0), hasRule(true), mode(Mode::FromCache) {}

    //генератор с правилом и фиксированным окном k элементов  эффективнее по памяти например фибоначи не нужно хранить весь кэш
    Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule, int k)
            : rule(generatorRule), windowSize(k), hasRule(true), mode(Mode::FromCache) {}

    //генератор из индексной функции Get(i) вычисляется независимо от истории
    //используется в Map Concat где i-й элемент не зависит от предыдущих
    //длина кэша в момент вызова = текущий запрашиваемый индекс
    static Generator<T> FromIndexFunction(std::function<T(int)> indexFn) {
        auto wrappedRule = [indexFn](const MutableArraySequence<T>& cache) -> T {
            return indexFn(cache.GetLength());
        };
        Generator<T> g(wrappedRule);
        g.mode = Mode::FromIndex;
        return g;
    }

    //генератор из функции без аргументов правило само знает что вернуть позволяет сделать файловый генератор снаружи одной функцией:
    //функция бросает исключение когда источник исчерпан
    static Generator<T> FromVoidFunction(std::function<T()> voidFn) {
        auto wrappedRule = [voidFn](const MutableArraySequence<T>&) -> T {
            return voidFn();
        };
        Generator<T> g(wrappedRule);
        g.mode = Mode::FromVoid;
        return g;
    }

    //породить следующий элемент
    //cache все что уже посчиталась в лэзи
    T GetNext(const MutableArraySequence<T>& cache) {
        if (!hasRule) {
            throw std::logic_error("generator has no rule");
        }

        if (windowSize > 0) {
            //обновляем скользящее окно из последних windowSize элементов
            int n = cache.GetLength();
            window = MutableArraySequence<T>();
            int from = (n - windowSize > 0) ? n - windowSize : 0;
            for (int i = from; i < n; i++) {
                window.Append(cache.Get(i));
            }
            return rule(window);
        }
        //передаём весь кэш
        return rule(cache);
    }

    bool HasRule() const {
        return hasRule;
    }

    Mode GetMode() const {
        return mode;
    }
};

#endif