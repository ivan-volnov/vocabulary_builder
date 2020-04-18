#include "main_window.h"
#include <ncurses.h>
#include "global.h"


constexpr uint64_t border_h = 3;
constexpr uint64_t border_w = 4;


MainWindow::MainWindow()
{
    int height, width;
    getmaxyx(stdscr, height, width);
    window = newwin(height - border_h * 2, width - border_w * 2, border_h, border_w);
    wbkgd(window, COLOR_PAIR(ColorScheme::ColorWindow));
}

MainWindow::~MainWindow()
{
    delwin(window);
}

void MainWindow::resize(int height, int width)
{
    wresize(window, height - border_h * 2, width - border_w * 2);
}

void MainWindow::paint()
{
    wclear(window);
    wmove(window, 0, 0);
    wnoutrefresh(window);
}
