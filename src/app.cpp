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
    layout->create<CursesWindow>()->set_color(ColorScheme::Test);
    layout->create<CursesWindow>()->set_color(ColorScheme::Test2);

    auto border = screen->create<SimpleBorder>(3, 4)->create<CursesBorder>();
    border->set_color(ColorScheme::Test3);
    auto win = border->create<MainWindow>();

    screen->paint();

    win->run_modal();
}



MainWindow::MainWindow() :
    model(std::make_shared<CardModel>())
{

}

void MainWindow::paint() const
{
    wclear(win);
    wmove(win, 0, 0);
    for (const auto &book : model->get_kindle_booklist()) {
        waddnstr(win, book.c_str(), book.size());
        waddch(win, '\n');
    }
    wnoutrefresh(win);
}

bool MainWindow::process_symbol(char32_t ch) const
{
    return ch != 27; // escape
}



Footer::Footer()
{

}

void Footer::paint() const
{
    wclear(win);
    wnoutrefresh(win);
}
