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

#include "ncursespp.h"
#include <ncurses.h>
#include <locale.h>
#include "utf8_tools.h"


Window::Window(uint16_t height, uint16_t width, uint16_t y, uint16_t x) :
    _height(height), _width(width), _y(y), _x(x)
{

}

void Window::run_modal()
{
    int key;
    utf8::decoder decoder;
    while (true) {
        if ((key = wgetch(stdscr)) < 0) {
            continue;
        }
        if (key <= 0xff) {
            if (decoder.decode_symbol(key)) {
                if (!process_symbol(decoder.symbol())) {
                    return;
                }
                paint();
                doupdate();
            }
        }
        else if (key == KEY_RESIZE) {
            int height, width;
            getmaxyx(stdscr, height, width);
            if (is_term_resized(width, height)) {
                auto top = shared_from_this();
                for (WindowPtr next; (next = top->parent().lock());) {
                    top = next;
                }
                top->resize(height, width);
                resize_term(height, width);
                top->paint();
            }
        }
        else if (key < KEY_MAX && !process_key(key)) {
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

bool Window::process_key(uint16_t) const
{
    return true;
}

bool Window::process_symbol(char32_t) const
{
    return true;
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

void Window::set_parent(WindowPtr win)
{
    _parent = win;
}

void Window::add(WindowPtr)
{

}

void Window::del(WindowPtr)
{

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

void CursesWindow::set_color(uint16_t color)
{
    wbkgd(win, COLOR_PAIR(color));
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



Layout::Layout(Layout::LayoutType type, uint16_t splitter_size) :
    type(type), splitter_size(splitter_size)
{

}

void Layout::resize(uint16_t height, uint16_t width)
{
    if (height == get_height() && width == get_width()) {
        return;
    }
    Window::resize(height, width);
    update_layout();
}

void Layout::move(uint16_t y, uint16_t x)
{
    if (y == get_y() && x == get_x()) {
        return;
    }
    Window::move(y, x);
    auto pos = get_pos(*this);
    for (auto &win : windows) {
        win.first->move(y, x);
        set_pos(*win.first, pos);
        pos += get_size(*win.first) + splitter_size;
    }
}

void Layout::paint() const
{
    for (auto &win : windows) {
        win.first->paint();
    }
}

bool Layout::process_key(uint16_t key) const
{
    for (auto &win : windows) {
        if (!win.first->process_key(key)) {
            return false;
        }
    }
    return true;
}

bool Layout::process_symbol(char32_t ch) const
{
    for (auto &win : windows) {
        if (!win.first->process_symbol(ch)) {
            return false;
        }
    }
    return true;
}

void Layout::add(WindowPtr win)
{
    if (win) {
        windows.push_back({ win, type == VerticalLayout ? win->get_height() : win->get_width() });
        win->set_parent(shared_from_this());
        update_layout();
    }
}

void Layout::del(WindowPtr win)
{
    auto it = std::find_if(windows.begin(), windows.end(), [win](const auto &pair) { return pair.first == win; });
    if (it != windows.end()) {
        windows.erase(it);
        update_layout();
    }
}

void Layout::update_layout()
{
    if (windows.empty()) {
        return;
    }
    auto size = get_size(*this);
    if (splitter_size) {
        size -= std::min(size, static_cast<uint16_t>(splitter_size * (windows.size() - 1)));
    }
    uint16_t expanders = 0;
    for (auto &win : windows) {
        if (!win.second) {
            ++expanders;
        }
        else {
            set_size(*win.first, std::min(size, win.second));
            size -= get_size(*win.first);
        }
    }
    const auto segment = expanders ? size / expanders : 0;
    auto remainder = expanders ? size % expanders : size;
    auto pos = get_pos(*this);
    for (auto &win : windows) {
        if (!win.second) {
            set_size(*win.first, segment + remainder);
            remainder = 0;
        }
        set_pos(*win.first, pos);
        pos += get_size(*win.first) + splitter_size;
    }
}

uint16_t Layout::get_size(Window &window) const
{
    return type == VerticalLayout ? window.get_height() : window.get_width();
}

uint16_t Layout::get_pos(Window &window) const
{
    return type == VerticalLayout ? window.get_y() : window.get_x();
}

void Layout::set_size(Window &window, uint16_t size) const
{
    if (type == VerticalLayout) {
        window.resize(size, get_width());
    }
    else {
        window.resize(get_height(), size);
    }
}

void Layout::set_pos(Window &window, uint16_t pos) const
{
    if (type == VerticalLayout) {
        window.move(pos, get_x());
    }
    else {
        window.move(get_y(), pos);
    }
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
    windows.clear();
    keypad(stdscr, false);
    echo();
    nocbreak();
    clear();
    endwin();
}

void Screen::show_cursor(bool value)
{
    curs_set(value);
}

void Screen::set_color(uint16_t color)
{
    bkgd(COLOR_PAIR(color));
}

void Screen::resize(uint16_t height, uint16_t width)
{
    for (auto &win : windows) {
        win->resize(height, width);
    }
}

void Screen::move(uint16_t y, uint16_t x)
{
    for (auto &win : windows) {
        win->move(y, x);
    }
}

void Screen::paint() const
{
    clear();
    wnoutrefresh(stdscr);
    for (auto &win : windows) {
        win->paint();
    }
    doupdate();
}

bool Screen::process_key(uint16_t key) const
{
    for (auto &win : windows) {
        if (!win->process_key(key)) {
            return false;
        }
    }
    return true;
}

bool Screen::process_symbol(char32_t ch) const
{
    for (auto &win : windows) {
        if (!win->process_symbol(ch)) {
            return false;
        }
    }
    return true;
}

void Screen::add(WindowPtr win)
{
    auto it = std::find(windows.begin(), windows.end(), win);
    if (it == windows.end()) {
        win->move(0, 0);
        win->resize(get_height(), get_width());
        win->set_parent(shared_from_this());
        windows.push_back(std::move(win));
    }
}

void Screen::del(WindowPtr win)
{
    auto it = std::find(windows.begin(), windows.end(), win);
    if (it != windows.end()) {
        (*it)->set_parent(nullptr);
        windows.erase(it);
    }
}
