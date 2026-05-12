#ifndef GENERATOR_H
#define GENERATOR_H

#include "MutableArraySequence.h"
#include <functional>
#include <stdexcept>

//генератор элементов для LazySequence
//инкапсулирует правило порождения очередного элемента существует в единственном экземпляре для каждого LazySequence
template <class T>
class Generator {
private:
    std::function<T(const MutableArraySequence<T>&)> rule;
    MutableArraySequence<T> window;//не храним всю историю только сколько нужно для следующего шага
    int windowSize;
    bool hasRule;

public:
    //генератор без правила (для конечных последовательностей)
    Generator() : windowSize(0), hasRule(false) {}

    //генератор с правилом ( весь кэш)
    explicit Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule)
            : rule(generatorRule), windowSize(0), hasRule(true) {}

    //генератор с правилом и фиксированным окном k элементов
    //например фибоначчи: windowSize=2, правило берёт последние 2
    //эффективнее по памяти — не нужно хранить весь кэш
    Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule,
              int k)
            : rule(generatorRule), windowSize(k), hasRule(true) {}

    //породить следующий элемент
    //cache — все уже вычисленные элементы LazySequence
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
};

#endif