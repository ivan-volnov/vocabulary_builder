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
    std::weak_ptr<Screen> screen_ptr;
    std::shared_ptr<CardModel> model;
    std::string txt;
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
