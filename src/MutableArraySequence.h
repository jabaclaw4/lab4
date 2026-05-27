#ifndef MUTABLE_ARRAY_SEQUENCE_H
#define MUTABLE_ARRAY_SEQUENCE_H

#include "ArraySequenceBase.h"
//изменяемая послед ть на базе массива
template <class T>
class MutableArraySequence : public ArraySequenceBase<T> {
protected:
    //изменяем себя
    Sequence<T>* instance() override {
        return this;
    }

    ArraySequenceBase<T>* CreateNew(int size) const override {
        return new MutableArraySequence<T>(size);
    }

public:
    MutableArraySequence() : ArraySequenceBase<T>() {}
    MutableArraySequence(T* items, int count) : ArraySequenceBase<T>(items, count) {}
    MutableArraySequence(int size) : ArraySequenceBase<T>(size) {}
    MutableArraySequence(const MutableArraySequence<T>& other) : ArraySequenceBase<T>(other) {}

    void Set(int index, const T& value) {
        this->items->Set(index, value);
    }
};

#endif