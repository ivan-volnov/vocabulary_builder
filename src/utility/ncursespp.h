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

#include <vector>


#define COLOR_TRANSPARRENT  -1
#define COLOR_LIGHT_BLACK   8
#define COLOR_LIGHT_RED     9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_YELLOW  11
#define COLOR_LIGHT_BLUE    12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_CYAN    14
#define COLOR_LIGHT_WHITE   15



struct _win_st;
using WINDOW = struct _win_st;



class Window
{
public:
    Window(uint16_t height = 0, uint16_t width = 0, uint16_t y = 0, uint16_t x = 0);
    virtual ~Window() = default;

    virtual void resize(uint16_t height, uint16_t width);
    virtual void move(uint16_t y, uint16_t x);
    virtual void paint() const;

    virtual bool process_key(uint16_t key) const;
    virtual bool process_symbol(char32_t ch) const;

    uint16_t get_height() const;
    uint16_t get_width() const;
    uint16_t get_y() const;
    uint16_t get_x() const;

private:
    uint16_t _height, _width, _y, _x;
};



class CursesWindow : public Window
{
public:
    CursesWindow();
    ~CursesWindow();

    void set_color(int16_t color);

    void resize(uint16_t height, uint16_t width) override;
    void move(uint16_t y, uint16_t x) override;
    void paint() const override;

    WINDOW *get_win();

protected:
    WINDOW *win;
};



class WindowBorder : public Window
{
public:
    WindowBorder(std::shared_ptr<Window> window, uint16_t border_h, uint16_t border_w);

    void resize(uint16_t height, uint16_t width) override;
    void move(uint16_t y, uint16_t x) override;
    void paint() const override;

    bool process_key(uint16_t key) const override;
    bool process_symbol(char32_t ch) const override;

private:
    std::shared_ptr<Window> inner_window;
    uint16_t border_h, border_w;
};



class Layout: public Window
{
public:
    enum LayoutType {
        HorizontalLayout,
        VerticalLayout,
    };

    Layout(LayoutType type, uint16_t splitter_size = 0);

    void add(std::shared_ptr<Window> window);

    void resize(uint16_t height, uint16_t width) override;
    void move(uint16_t y, uint16_t x) override;
    void paint() const override;

    bool process_key(uint16_t key) const override;
    bool process_symbol(char32_t ch) const override;

private:
    uint16_t get_size(Window &window) const;
    uint16_t get_pos(Window &window) const;
    void set_size(Window &window, uint16_t size) const;
    void set_pos(Window &window, uint16_t pos) const;

private:
    LayoutType type;
    std::vector<std::pair<std::shared_ptr<Window>, uint16_t>> windows;
    uint16_t splitter_size;
};



class Screen
{
public:
    Screen();
    virtual ~Screen();

    void exec();

    void show_cursor(bool value = true);
    void set_window(std::shared_ptr<Window> win);
    void set_color(int16_t color);

protected:
    virtual void paint() const;

private:
    std::shared_ptr<Window> window;
};


#endif // CURSES_WINDOW_H
