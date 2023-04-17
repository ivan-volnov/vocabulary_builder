#include "tools.hpp"
#include <cassert>
#include <filesystem>
#include <string_essentials/string_essentials.hpp>
#include <sys/sysctl.h>
#include <unistd.h>

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
    auto str = string;
    string_essentials::strip_html_tags(str);
    string_essentials::replace(str, ",", ", ");
    string_essentials::replace(str, "!", "! ");
    string_essentials::replace(str, " )", ")");
    string_essentials::replace(str, "( ", "(");
    string_essentials::replace(str, " ,", ",");
    string_essentials::replace_recursive(str, "  ", " ");
    string_essentials::trim(str);
    return str;
}

std::string tools::clear_string(const std::string &string, bool &changed)
{
    auto str = clear_string(string);
    if (str != string) {
        changed = true;
    }
    return str;
}
