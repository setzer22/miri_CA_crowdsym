#pragma once

#include "Utils.hpp"
#include "StackTrace.h"
#include <stdlib.h>
#include "Allocator.h"

inline bool in_bounds(int i, int size) {
    return i >= 0 && i < size;
}

template <typename T> struct Array {
    int size;
    T* data;

    T operator[](int i) const {
        if(!in_bounds(i, size)) {
            error("Attempted out of bounds access at index:");
            error(i);
            print_stack_trace();
            exit(0);
        }
        return data[i];
    }
    T& operator[] (int i) {
        if(!in_bounds(i, size)) {
            error("Attempted out of bounds access at index:");
            error(i);
            print_stack_trace();
            exit(0);
        }
        return data[i];
    }
};

template <typename T> Array<T> alloc_array(int size, Allocator* allocator) {
    Array<T> arr;
    arr.size = size;
    arr.data = alloc<T>(size*sizeof(T), allocator);
    return arr;
}

template <typename T> Array<T> alloc_array(int size) {
    Array<T> arr;
    arr.size = size;
    arr.data = alloc<T>(size*sizeof(T));
    return arr;
}
