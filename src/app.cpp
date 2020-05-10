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
    auto win = border->create<MainWindow>();

    screen->paint();

    win->run_modal();

//    tst->set_color(ColorScheme::Test2);
//    screen->paint();

//    screen->run_modal();
}



MainWindow::MainWindow() :
    model(std::make_shared<CardModel>())
{
    list = model->get_kindle_booklist();
}

void MainWindow::paint() const
{
    wclear(win);
    wmove(win, 0, 0);
    for (size_t i = 0; i < list.size(); ++i) {
        auto book = list[i];
        book.resize(get_width(), ' ');
        if (i == current_item) {
            wattron(win, A_STANDOUT);
        }
        waddnstr(win, book.c_str(), book.size());
        if (i == current_item) {
            wattroff(win, A_STANDOUT);
        }
    }
    wnoutrefresh(win);
}

uint8_t MainWindow::process_key(char32_t ch, bool is_symbol)
{
    if (is_symbol && ch == 27) { // escape
        return PleaseExitModal;
    }
    uint8_t res = 0;
    if (ch == 10 && is_symbol) {
        close();
        res |= PleasePaint;
    }
    else if (ch == (is_symbol ? 'j' : KEY_DOWN)) {
        if (current_item < list.size() - 1) {
            ++current_item;
            res |= PleasePaint;
        }
    }
    else if (ch == (is_symbol ? 'k' : KEY_UP)) {
        if (current_item > 0) {
            --current_item;
            res |= PleasePaint;
        }
    }
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
