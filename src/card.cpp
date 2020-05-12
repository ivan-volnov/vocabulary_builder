#include "card.h"
#include "utility/tools.h"


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

string_set Card::get_levels() const
{
    return levels;
}

string_set Card::get_pos() const
{
    return levels;
}

std::string Card::get_level_string() const
{
    return tools::join<std::string>(levels, ", ");
}

std::string Card::get_pos_string() const
{
    return tools::join<std::string>(pos, ", ");
}

void Card::set_anki_note_id(uint64_t id)
{
    anki_note_id = id;
}
