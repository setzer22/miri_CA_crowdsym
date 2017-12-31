#pragma once

#include <iostream>

template <typename T> void print(T str) { std::cout << str << std::endl; }
template <typename T> void error(T str) { std::cerr << str << std::endl; }

