#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>


struct _win_st;
using WINDOW = struct _win_st;
typedef unsigned int chtype;
class CardModel;


class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

public:
    void resize(int height, int width);
    void paint(const CardModel &model);

public:
    WINDOW *window;
    WINDOW *footer;
};

#endif // MAINWINDOW_H
