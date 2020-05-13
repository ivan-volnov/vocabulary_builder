/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT

Copyright (c) 2020 Ivan Volnov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef STRING_ESSENTIALS_H
#define STRING_ESSENTIALS_H

#include <sstream>


namespace string_essentials {

namespace utf8 {

class decoder
{
public:
    bool decode_symbol(uint8_t ch);
    bool skip_symbol(uint8_t ch);

    char32_t symbol() const;

    template<typename Iterator>
    bool iterate(Iterator &it, Iterator end)
    {
        while (it != end) {
            if (decode_symbol(*it++)) {
                return true;
            }
        }
        return false;
    }

    template<typename Iterator>
    bool anvance(Iterator &it, Iterator end, size_t n = 1)
    {
        if (!n) {
            return true;
        }
        while (it != end) {
            if (skip_symbol(*it++) && !--n) {
                return true;
            }
        }
        return false;
    }

private:
    uint32_t state = 0;
    char32_t codepoint;
};


void encode_symbol(char32_t codepoint, std::string &str);

std::u32string decode(std::string::const_iterator begin, std::string::const_iterator end);
std::u32string decode(const std::string &str);

std::string encode(std::u32string::const_iterator begin, std::u32string::const_iterator end);
std::string encode(const std::u32string &str);
std::string encode(char32_t codepoint);

size_t strlen(const std::string &str);
size_t strlen(const char *str);
size_t strlen(char32_t codepoint);

char32_t at(size_t idx, const std::string &str);
char32_t at(size_t idx, const char *str);

void resize(std::string &str, size_t n, char ch);

template<typename Iterator>
Iterator next(Iterator it, Iterator end, size_t n = 1)
{
    decoder().anvance(it, end, n);
    return it;
}

template<typename Iterator>
size_t strlen(Iterator begin, Iterator end)
{
    size_t len = 0;
    decoder decoder;
    while (decoder.anvance(begin, end)) { ++len; }
    return len;
}


} // namespace utf8


template <typename T, typename Iterator>
std::basic_string<T> join(Iterator begin, Iterator end, const T *delimiter)
{
    std::basic_ostringstream<T> stream;
    for (auto it = begin; it != end; ++it) {
        if (it != begin) {
            stream << delimiter;
        }
        stream << *it;
    }
    return stream.str();
}

template <typename T, typename Container>
std::basic_string<T> join(const Container &container, const T *delimiter)
{
    return join(container.begin(), container.end(), delimiter);
}

template <template<typename...> class Container = std::vector, typename T>
auto split(const std::basic_string<T> &str, const T *delimiter)
{
    Container<std::basic_string<T>> container;
    const auto len = std::char_traits<T>::length(delimiter);
    typename std::basic_string<T>::size_type start_pos = 0, pos;
    do {
        pos = str.find(delimiter, start_pos);
        container.insert(container.end(), str.substr(start_pos, pos));
        start_pos = pos + len;
    } while (pos != std::basic_string<T>::npos);
    return container;
}

template <typename T>
void replace(std::basic_string<T> &str, const T *src, const T *dst)
{
    const auto src_len = std::char_traits<T>::length(src);
    const auto dst_len = std::char_traits<T>::length(dst);
    typename std::basic_string<T>::size_type pos = 0;
    while ((pos = str.find(src, pos)) != std::basic_string<T>::npos) {
        str.replace(pos, src_len, dst);
        pos += dst_len;
    }
}

std::string url_encode(const std::string &str);

} // namespace string_essentials

#endif // STRING_ESSENTIALS_H
