#include "app.h"
#include <iostream>
#include <libs/argh.h>
#include "utility/tools.h"
#include "card_model.h"
#include "config.h"



void run(int argc, char *argv[])
{
    argh::parser cmdl(argc, argv);
    Config::instance().set_sound_enabled(cmdl["s"]);

    auto model = std::make_shared<CardModel>();

    if (cmdl["check_collection"]) {
        model->anki_check_collection();
        return;
    }
    if (cmdl["fix_collection"]) {
        model->anki_fix_collection();
        return;
    }


    {
        auto screen = std::make_shared<Screen>();
        screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
        screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
        screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);
        screen->show_cursor(false);

        model->open_kindle_db();
        auto menu = screen->create<VerticalListMenu>(model->get_kindle_booklist());
        menu->run_modal();
        if (menu->is_cancelled()) {
            throw std::runtime_error("You must select a book first");
        }
        size_t current_card_idx = 0;
        model->load_from_kindle(menu->get_item_string(), current_card_idx);
        model->close_kindle_db();

        auto layout = screen->create<SimpleBorder>()->create<VerticalLayout>();
        layout->create<SimpleBorder>(3, 4)->create<MainWindow>(screen, model, current_card_idx);
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
