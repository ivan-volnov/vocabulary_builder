#ifndef CARD_H
#define CARD_H

#include <string>
#include <set>

using string_set = std::set<std::string>;
using string_set_pair = std::pair<string_set, string_set>;
using string_set_tuple3 = std::tuple<string_set, string_set, string_set>;


class Card
{
public:
    Card(std::string &&front, string_set &&levels, string_set &&pos);

    std::string get_front() const;
    std::string get_back() const;
    string_set get_levels() const;
    string_set get_pos() const;
    std::string get_level_string() const;
    std::string get_pos_string() const;

    template<typename T>
    void set_back(T &&value)
    {
        back = std::forward<T>(value);
    }

private:
    std::string front;
    std::string back;
    string_set levels;
    string_set pos;
};


#endif // CARD_H
