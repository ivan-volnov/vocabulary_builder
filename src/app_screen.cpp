#include "app_screen.h"
#include <ncurses.h>
#include <locale.h>
#include "main_window.h"
#include "global.h"



AppScreen::AppScreen()
{
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    timeout(-1);
    init_colors();
    bkgd(COLOR_PAIR(ColorScheme::ColorWindow));
    main_window = std::make_unique<MainWindow>();
}

AppScreen::~AppScreen()
{
    main_window.reset();
    keypad(stdscr, false);
    echo();
    nocbreak();
    clear();
    endwin();
}

void AppScreen::run()
{
    paint();
    int key, height, width;
    while (true)
    {
        key = wgetch(stdscr);
        switch (key) {
        case 27: // escape
            return;
        case ERR:
            break;
        case KEY_RESIZE:
            getmaxyx(stdscr, height, width);
            if (is_term_resized(width, height)) {
                clear();
                main_window->resize(height, width);
                resize_term(height, width);
                paint();
            }
            break;
        default:
//            if (!process_key(key)) {
//                return;
//            }
            main_window->paint();
            doupdate();
            break;
        }
    }
}

void AppScreen::init_colors()
{
    start_color();
    use_default_colors();
    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorError, COLOR_RED, -1);
    init_pair(ColorScheme::ColorMultipleErrors, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::ColorGray, 251, -1); // https://jonasjacek.github.io/colors/
}

void AppScreen::paint()
{
    clear();
    int height, width;
    getmaxyx(stdscr, height, width);
    mvwhline(stdscr, height - 2, 1, '_', width - 2);
    wnoutrefresh(stdscr);
    main_window->paint();
    doupdate();
}
