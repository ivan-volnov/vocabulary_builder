#include "app.h"
#include <ncurses.h>
#include "card_model.h"



App::App()
{
    screen = std::make_shared<Screen>();

    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorError, COLOR_RED, COLOR_TRANSPARRENT);
    init_pair(ColorScheme::ColorGray, 251, COLOR_TRANSPARRENT); // https://jonasjacek.github.io/colors/

    init_pair(ColorScheme::ColorTest, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::ColorTest2, COLOR_WHITE, COLOR_BLUE);
    init_pair(ColorScheme::ColorTest3, COLOR_WHITE, COLOR_BLACK);

    screen->show_cursor(false);
}

void App::run()
{
    auto layout = screen->create<SimpleBorder>()->create<Layout>(Layout::HorizontalLayout, 1);
    layout->create<CursesWindow>()->set_color(ColorScheme::ColorTest);
    layout->create<CursesWindow>()->set_color(ColorScheme::ColorTest2);

    auto border = screen->create<SimpleBorder>(3, 4)->create<CursesBorder>();
    border->set_color(ColorScheme::ColorTest3);
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
