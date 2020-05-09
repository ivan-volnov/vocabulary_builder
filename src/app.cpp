#include "app.h"
#include <ncurses.h>
#include "card_model.h"


AppScreen::AppScreen()
{
    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorError, COLOR_RED, COLOR_TRANSPARRENT);
    init_pair(ColorScheme::ColorGray, 251, COLOR_TRANSPARRENT); // https://jonasjacek.github.io/colors/

    init_pair(ColorScheme::ColorTest, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::ColorTest2, COLOR_WHITE, COLOR_BLUE);
    init_pair(ColorScheme::ColorTest3, COLOR_WHITE, COLOR_BLACK);

    show_cursor(false);

    auto main_window = std::make_shared<MainWindow>();
    main_window->set_color(ColorScheme::ColorWindow);

//    auto footer = std::make_shared<Footer>();
//    footer->resize(2, 0);

    auto layout2 = std::make_shared<Layout>(Layout::HorizontalLayout, 1);
    auto win1 = std::make_shared<CursesWindow>();
    win1->set_color(ColorScheme::ColorTest);
    layout2->add(win1);
    auto win2 = std::make_shared<CursesWindow>();
    win2->set_color(ColorScheme::ColorTest2);
    layout2->add(win2);

//    auto layout = std::make_shared<Layout>(Layout::VerticalLayout, 0);
//    layout->add(std::make_shared<WindowBorder>(main_window, 2, 3));
//    layout->add(footer);

//    set_window(std::make_shared<WindowBorder>(layout, 1, 1));
    set_window(std::make_shared<WindowBorder>(layout2, 1, 1));

    auto border = std::make_shared<CursesBorder>(main_window);
    border->set_color(ColorScheme::ColorTest3);

    set_modal(std::make_shared<WindowBorder>(border, 3, 4));
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
