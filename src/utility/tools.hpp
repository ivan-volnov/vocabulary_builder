#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>

namespace tools {


std::string weekday_to_string(uint32_t day);

std::string clear_string(const std::string &string);
std::string clear_string(const std::string &string, bool &changed);

template<template<class...> class Container = std::vector, class T>
auto split(const std::basic_string<T> &str, const T *delimiter)
{
    Container<std::basic_string<T>> container;
    const auto len = std::char_traits<T>::length(delimiter);
    typename std::basic_string<T>::size_type start_pos = 0, pos;
    do {
        pos = str.find(delimiter, start_pos);
        if (pos != std::basic_string<T>::npos) {
            pos -= start_pos;
        }
        container.insert(container.end(), str.substr(start_pos, pos));
        start_pos += pos + len;
    } while (pos != std::basic_string<T>::npos);
    return container;
}


} // namespace tools

#endif // TOOLS_HPP
