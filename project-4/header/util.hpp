#ifndef UTIL_HPP
#define UTIL_HPP

#include "header.hpp"
#include <cstring>
#include "const.hpp"

inline void clearBuffer(void* data) {
    bzero(data, BUFFER_SIZE);
}

inline void dataToMessage(const char* source, std::string& destination, size_t size) {
    char* buffer = new char[size + 1];
    strncpy(buffer, source, size);
    buffer[size] = '\0';
    destination = std::string(buffer);
    delete[] buffer;
}

#endif
