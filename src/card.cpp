#include "card.hpp"
#include <fmt/format.h>

std::string Card::get_front() const
{
    return front;
}

std::string Card::get_back() const
{
    return back;
}

std::string Card::get_forms() const
{
    return forms;
}

std::string Card::get_level() const
{
    return levels.empty() ? "D1" : *levels.begin();
}

string_set Card::get_levels() const
{
    return levels;
}

string_set Card::get_pos() const
{
    return levels;
}

string_set Card::get_tags() const
{
    auto result = tags;
    result.insert(levels.begin(), levels.end());
    return result;
}

uint64_t Card::get_note_id() const
{
    return note_id;
}

std::string Card::get_level_string() const
{
    return fmt::format("{}", fmt::join(levels, ", "));
}

std::string Card::get_pos_string() const
{
    return fmt::format("{}", fmt::join(pos, ", "));
}

void Card::add_tag(const std::string &tag)
{
    tags.insert(tag);
}

void Card::set_note_id(uint64_t id)
{
    note_id = id;
}
