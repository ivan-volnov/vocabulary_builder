#ifndef APP_HPP
#define APP_HPP

#include <st/tiled_ncurses.hpp>
#include <unordered_set>


class CardModel;

namespace ColorScheme {

constexpr auto Window = "window";
constexpr auto Error = "error";
constexpr auto Gray = "gray";
constexpr auto Blue = "blue";

} // namespace ColorScheme

class MainWindow : public st::CursesWindow
{
public:
    MainWindow(
        std::shared_ptr<st::Screen> screen,
        std::weak_ptr<st::ProgressBar> progressbar_ptr, std::shared_ptr<CardModel> model_,
        size_t current_card_idx);

public:
    void paint() const override;
    uint8_t process_key(char32_t ch, bool is_symbol) override;

    void save_state();

private:
    void print(const std::string &str) const;
    void current_card_idx_changed(size_t prev_card_idx);

private:
    std::weak_ptr<st::Screen> screen_ptr;
    std::weak_ptr<st::ProgressBar> progressbar_ptr;
    std::shared_ptr<CardModel> model;
    std::string txt;
    size_t current_card_idx;
    std::unordered_set<std::string> skipped_list;
};

class Footer : public st::CursesWindow
{
public:
    Footer();

    void paint() const override;
};

#endif // APP_HPP
