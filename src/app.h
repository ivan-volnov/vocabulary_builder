#ifndef APP_H
#define APP_H

#include <tiled_ncurses/tiled_ncurses.hpp>


class CardModel;



namespace ColorScheme {

constexpr auto Window = "window";
constexpr auto Error  = "error";
constexpr auto Gray   = "gray";
constexpr auto Blue   = "blue";

}



class MainWindow : public CursesWindow
{
public:
    MainWindow(std::shared_ptr<Screen> screen, std::weak_ptr<ProgressBar> progressbar_ptr, std::shared_ptr<CardModel> model_, size_t current_card_idx);

public:
    void paint() const override;
    uint8_t process_key(char32_t ch, bool is_symbol) override;

private:
    void print(const std::string &str) const;
    void current_card_idx_changed(size_t prev_card_idx);

private:
    std::weak_ptr<Screen> screen_ptr;
    std::weak_ptr<ProgressBar> progressbar_ptr;
    std::shared_ptr<CardModel> model;
    std::string txt;
    size_t current_card_idx;
};



class Footer : public CursesWindow
{
public:
    Footer();

    void paint() const override;
};

#endif // APP_H
