#ifndef CARDINAL_H
#define CARDINAL_H

#include <stdexcept>
#include <string>

//тип для представления размера последовательности
//может быть либо обычным натуральным числом либо бесконечностью !!используется в GetLength() для LazySequence
class Cardinal {
private:
    bool infinite;
    int value;

public:
    //создать конечное значение
    Cardinal(int val) : infinite(false), value(val) {
        if (val < 0) {
            throw std::invalid_argument("cardinal value cannot be negative");
        }
    }

    //создать бесконечное значение
    static Cardinal Infinity() {
        Cardinal c(0);
        c.infinite = true;
        return c;
    }

    //бесконечное ли
    bool IsInfinite() const {
        return infinite;
    }

    //получить значение (только если конечное)
    int GetValue() const {
        if (infinite) {
            throw std::logic_error("cannot get value of infinite cardinal");
        }
        return value;
    }

    //операторы сравнения
    bool operator==(const Cardinal& other) const {
        if (infinite && other.infinite) return true;
        if (infinite || other.infinite) return false;
        return value == other.value;
    }

    bool operator!=(const Cardinal& other) const {
        return !(*this == other);
    }

    bool operator<(const Cardinal& other) const {
        if (infinite) return false;//бесконечность не меньше ничего
        if (other.infinite) return true;//любое число меньше бесконечности
        return value < other.value;
    }

    bool operator<=(const Cardinal& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>(const Cardinal& other) const {
        return !(*this <= other);
    }

    bool operator>=(const Cardinal& other) const {
        return !(*this < other);
    }

    //для вывода
    std::string ToString() const {
        if (infinite) return "∞";
        return std::to_string(value);
    }
};

#endif