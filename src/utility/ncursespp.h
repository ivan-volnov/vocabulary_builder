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

#ifndef CURSES_WINDOW_H
#define CURSES_WINDOW_H

#include <list>


#define COLOR_TRANSPARRENT  -1
#define COLOR_BLACK          0
#define COLOR_RED            1
#define COLOR_GREEN          2
#define COLOR_YELLOW         3
#define COLOR_BLUE           4
#define COLOR_MAGENTA        5
#define COLOR_CYAN           6
#define COLOR_WHITE          7
#define COLOR_LIGHT_BLACK    8
#define COLOR_LIGHT_RED      9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_YELLOW  11
#define COLOR_LIGHT_BLUE    12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_CYAN    14
#define COLOR_LIGHT_WHITE   15
// for more colors get values on https://jonasjacek.github.io/colors/



struct _win_st;
using NCursesWindowPtr = struct _win_st;

class Window;
using WindowPtr = std::shared_ptr<Window>;



class Window : public std::enable_shared_from_this<Window>
{
public:
    enum ResultMasks : uint8_t
    {
        // set |    check &    toggle ^    reset & ~
        PleasePaint             = 1 << 0,
        PleaseExitModal         = 1 << 7,
    };

    Window(uint16_t height = 0, uint16_t width = 0, uint16_t y = 0, uint16_t x = 0);
    virtual ~Window() = default;

    void run_modal();

    virtual void resize(uint16_t height, uint16_t width);
    virtual void move(uint16_t y, uint16_t x);
    virtual void paint() const;

    virtual uint8_t process_key(uint16_t key);
    virtual uint8_t process_symbol(char32_t ch);
    virtual void close();

    uint16_t get_height() const;
    uint16_t get_width() const;
    uint16_t get_y() const;
    uint16_t get_x() const;

    std::weak_ptr<Window> parent() const;
    void set_parent(WindowPtr win);

    virtual void add(WindowPtr win);
    virtual void del(WindowPtr win);

    template<class T, class ...Args>
    typename std::enable_if_t<std::is_base_of_v<Window, T>, std::shared_ptr<T>> create(Args&& ...args)
    {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        add(ptr);
        return ptr;
    }

private:
    uint16_t _height, _width, _y, _x;
    std::weak_ptr<Window> _parent;
};



class CursesWindow : public Window
{
public:
    CursesWindow();
    CursesWindow(const CursesWindow &) = delete;
    CursesWindow &operator=(const CursesWindow &) = delete;
    ~CursesWindow();

    void set_color(const std::string &color);

    void resize(uint16_t height, uint16_t width) override;
    void move(uint16_t y, uint16_t x) override;
    void paint() const override;

    NCursesWindowPtr *get_win();

protected:
    NCursesWindowPtr *win;
};



template <typename T>
class Border : public T
{
public:
    Border(uint16_t border_h = 1, uint16_t border_w = 1) :
        border_h(border_h), border_w(border_w)
    {

    }

    void resize(uint16_t height, uint16_t width) override
    {
        if (height != T::get_height() || width != T::get_width()) {
            T::resize(height, width);
            if (inner_window) {
                inner_window->resize(std::max(0, static_cast<int32_t>(height) - border_h * 2),
                                     std::max(0, static_cast<int32_t>(width)  - border_w * 2));
            }
        }
    }

    void move(uint16_t y, uint16_t x) override
    {
        if (y != T::get_y() || x != T::get_x()) {
            T::move(y, x);
            if (inner_window) {
                inner_window->move(y + border_h, x + border_w);
            }
        }
    }

    void paint() const override
    {
        T::paint();
        if (inner_window) {
            inner_window->paint();
        }
    }

    uint8_t process_key(uint16_t key) override
    {
        return inner_window ? inner_window->process_key(key) : 0;
    }

    uint8_t process_symbol(char32_t ch) override
    {
        return inner_window ? inner_window->process_symbol(ch) : 0;
    }

    void add(WindowPtr win) override
    {
        if ((inner_window = std::move(win))) {
            inner_window->set_parent(T::shared_from_this());
            inner_window->move(T::get_y() + border_h, T::get_x() + border_w);
            inner_window->resize(std::max(0, static_cast<int32_t>(T::get_height()) - border_h * 2),
                                 std::max(0, static_cast<int32_t>(T::get_width())  - border_w * 2));
        }
    }

    void del(WindowPtr win) override
    {
        if (inner_window == win) {
            inner_window->set_parent(nullptr);
            inner_window = nullptr;
        }
    }

protected:
    WindowPtr inner_window;

private:
    uint16_t border_h, border_w;
};

using SimpleBorder = Border<Window>;
using CursesBorder = Border<CursesWindow>;



template<bool vertical>
class Layout: public Window
{
public:

    Layout(uint16_t splitter_size = 0) :
        splitter_size(splitter_size)
    {

    }

    void resize(uint16_t height, uint16_t width) override
    {
        if (height == get_height() && width == get_width()) {
            return;
        }
        Window::resize(height, width);
        update_layout();
    }

    void move(uint16_t y, uint16_t x) override
    {
        if (y == get_y() && x == get_x()) {
            return;
        }
        Window::move(y, x);
        auto pos = get_pos(*this);
        for (auto it = windows.begin(); it != windows.end();) {
            auto win = (it++)->first;
            win->move(y, x);
            set_pos(*win, pos);
            pos += get_size(*win) + splitter_size;
        }
    }

    void paint() const override
    {
        for (auto it = windows.begin(); it != windows.end();) {
            (it++)->first->paint();
        }
    }

    uint8_t process_key(uint16_t key) override
    {
        uint8_t res = 0;
        for (auto it = windows.begin(); it != windows.end();) {
            res |= (it++)->first->process_key(key);
        }
        return res;
    }

    uint8_t process_symbol(char32_t ch) override
    {
        uint8_t res = 0;
        for (auto it = windows.begin(); it != windows.end();) {
            res |= (it++)->first->process_symbol(ch);
        }
        return res;
    }

    void add(WindowPtr win) override
    {
        if (win) {
            windows.push_back({ win, vertical ? win->get_height() : win->get_width() });
            win->set_parent(shared_from_this());
            update_layout();
        }
    }

    void del(WindowPtr win) override
    {
        auto it = std::find_if(windows.begin(), windows.end(), [win](const auto &pair) { return pair.first == win; });
        if (it != windows.end()) {
            it->first->set_parent(nullptr);
            windows.erase(it);
            update_layout();
        }
    }

private:
    void update_layout()
    {
        if (windows.empty()) {
            return;
        }
        auto size = get_size(*this);
        if (splitter_size) {
            size -= std::min(size, static_cast<uint16_t>(splitter_size * (windows.size() - 1)));
        }
        uint16_t expanders = 0;
        for (auto it = windows.begin(); it != windows.end();) {
            auto win = *it++;
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
        for (auto it = windows.begin(); it != windows.end();) {
            auto win = *it++;
            if (!win.second) {
                set_size(*win.first, segment + remainder);
                remainder = 0;
            }
            set_pos(*win.first, pos);
            pos += get_size(*win.first) + splitter_size;
        }
    }

    uint16_t get_size(Window &window) const
    {
        return vertical ? window.get_height() : window.get_width();
    }

    uint16_t get_pos(Window &window) const
    {
        return vertical ? window.get_y() : window.get_x();
    }

    void set_size(Window &window, uint16_t size) const
    {
        vertical ? window.resize(size, get_width()) : window.resize(get_height(), size);
    }

    void set_pos(Window &window, uint16_t pos) const
    {
        vertical ? window.move(pos, get_x()) : window.move(get_y(), pos);
    }

private:
    std::list<std::pair<WindowPtr, uint16_t>> windows;
    uint16_t splitter_size;
};

using VerticalLayout = Layout<true>;
using HorizontalLayout = Layout<false>;



class Screen : public Window
{
public:
    Screen();
    ~Screen();

    void show_cursor(bool value = true);
    void init_color(const std::string &color, int16_t foregroung, int16_t background);

    void set_color(const std::string &color);

    void resize(uint16_t height, uint16_t width) override;
    void move(uint16_t y, uint16_t x) override;
    void paint() const override;

    uint8_t process_key(uint16_t key) override;
    uint8_t process_symbol(char32_t ch) override;

    void add(WindowPtr win) override;
    void del(WindowPtr win) override;

private:
    std::list<WindowPtr> windows;
};


#endif // CURSES_WINDOW_H
