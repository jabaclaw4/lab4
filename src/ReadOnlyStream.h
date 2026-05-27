#ifndef READ_ONLY_STREAM_H
#define READ_ONLY_STREAM_H

#include "LazySequence.h"
#include "Sequence.h"
#include <functional>
#include <stdexcept>
#include <fstream>

//поток только для чтения ,чтение = продвижение вперёд прочитанный элемент уходит какбы
template <class T>
class ReadOnlyStream {
public:
    //тип источника данных
    enum class SourceType {
        FromLazySequence,
        FromSequence,
        FromFile
    };

private:
    SourceType sourceType;
    LazySequence<T>* lazySource;
    const Sequence<T>* seqSource;
    std::ifstream fileSource;
    //функция десериализации: строка -> T
    std::function<T(const std::string&)> deserializer;

    //текущая позиция сколько элементов прочитано
    int position;
    //для конечных источников — сколько всего элементов
    //-1 означает бесконечный источник (LazySequence infinite)
    int totalCount;

    bool isOpen;

public:
    //создать из LazySequence
    ReadOnlyStream(LazySequence<T>* lazy, int count = -1)
            : sourceType(SourceType::FromLazySequence),
              lazySource(lazy),
              seqSource(nullptr),
              position(0),
              totalCount(count),
              isOpen(false) {}

    //создать из обычной Sequence
    ReadOnlyStream(const Sequence<T>* seq)
            : sourceType(SourceType::FromSequence),
              lazySource(nullptr),
              seqSource(seq),
              position(0),
              totalCount(seq->GetLength()),
              isOpen(false) {}

    //deserializer как превратить строку в T
    ReadOnlyStream(const std::string& filename,
                   std::function<T(const std::string&)> deserializer)
            : sourceType(SourceType::FromFile),
              lazySource(nullptr),
              seqSource(nullptr),
              deserializer(deserializer),
              position(0),
              totalCount(-1),
              isOpen(false) {
        fileSource.open(filename);
        if (!fileSource.is_open()) {
            throw std::runtime_error("cannot open file: " + filename);
        }
    }

    ~ReadOnlyStream() {
        if (fileSource.is_open()) {
            fileSource.close();
        }
    }

    //открыть поток перед чтением
    void Open() {
        if (isOpen) {
            throw std::runtime_error("stream is already open");
        }
        isOpen = true;
        position = 0;
    }

    //закрыть поток после чтения
    void Close() {
        if (!isOpen) {
            throw std::runtime_error("stream is not open");
        }
        isOpen = false;
        if (fileSource.is_open()) {
            fileSource.close();
        }
    }

    //достигнут ли конец потока
    bool IsEndOfStream() const {
        if (!isOpen) {
            throw std::runtime_error("stream is not open");
        }
        //бесконечный поток — никогда не кончается
        if (totalCount == -1) {
            return false;
        }
        return position >= totalCount;
    }

    //прочитать следующий элемент и сдвинуться вперёд
    T Read() {
        if (!isOpen) {
            throw std::runtime_error("stream is not open");
        }
        if (IsEndOfStream()) {
            throw std::out_of_range("end of stream reached");
        }

        T result;

        if (sourceType == SourceType::FromLazySequence) {
            result = lazySource->Get(position);
        }
        else if (sourceType == SourceType::FromSequence) {
            result = seqSource->Get(position);
        }
        else {
            //читаем строку из файла и десериализуем
            std::string line;
            if (!std::getline(fileSource, line)) {
                throw std::out_of_range("end of file reached");
            }
            result = deserializer(line);
        }

        position++;
        return result;
    }

    int GetPosition() const {
        return position;
    }

    //можно ли перемотать (только не для файла)
    bool IsCanSeek() const {
        return sourceType != SourceType::FromFile;
    }

    //перейти на позицию index только если IsCanSeek()
    void Seek(int index) {
        if (!IsCanSeek()) {
            throw std::runtime_error("seek is not supported for file streams");
        }
        if (index < 0) {
            throw std::out_of_range("index cannot be negative");
        }
        if (totalCount != -1 && index > totalCount) {
            throw std::out_of_range("index out of stream bounds");
        }
        position = index;
    }
};

#endif