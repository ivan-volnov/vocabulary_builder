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
    Card(std::string &&front, string_set &&levels, string_set &&pos) :
        front(std::move(front)), levels(std::move(levels)), pos(std::move(pos))
    {

    }

    template<typename T>
    void set_levels(T &&value)
    {
        levels = std::forward<T>(value);
    }

    string_set get_levels();

private:
    std::string front;
    std::string back;
    string_set levels;
    string_set pos;
};


#endif // CARD_H
