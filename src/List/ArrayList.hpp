#ifndef __ARRAY_LIST__HPP
#define __ARRAY_LIST__HPP

template<typename T>
class ArrayList {

public:
    /**
     * Creates a new list with default size and step of increment/decrement.
    */
    ArrayList(void) : ArrayList(6UL) { }
    /**
     * Creates a new list with the specified initial size and default step of increment/decrement
     * that is minimum between half of the size decided and 1.
    */
    ArrayList(const unsigned long size);
    /**
     * Creates a new list with the specified initial size and step of increment/decrement.
    */
    ArrayList(const unsigned long size, const unsigned long step);
    /**
     * Creates a new list with the specified initial size, step of increment/decrement and default value.
    */
    ArrayList(const unsigned long size, const unsigned long step, const T value);
    /**
     * Creates a new list with the specified initial size, minimum size, step of increment and step of decrement.
    */
    ArrayList(const unsigned long startSize, const unsigned long minSize,
              const unsigned long incrStep, const unsigned long decrStep);
    /**
     * Creates a new list with the specified initial size, minimum size, step of increment, step of decrement and default value.
    */
    ArrayList(const unsigned long startSize, const unsigned long minSize,
              const unsigned long incrStep, const unsigned long decrStep,
              const T value);
    /**
     * Sets a new step of increment/decrement.
    */
    void setStep(const unsigned long step);
    /**
     * Retrieves the element at the specified position.
    */
    T& operator[](const unsigned long i);
    /**
     * Retrieves the element at the specified position.
    */
    T& get(const unsigned long i);
    /**
     * Retrieves the first element.
    */
    T& getFirst(void);
    /**
     * Retrieves the last element.
    */
    T& getLast(void);
    /**
     * Retrieves the position of the first element.
    */
    unsigned long getFirstIndex(void);
    /**
     * Retrieves the position of the last element.
    */
    unsigned long getLastIndex(void);
    /**
     * Adds item and returns inserted position.
    */
    virtual unsigned long add(T t);
    /**
     * Removes element at specified position.
    */
    void removeAt(const unsigned long i);
    /**
     * Removes first equal element in the list.
    */
    virtual bool removeFirst(T t);
    /**
     * Removes last equal element in the list.
    */
    virtual bool removeLast(T t);
    /**
     * Removes all equal elements in the list.
    */
    virtual bool removeAll(T t);
    /**
     * Removes first element in the list.
    */
    virtual void removeFirst(void);
    /**
     * Removes last element in the list.
    */
    virtual void removeLast(void);
    /**
     * Retrieves the number of elements of the list.
    */
    unsigned long getLength(void);
    /**
     * Tells if the list is not empty.
    */
    bool containsSomething(void);
    /**
     * Tells if the list is empty.
    */
    bool isEmpty(void);
    /**
     * Removes everything from the list;
    */
    void clear(void);
    virtual ~ArrayList(void);

private:
    unsigned long minSize = 0;
    unsigned long incrStep;
    unsigned long decrStep;
    unsigned long elements = 0;
    unsigned long length;
    T *array;

    void checkIfIncreaseDimension(void);
    void checkIfDecreaseDimension(void);
    void setAll(const T& value);

};

#include "ArrayList.cpp"

#endif // __ARRAY_LIST__HPP
