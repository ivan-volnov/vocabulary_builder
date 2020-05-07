#include "main_window.h"
#include <ncurses.h>
#include "global.h"
#include "card_model.h"


constexpr uint64_t border_h = 3;
constexpr uint64_t border_w = 4;
constexpr uint64_t footer_h = 1;
constexpr uint64_t footer_border_w = 1;


MainWindow::MainWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = subwin(stdscr, height - border_h * 2 - footer_h, width - border_w * 2, border_h, border_w);
    footer = subwin(stdscr, footer_h, width - footer_border_w * 2, height - footer_h, footer_border_w);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
    wbkgd(footer, COLOR_PAIR(ColorScheme::ColorWindow));
}

MainWindow::~MainWindow()
{
    delwin(footer);
    delwin(window);
}

void MainWindow::resize(int height, int width)
{
    wresize(window, height - border_h * 2 - footer_h, width - border_w * 2);
    wresize(footer, footer_h, width - footer_border_w * 2);
    mvwin(footer, height - footer_h, footer_border_w);
}

void MainWindow::paint(const CardModel &model)
{
    wclear(footer);
    wmove(footer, 0, 0);
    waddnstr(footer, "ss", -1);
    wnoutrefresh(footer);

    wclear(window);
    wmove(window, 0, 0);

    for(const auto &book : model.get_kindle_booklist()) {
        waddnstr(window, book.c_str(), book.size());
        waddch(window, '\n');
    }

    wnoutrefresh(window);
}
