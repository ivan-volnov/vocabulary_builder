#ifndef APP_H
#define APP_H

#include "utility/tiled_ncurses.h"
#include <vector>
#include <string>


class CardModel;


class App
{
public:
    App();
    void run();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

private:
    std::shared_ptr<Screen> screen;
};


class MainWindow : public CursesWindow
{
public:
    MainWindow(std::shared_ptr<Screen> screen);

public:
    void paint() const override;
    uint8_t process_key(char32_t ch, bool is_symbol) override;

private:
    void print(const std::string &str) const;
    void current_card_idx_changed();

private:
    std::weak_ptr<Screen> screen_ptr;
    std::unique_ptr<CardModel> model;
    std::string txt;
    size_t current_card_idx = 0;
};



class Footer : public CursesWindow
{
public:
    Footer();

    void paint() const override;
};

#endif // APP_H
