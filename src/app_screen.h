#ifndef APP_SCREEN_H
#define APP_SCREEN_H

#include <memory>

class MainWindow;
struct _win_st;
using WINDOW = struct _win_st;


class AppScreen
{
public:
    AppScreen();
    ~AppScreen();

    void run();

private:
    void init_colors();
    void paint();

private:
    WINDOW *win;
    std::unique_ptr<MainWindow> main_window;
};

#endif // APP_SCREEN_H
