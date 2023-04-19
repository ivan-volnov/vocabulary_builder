#include "tools.hpp"
#include <st/string_functions.hpp>

std::string tools::weekday_to_string(uint32_t day)
{
    switch (day) {
    case 0:
        return "Sunday";
    case 1:
        return "Monday";
    case 2:
        return "Tuesday";
    case 3:
        return "Wednesday";
    case 4:
        return "Thursday";
    case 5:
        return "Friday";
    case 6:
        return "Saturday";
    default:
        throw std::runtime_error("Wrong weekday value");
    }
}

std::string tools::clear_string(const std::string &string)
{
    auto s = string;
    st::strip_html_tags(s);
    st::replace_all(s, ",", ", ");
    st::replace_all(s, "!", "! ");
    st::replace_all(s, " )", ")");
    st::replace_all(s, "( ", "(");
    st::replace_all(s, " ,", ",");
    st::replace_recursive(s, "  ", " ");
    st::trim(s);
    return s;
}

std::string tools::clear_string(const std::string &string, bool &changed)
{
    auto str = clear_string(string);
    if (str != string) {
        changed = true;
    }
    return str;
}
