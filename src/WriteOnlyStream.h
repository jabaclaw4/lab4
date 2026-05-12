#ifndef WRITE_ONLY_STREAM_H
#define WRITE_ONLY_STREAM_H

#include "MutableArraySequence.h"
#include <functional>
#include <stdexcept>
#include <fstream>
#include <string>

//поток только для записи
//пишем элементы последовательно в конец
//назначением может быть: MutableArraySequence, файл
template <class T>
class WriteOnlyStream {
public:
    //тип назначения
    enum class TargetType {
        ToSequence,
        ToFile
    };

private:
    TargetType targetType;

    //назначение 1: последовательность в памяти
    MutableArraySequence<T>* seqTarget;

    //назначение 2: файл
    std::ofstream fileTarget;
    //функция сериализации: T -> строка
    std::function<std::string(const T&)> serializer;

    //сколько элементов записано
    int position;

    bool isOpen;

public:
    //создать поток в последовательность
    WriteOnlyStream(MutableArraySequence<T>* seq)
            : targetType(TargetType::ToSequence),
              seqTarget(seq),
              position(0),
              isOpen(false) {}

    //создать поток в файл
    //serializer: как превратить T в строку
    //пример для int: [](const int& x) { return std::to_string(x); }
    WriteOnlyStream(const std::string& filename,
                    std::function<std::string(const T&)> serializer)
            : targetType(TargetType::ToFile),
              seqTarget(nullptr),
              serializer(serializer),
              position(0),
              isOpen(false) {
        fileTarget.open(filename);
        if (!fileTarget.is_open()) {
            throw std::runtime_error("cannot open file: " + filename);
        }
    }

    ~WriteOnlyStream() {
        if (fileTarget.is_open()) {
            fileTarget.close();
        }
    }

    //открыть поток перед записью
    void Open() {
        if (isOpen) {
            throw std::runtime_error("stream is already open");
        }
        isOpen = true;
        position = 0;
    }

    //закрыть поток после записи
    void Close() {
        if (!isOpen) {
            throw std::runtime_error("stream is not open");
        }
        isOpen = false;
        if (fileTarget.is_open()) {
            fileTarget.close();
        }
    }

    //записать элемент в конец потока
    //возвращает позицию следующей записи
    int Write(const T& item) {
        if (!isOpen) {
            throw std::runtime_error("stream is not open");
        }

        if (targetType == TargetType::ToSequence) {
            seqTarget->Append(item);
        }
        else {
            //сериализуем и пишем строку в файл
            fileTarget << serializer(item) << "\n";
            if (!fileTarget.good()) {
                throw std::runtime_error("file write error");
            }
        }

        position++;
        return position;
    }

    //сколько элементов записано
    int GetPosition() const {
        return position;
    }
};

#endif