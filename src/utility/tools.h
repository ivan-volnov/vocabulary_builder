#ifndef TOOLS_H
#define TOOLS_H

#include <sstream>


namespace tools {

std::string weekday_to_string(uint32_t day);
void clone_file(const std::string &src, const std::string &dst);
bool am_I_being_debugged();


template <typename T>
void string_replace(std::basic_string<T> &str, const std::basic_string<T> &src, const std::basic_string<T> &dst)
{
    size_t pos = 0;
    while ((pos = str.find(src, pos)) != std::string::npos) {
        str.replace(pos, src.size(), dst);
        pos += dst.size();
    }
}

template <typename T, typename Iterator, typename D>
std::enable_if_t<std::is_convertible_v<T, std::basic_string<typename T::value_type>>, T>
join(Iterator begin, Iterator end, const D &delimiter)
{
    std::basic_ostringstream<typename T::value_type> ss;
    for (auto it = begin; it != end; ++it) {
        if (it != begin) {
            ss << delimiter;
        }
        ss << *it;
    }
    return ss.str();
}

template <typename T, typename Container, typename D>
T join(const Container &container, const D &delimiter)
{
    return join<T>(container.begin(), container.end(), delimiter);
}


} // namespace tools


#endif // TOOLS_H
