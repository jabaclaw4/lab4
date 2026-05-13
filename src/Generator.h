#ifndef GENERATOR_H
#define GENERATOR_H

#include "MutableArraySequence.h"
#include <functional>
#include <stdexcept>

//генератор элементов для LazySequence хранит одно состояние тк если бы генераторов было несколько они бы хранили разные окна и запутались кто где находится
//инкапсулирует правило появления нового элемента и существует в единственном экземпляре для каждого LazySequence
template <class T>
class Generator {
private:
    std::function<T(const MutableArraySequence<T>&)> rule;//функция которая смотрит на предыдущие элементы и возвращает следующий ну типа само правило
    MutableArraySequence<T> window;//не храним всю историю только сколько нужно для следующего шага
    int windowSize;//размер окна
    bool hasRule;//мб нет правила

public:
    //генератор без правила для конечных последовательностей
    Generator() : windowSize(0), hasRule(false) {}

    //генератор с правилом ( весь кэш)
    explicit Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule)//запрещает неявное преобразование при создании объекта:
            : rule(generatorRule), windowSize(0), hasRule(true) {}
    //генератор с правилом и фиксированным окном k элементов  эффективнее по памяти например фибоначи не нужно хранить весь кэш
    Generator(std::function<T(const MutableArraySequence<T>&)> generatorRule,
              int k)
            : rule(generatorRule), windowSize(k), hasRule(true) {}

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
};

#endif