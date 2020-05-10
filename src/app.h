#ifndef APP_H
#define APP_H

#include "utility/tiled_ncurses.h"
#include <vector>


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
    MainWindow();

public:
    void paint() const override;
    uint8_t process_key(char32_t ch, bool is_symbol) override;

private:
    std::shared_ptr<CardModel> model;
    std::vector<std::string> list;
    size_t current_item = 0;
};



class Footer : public CursesWindow
{
public:
    Footer();

    uint8_t process_key(char32_t ch, bool is_symbol) override;

public:
    void paint() const override;
};

#endif // APP_H
