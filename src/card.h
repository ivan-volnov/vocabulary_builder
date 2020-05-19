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
    Card() = default;
    ~Card() = default;
    Card(const Card &) = delete;
    Card &operator=(const Card &) = delete;
    Card(Card &&) = delete;
    Card &operator=(Card &&) = delete;

    std::string get_front() const;
    std::string get_back() const;
    std::string get_forms() const;
    std::string get_level() const;
    string_set get_levels() const;
    string_set get_pos() const;
    string_set get_tags() const;
    uint64_t get_note_id() const;
    std::string get_level_string() const;
    std::string get_pos_string() const;

    template<typename T>
    void set_front(T &&value)
    {
        front = std::forward<T>(value);
    }

    template<typename T>
    void set_back(T &&value)
    {
        back = std::forward<T>(value);
    }

    template<typename T>
    void set_forms(T &&value)
    {
        forms = std::forward<T>(value);
    }

    template<typename T>
    void set_levels(T &&value)
    {
        levels = std::forward<T>(value);
    }

    template<typename T>
    void set_pos(T &&value)
    {
        pos = std::forward<T>(value);
    }

    void add_tag(const std::string &tag);

    void set_note_id(uint64_t id);

private:
    std::string front;
    std::string back;
    std::string forms;
    string_set levels;
    string_set pos;
    string_set tags;
    uint64_t note_id = 0;
};


#endif // CARD_H
