#include "app.h"
#include <ncurses.h>
#include "card_model.h"

namespace ColorScheme {

constexpr auto Window = "window";
constexpr auto Error  = "error";
constexpr auto Gray   = "gray";

}


App::App()
{
    screen = std::make_shared<Screen>();
    screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
    screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);

    screen->show_cursor(false);
}

void App::run()
{
    auto layout = screen->create<SimpleBorder>()->create<VerticalLayout>();
    layout->create<SimpleBorder>(3, 4)->create<MainWindow>(screen);
    layout->create<Footer>();
    screen->run_modal();
}



MainWindow::MainWindow(std::shared_ptr<Screen> screen) :
    screen_ptr(screen), model(std::make_unique<CardModel>())
{
    auto menu = screen->create<VerticalListMenu>(model->get_kindle_booklist());
    menu->run_modal();
    if (menu->is_cancelled()) {
        throw std::runtime_error("You must select a book first");
    }
    model->load_from_kindle(menu->get_item_string(), current_card_idx);
    model->close_kindle_db();
    current_card_idx_changed(-1);
}

void MainWindow::paint() const
{
    wclear(win);

    auto &card = model->get_card(current_card_idx);

    print("Front : " + card.get_front());
    print("Back  : " + card.get_back());
    print("PoS   : " + card.get_pos_string());
    print("Level : " + card.get_level_string());

    wmove(win, get_height() - 1, 0);
    print("Left  : " + std::to_string(model->size() - current_card_idx));

    wnoutrefresh(win);
}

uint8_t MainWindow::process_key(char32_t ch, bool is_symbol)
{
    if (ch == 27 && is_symbol) { // escape
        return PleaseExitModal;
    }
    else if (ch == 'a' && is_symbol) {
        model->anki_add_card(model->get_card(current_card_idx));
    }
    else if (ch == 'e' && is_symbol) {
        model->anki_open_browser(model->get_card(current_card_idx));
    }
    else if (ch == 'r' && is_symbol) {
        model->anki_reload_card(model->get_card(current_card_idx));
    }
    else if (ch == (is_symbol ? 'k' : KEY_UP)) {
        if (current_card_idx > 0) {
            current_card_idx_changed(current_card_idx--);
        }
    }
    else if (ch == (is_symbol ? 'j' : KEY_DOWN)) {
        if (current_card_idx + 1 < model->size()) {
            current_card_idx_changed(current_card_idx++);
        }
    }
    else {
        return 0;
    }
    return PleasePaint;
}

void MainWindow::print(const std::string &str) const
{
    waddnstr(win, str.c_str(), str.size());
    waddch(win, '\n');
}

void MainWindow::current_card_idx_changed(size_t prev_card_idx)
{
    if (prev_card_idx != static_cast<size_t>(-1)) {
        model->anki_reload_card(model->get_card(prev_card_idx));
    }
    auto word = model->get_card(current_card_idx).get_front();
    model->say(word);
    model->look_up_in_safari(word);
}



Footer::Footer() :
    CursesWindow(1)
{

}

void Footer::paint() const
{
    wclear(win);
//    whline(win, '_', get_width());
//    wmove(win, 1, 0);
    for (const std::string str : {"[ESC]Exit", "[A]Add", "[E]Edit", "[J]Next", "[K]Back", "[R]Reload"}) {
//        wattron(win, A_STANDOUT );
        waddnstr(win, str.c_str(), std::min(str.size(), static_cast<size_t>(get_width() - getcurx(win))));
//        wattroff(win, A_STANDOUT);
        waddch(win, ' ');
        waddch(win, ' ');
    }
    wnoutrefresh(win);
}
