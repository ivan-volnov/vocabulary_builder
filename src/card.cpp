#include "card.h"
#include <string_essentials/string_essentials.hpp>


Card::Card(std::string &&front, string_set &&levels, string_set &&pos) :
    front(std::move(front)), levels(std::move(levels)), pos(std::move(pos))
{

}

std::string Card::get_front() const
{
    return front;
}

std::string Card::get_back() const
{
    return back;
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

uint64_t Card::get_note_id() const
{
    return note_id;
}

std::string Card::get_level_string() const
{
    return string_essentials::join(levels, ", ");
}

std::string Card::get_pos_string() const
{
    return string_essentials::join(pos, ", ");
}

void Card::set_note_id(uint64_t id)
{
    note_id = id;
}
