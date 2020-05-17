#include "app.h"
#include <iostream>
#include <libs/argh.h>
#include "utility/tools.h"
#include "card_model.h"
#include "config.h"


namespace ColorScheme {

constexpr auto Window = "window";
constexpr auto Error  = "error";
constexpr auto Gray   = "gray";

}



void run(int argc, char *argv[])
{
    argh::parser cmdl(argc, argv);
    Config::instance().set_sound_enabled(cmdl["sound"]);
    {
        auto screen = std::make_shared<Screen>();
        screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
        screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
        screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);
        screen->show_cursor(false);
        auto layout = screen->create<SimpleBorder>()->create<VerticalLayout>();
        layout->create<SimpleBorder>(3, 4)->create<MainWindow>(screen);
        layout->create<Footer>();
        screen->run_modal();
    }
}



int main(int argc, char *argv[])
{
    if (tools::am_I_being_debugged()) {
        run(argc, argv);
    }
    else {
        try {
            run(argc, argv);
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}
