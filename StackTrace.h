#pragma once

//TODO: Linux-only
#include <execinfo.h>

void print_stack_trace() {
    const int buffer_size = 256;
    void* addresses[buffer_size];

    int stack_size = backtrace(addresses, buffer_size);
    char** stack_strings = backtrace_symbols(addresses, stack_size);

    print("-----Call stack-----");
    for(int i = 0; i < stack_size; ++i) {
        print(stack_strings[i]);
    }
    print("--------------------");
}

