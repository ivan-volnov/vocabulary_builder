#include "app.h"
#include <ncurses.h>
#include "card_model.h"



MainWindow::MainWindow(std::shared_ptr<Screen> screen, std::shared_ptr<CardModel> model_, size_t current_card_idx) :
    screen_ptr(screen), model(std::move(model_)), current_card_idx(current_card_idx)
{
    assert(model->size());
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
    else if (ch == 'i' && is_symbol) {
        if (auto screen = screen_ptr.lock()) {
            screen->show_cursor(true);
            auto border = screen->create<SimpleBorder>(4, 5);
            auto line = border->create<InputLine>("New word: ");
            line->run_modal();
            if (!line->is_cancelled()) {
                model->insert_new_card(line->get_text(), current_card_idx);
                current_card_idx_changed(current_card_idx + 1);
            }
            border->close();
            screen->show_cursor(false);
        }
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
    else if (ch == (is_symbol ? 'k' : KEY_UP) || (ch == 'b' && is_symbol)) {
        if (current_card_idx > 0) {
            current_card_idx_changed(current_card_idx--);
        }
    }
    else if (ch == (is_symbol ? 'j' : KEY_DOWN) || (ch == 10 && is_symbol)) { // enter
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
    for (const std::string str : {"[A]Add", "[I]Insert", "[E]Edit", "[J]Next", "[K]Back", "[R]Reload"}) {
//        wattron(win, A_STANDOUT );
        waddnstr(win, str.c_str(), std::min(str.size(), static_cast<size_t>(get_width() - getcurx(win))));
//        wattroff(win, A_STANDOUT);
        waddch(win, ' ');
        waddch(win, ' ');
    }
    wnoutrefresh(win);
}
