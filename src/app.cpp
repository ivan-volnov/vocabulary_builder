#include "app.h"
#include <ncurses.h>
#include "card_model.h"

namespace ColorScheme {

constexpr auto Window = "window";
constexpr auto Error =  "error";
constexpr auto Gray =   "gray";
constexpr auto Test =   "test";
constexpr auto Test2 =  "test2";
constexpr auto Test3 =  "test3";

}


App::App()
{
    screen = std::make_shared<Screen>();
    screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
    screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);

    screen->init_color(ColorScheme::Test, COLOR_WHITE, COLOR_RED);
    screen->init_color(ColorScheme::Test2, COLOR_WHITE, COLOR_BLUE);
    screen->init_color(ColorScheme::Test3, COLOR_WHITE, COLOR_BLACK);

    screen->show_cursor(false);
}

void App::run()
{
    auto layout = screen->create<SimpleBorder>()->create<HorizontalLayout>(1);
    auto tst = layout->create<CursesWindow>();
    layout->create<CursesWindow>()->set_color(ColorScheme::Test2);
    layout->create<Footer>();

    tst->set_color(ColorScheme::Test);

    auto border = screen->create<SimpleBorder>(3, 4)->create<CursesBorder>();
    border->set_color(ColorScheme::Test3);
    auto win = border->create<MainWindow>(screen);

//    win->run_modal();

    screen->run_modal();
}



MainWindow::MainWindow(std::shared_ptr<Screen> screen) :
    screen_ptr(screen), model(std::make_shared<CardModel>())
{
    auto menu = screen->create<VerticalListMenu>(model->get_kindle_booklist());
    menu->run_modal();
    txt = menu->is_cancelled() ? "selected: none" : "selected: " + menu->get_item_string();
}

void MainWindow::paint() const
{
    wclear(win);
    waddnstr(win, txt.c_str(), txt.size());
    wnoutrefresh(win);
}

uint8_t MainWindow::process_key(char32_t ch, bool is_symbol)
{
    if (is_symbol && ch == 27) { // escape
        return PleaseExitModal;
    }
    uint8_t res = 0;
    return res;
}



Footer::Footer()
{

}

uint8_t Footer::process_key(char32_t ch, bool is_symbol)
{
    if (is_symbol && ch == 27) { // escape
        return PleaseExitModal;
    }
    if (is_symbol && ch == 'q') {
        close();
        return PleasePaint;
    }
    return 0;
}

void Footer::paint() const
{
    wclear(win);
    wnoutrefresh(win);
}
