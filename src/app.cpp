#include "app.h"
#include <ncurses.h>
#include "card_model.h"


AppScreen::AppScreen()
{
    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorError, COLOR_RED, COLOR_TRANSPARRENT);
    init_pair(ColorScheme::ColorGray, 251, COLOR_TRANSPARRENT); // https://jonasjacek.github.io/colors/

    show_cursor(false);

    auto main_window = std::make_shared<MainWindow>();

    auto footer = std::make_shared<Footer>();
    footer->resize(2, 0);

    auto layout = std::make_shared<Layout>(Layout::VerticalLayout, 0);
    layout->add(std::make_shared<WindowBorder>(main_window, 2, 3));
    layout->add(footer);

    set_window(std::make_shared<WindowBorder>(layout, 1, 1));
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
