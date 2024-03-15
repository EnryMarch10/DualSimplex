#ifndef __ARRAY_LIST__CPP
#define __ARRAY_LIST__CPP

#include "ArrayList.hpp"
#include "utils.hpp"
#include <assert.h>
#include <cstring>

template<typename T>
ArrayList<T>::ArrayList(const unsigned long size)
{
    length = size;
    incrStep = decrStep = MAX(length / 2UL, 1UL);
    array = new T[length];
}

template<typename T>
ArrayList<T>::ArrayList(const unsigned long size, const unsigned long step)
{
    assert(step > 0 && step < size);
    length = size;
    incrStep = decrStep = step;
    array = new T[length];
}

template<typename T>
ArrayList<T>::ArrayList(const unsigned long size, const unsigned long step, const T value) : ArrayList(size, step)
{
    setAll(value);
}

template<typename T>
ArrayList<T>::ArrayList(const unsigned long startSize, const unsigned long minSize,
                        const unsigned long incrStep, const unsigned long decrStep)
{
    assert(incrStep > 0 && decrStep > 0 && decrStep < minSize && minSize < startSize);
    this->minSize = minSize;
    length = startSize;
    this->incrStep = incrStep;
    this->decrStep = decrStep;
    array = new T[length];
}

template<typename T>
ArrayList<T>::ArrayList(const unsigned long startSize, const unsigned long minSize,
                        const unsigned long incrStep, const unsigned long decrStep,
                        const T value) : ArrayList(startSize, minSize, incrStep, decrStep)
{
    setAll(value);
}

template<typename T>
void ArrayList<T>::setAll(const T& value)
{
    for (unsigned long i = 0UL; i < length; i++) {
        array[i] = value;
    }
    elements = length;
}

template<typename T>
void ArrayList<T>::checkIfIncreaseDimension(void)
{
    if (elements == length) {
        length += incrStep;
        T *newArray = new T[length];
        memcpy(newArray, array, sizeof(T) * elements);
        delete [] array;
        array = newArray;
    }
    assert(elements < length);
}

template<typename T>
void ArrayList<T>::checkIfDecreaseDimension(void)
{
    if ((elements >= minSize) && (decrStep < length) && (elements <= length - decrStep)) {
        length = MAX(minSize, length - decrStep);
        T *newArray = new T[length];
        memcpy(newArray, array, sizeof(T) * elements);
        delete [] array;
        array = newArray;
    }
    assert(elements < length);
}

template<typename T>
void ArrayList<T>::setStep(const unsigned long step)
{
    assert(step > 0);
    this->step = step;
}

template<typename T>
T& ArrayList<T>::operator[](const unsigned long i)
{
    assert(i < elements);
    return array[i];
}

template<typename T>
T& ArrayList<T>::get(const unsigned long i)
{
    return (*this)[i];
}

template<typename T>
T& ArrayList<T>::getFirst(void)
{
    return (*this)[getFirstIndex()];
}

template<typename T>
T& ArrayList<T>::getLast(void)
{
    return (*this)[getLastIndex()];
}

template<typename T>
unsigned long ArrayList<T>::getFirstIndex(void)
{
    assert(containsSomething());
    return 0UL;
}

template<typename T>
unsigned long ArrayList<T>::getLastIndex(void)
{
    assert(containsSomething());
    return elements - 1UL;
}

template<typename T>
unsigned long ArrayList<T>::add(T t)
{
    checkIfIncreaseDimension();
    array[elements] = t;
    return elements++;
}

template<typename T>
void ArrayList<T>::removeAt(const unsigned long i)
{
    assert(i < elements);

    for (unsigned long j = i + 1UL; j < elements; j++) {
        array[j - 1UL] = array[j];
    }
    elements--;
}

template<typename T>
bool ArrayList<T>::removeFirst(T t)
{
    bool found = false;
    for (unsigned long i = 0UL; i < elements; i++) {
        if (found) {
            array[i - 1] = array[i];
        } else if (array[i] == t) {
            found = true;
            elements--;
        }
    }
    checkIfDecreaseDimension();

    return found;
}

template<typename T>
bool ArrayList<T>::removeLast(T t)
{
    if (isEmpty()) {
        return false;
    }

    bool found = false;
    unsigned long i = elements - 1UL;
    for (; i > 0UL; i--) {
        if (array[i] == t) {
            found = true;
            break;
        }
    }
    if (found) {
        for (; i < elements - 1UL; i++) {
            array[i] = array[i + 1UL];
        }
        elements--;
    }
    checkIfDecreaseDimension();

    return found;
}

template<typename T>
bool ArrayList<T>::removeAll(T t)
{
    bool found = false;
    unsigned long offset = 1UL;
    for (unsigned long i = 0UL; i < elements; i++) {
        if (found) {
            if (array[i] != t) {
                array[i - offset] = array[i];
            } else {
                offset++;
            }
        } else if (array[i] == t) {
            found = true;
        }
    }
    if (found) {
        elements -= offset;
    }
    checkIfDecreaseDimension();

    return found;
}

template<typename T>
void ArrayList<T>::removeFirst(void)
{
    removeAt(getFirstIndex());
}

template<typename T>
void ArrayList<T>::removeLast(void)
{
    assert(containsSomething());
    elements--;
}

template<typename T>
unsigned long ArrayList<T>::getLength(void)
{
    return elements;
}

template<typename T>
bool ArrayList<T>::isEmpty(void)
{
    return elements == 0UL;
}

template<typename T>
bool ArrayList<T>::containsSomething(void)
{
    return elements != 0UL;
}

template<typename T>
void ArrayList<T>::clear(void) {
    T *newArray = new T[0UL];
    delete [] array;
    array = newArray;
    minSize = 0UL;
}

template<typename T>
ArrayList<T>::~ArrayList(void)
{
    delete [] array;
}

#endif // __ARRAY_LIST__CPP
