/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT

Copyright (c) 2020 Ivan Volnov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "tiled_ncurses.h"
#include <ncurses.h>
#include <locale.h>
#include <mutex>
#include "utf8_tools.h"



class internal
{
public:
    static internal &instance()
    {
        static internal i;
        return i;
    }

    void init_color(const std::string &color, int16_t foregroung, int16_t background)
    {
        std::lock_guard lock(mutex);
        auto it = std::find(colors.begin(), colors.end(), color);
        if (it == colors.end()) {
            init_pair(colors.size(), foregroung, background);
            colors.push_back(color);
        }
    }

    uint32_t get_color(const std::string &color)
    {
        std::lock_guard lock(mutex);
        for (uint16_t i = 0; i < colors.size(); ++i) {
            if (colors[i] == color) {
                return COLOR_PAIR(i);
            }
        }
        assert(!"color haven't found");
        return 0;
    }

private:
    internal() = default;
    std::vector<std::string> colors;
    std::mutex mutex;
};



Window::Window(uint16_t height, uint16_t width, uint16_t y, uint16_t x) :
    _height(height), _width(width), _y(y), _x(x)
{

}

void Window::run_modal()
{
    get_top_window()->paint();
    int key;
    utf8::decoder decoder;
    uint8_t res;
    const bool parent_state = _parent.expired();
    while (parent_state || !_parent.expired()) {
        if ((key = wgetch(stdscr)) < 0) {
            continue;
        }
        res = 0;
        if (key <= 0xff) {
            if (decoder.decode_symbol(key)) {
                res = process_key(decoder.symbol(), true);
            }
        }
        else if (key == KEY_RESIZE) {
            int height, width;
            getmaxyx(stdscr, height, width);
            if (is_term_resized(width, height)) {
                auto top = get_top_window();
                top->resize(height, width);
                resize_term(height, width);
                top->paint();
            }
            continue;
        }
        else if (key < KEY_MAX) {
            res = process_key(key, false);
        }
        if (res & PleasePaint) {
            paint();
            doupdate();
        }
        if (res & PleaseExitModal) {
            return;
        }
    }
}

void Window::resize(uint16_t height, uint16_t width)
{
    _height = height;
    _width = width;
}

void Window::move(uint16_t y, uint16_t x)
{
    _y = y;
    _x = x;
}

void Window::paint() const
{

}

uint8_t Window::process_key(char32_t, bool)
{
    return 0;
}

void Window::close()
{
    if (auto p = parent().lock()) {
        auto self = shared_from_this();
        p->remove_window(self);
    }
}

uint16_t Window::get_height() const
{
    return _height;
}

uint16_t Window::get_width() const
{
    return _width;
}

uint16_t Window::get_y() const
{
    return _y;
}

uint16_t Window::get_x() const
{
    return _x;
}

std::weak_ptr<Window> Window::parent() const
{
    return _parent;
}

void Window::add_window(WindowPtr win)
{
    win->_parent = shared_from_this();
}

void Window::remove_window(WindowPtr win)
{
    win->_parent.reset();
}

WindowPtr Window::get_top_window()
{
    auto top = shared_from_this();
    for (WindowPtr next; (next = top->parent().lock());) {
        top = next;
    }
    return top;
}



CursesWindow::CursesWindow()
{
    win = subwin(stdscr, 1, 1, 0, 0);
    leaveok(win, true);
}

CursesWindow::~CursesWindow()
{
    delwin(win);
}

void CursesWindow::set_color(const std::string &color)
{
    wbkgd(win, internal::instance().get_color(color));
}

void CursesWindow::resize(uint16_t height, uint16_t width)
{
    if (height != get_height() || width != get_width()) {
        Window::resize(height, width);
        wresize(win, height < 1 ? 1 : height, width < 1 ? 1 : width);
    }
}

void CursesWindow::move(uint16_t y, uint16_t x)
{
    if (y != get_y() || x != get_x()) {
        Window::move(y, x);
        mvwin(win, y, x);
    }
}

void CursesWindow::paint() const
{
    wclear(win);
    wnoutrefresh(win);
}

NCursesWindow *CursesWindow::get_win()
{
    return win;
}



VerticalListMenu::VerticalListMenu(const std::vector<std::string> &list, Callback &&callback) :
    list(list), callback(std::move(callback))
{
}

VerticalListMenu::VerticalListMenu(std::vector<std::string> &&list, VerticalListMenu::Callback &&callback) :
    list(std::move(list)), callback(std::move(callback))
{

}

void VerticalListMenu::paint() const
{
    wclear(win);
    wmove(win, 0, 0);
    for (size_t i = 0; i < list.size(); ++i) {
        auto item = list[i];
        auto len = utf8::strlen(item);
        if (len > get_width()) {
            item.erase(utf8::next(item.begin(), item.end(), get_width() - 1), std::prev(item.end()));
        }
        else if (len < get_width()) {
            item.append(get_width() - len, ' ');
        }
        if (i == current_item) {
            wattron(win, A_STANDOUT);
        }
        waddnstr(win, item.c_str(), item.size());
        if (i == current_item) {
            wattroff(win, A_STANDOUT);
        }
    }
    wnoutrefresh(win);
}

uint8_t VerticalListMenu::process_key(char32_t ch, bool is_symbol)
{
    if (ch == 10 && is_symbol) {
        callback(current_item);
        close();
    }
    else if (ch == (is_symbol ? 'j' : KEY_DOWN)) {
        if (current_item < list.size() - 1) {
            ++current_item;
        }
    }
    else if (ch == (is_symbol ? 'k' : KEY_UP)) {
        if (current_item > 0) {
            --current_item;
        }
    }
    else {
        return 0;
    }
    return PleasePaint;
}



Screen::Screen()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    timeout(-1);
    start_color();
    use_default_colors();
    static_cast<Window &>(*this) = Window(getmaxy(stdscr), getmaxx(stdscr));
}

Screen::~Screen()
{
    for (auto it = layers.begin(); it != layers.end(); it = layers.erase(it)) {
        Window::remove_window(*it);
    }
    keypad(stdscr, false);
    echo();
    nocbreak();
    ::clear();
    endwin();
}

void Screen::show_cursor(bool value)
{
    curs_set(value);
}

void Screen::init_color(const std::string &color, int16_t foregroung, int16_t background)
{
    internal::instance().init_color(color, foregroung, background);
}

void Screen::set_color(const std::string &color)
{
    bkgd(internal::instance().get_color(color));
}

void Screen::resize(uint16_t height, uint16_t width)
{
    for (auto it = layers.begin(); it != layers.end();) {
        (*it++)->resize(height, width);
    }
}

void Screen::move(uint16_t y, uint16_t x)
{
    for (auto it = layers.begin(); it != layers.end();) {
        (*it++)->move(y, x);
    }
}

void Screen::paint() const
{
    ::clear();
    wnoutrefresh(stdscr);
    for (auto it = layers.begin(); it != layers.end();) {
        (*it++)->paint();
    }
    doupdate();
}

uint8_t Screen::process_key(char32_t ch, bool is_symbol)
{
    uint8_t res = 0;
    for (auto it = layers.begin(); it != layers.end();) {
        res |= (*it++)->process_key(ch, is_symbol);
    }
    return res;
}

void Screen::add_window(WindowPtr win)
{
    auto it = std::find(layers.begin(), layers.end(), win);
    if (it == layers.end()) {
        Window::add_window(win);
        win->move(0, 0);
        win->resize(get_height(), get_width());
        layers.push_back(std::move(win));
    }
}

void Screen::remove_window(WindowPtr win)
{
    auto it = std::find(layers.begin(), layers.end(), win);
    if (it != layers.end()) {
        Window::remove_window(win);
        layers.erase(it);
    }
}
