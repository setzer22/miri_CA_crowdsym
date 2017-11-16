#pragma once 

#include <cassert>

struct Allocator {
    void* data;
    int pos;
    int size;
};
Allocator __default_memory_allocator;

Allocator mk_allocator(int bytes) {
    Allocator allocator;
    allocator.data = malloc(bytes);
    allocator.pos = 0;
    allocator.size = bytes;
    return allocator;
}

template <typename T> T* alloc(int size_bytes, Allocator* allocator) {
    if (allocator->pos + size_bytes > allocator->size) {
        error("Too much memory allocated");
        exit(0);
    }
    //void* start = allocator->data+ allocator->pos; <- generated warning
    void* start = static_cast<void*>((char*)(allocator->data)+ allocator->pos);
    allocator->pos += size_bytes;
    return static_cast<T*>(start);
}

template <typename T> T* alloc(int size) {
    assert(__default_memory_allocator.data != nullptr);
    return alloc<T>(size, &__default_memory_allocator);
}

template <typename T> T* alloc_one(Allocator* allocator) {
    assert(allocator->data != nullptr);
    return alloc<T>(1*sizeof(T), allocator);
}

template <typename T> T* alloc_one() {
    assert(__default_memory_allocator.data != nullptr);
    return alloc<T>(1*sizeof(T), &__default_memory_allocator);
}


/*Allocator copy_allocator(Allocator a) {
    Allocator b;
    b = a;
    b.data = malloc(a.size);
    memcpy(b.data, a.data, a.size);
}*/





