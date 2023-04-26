#ifndef CATCH_FORMATTERS_HPP
#define CATCH_FORMATTERS_HPP

#include <catch2/catch.hpp>
#include <st/money.hpp>

namespace Catch {


template<class T>
requires std::same_as<T, st::Money> || std::same_as<T, st::int128_t> ||
    std::same_as<T, st::uint128_t>
struct StringMaker<T>
{
    static std::string convert(const T &value)
    {
        return fmt::format("{}", value);
    }
};


} // namespace Catch

#endif // CATCH_FORMATTERS_HPP
