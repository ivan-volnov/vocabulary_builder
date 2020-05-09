#ifndef APP_H
#define APP_H

#include "utility/ncursespp.h"

class CardModel;



enum ColorScheme : uint16_t
{
    ColorWindow,
    ColorError,
    ColorGray,
    ColorTest,
    ColorTest2,
    ColorTest3,
};



class AppScreen : public Screen
{
public:
    AppScreen();
};



class MainWindow : public CursesWindow
{
public:
    MainWindow();

public:
    void paint() const override;
    bool process_symbol(char32_t ch) const override;

private:
    std::shared_ptr<CardModel> model;
};



class Footer : public CursesWindow
{
public:
    Footer();

public:
    void paint() const override;
};

#endif // APP_H
