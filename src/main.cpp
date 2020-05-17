#include "app.h"
#include <iostream>
#include <libs/argh.h>
#include "utility/tools.h"
#include "card_model.h"
#include "config.h"



void run(int argc, char *argv[])
{
    argh::parser cmdl(argc, argv);
    Config::instance().set_sound_enabled(cmdl[{"s", "sound"}]);

    if (cmdl[{"h", "help"}]) {
        std::cout << "Usage: vocabulary_builder [options]\n\n"
                     "Optional arguments:\n"
                     "-h --help               show this help message and exit\n"
                     "-k --kindle             import cards from kindle\n"
                     "-s --sound              read aloud current card\n"
                     "--check_collection      check whole collection\n"
                     "--fix_collection        fix whole collection\n"
                  << std::endl;
        return;
    }

    auto model = std::make_shared<CardModel>();

    if (cmdl["check_collection"]) {
        model->anki_fix_collection(false);
        return;
    }
    if (cmdl["fix_collection"]) {
        model->anki_fix_collection(true);
        return;
    }

    auto screen = std::make_shared<Screen>();
    screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
    screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);
    screen->init_color(ColorScheme::Blue, COLOR_BLUE, COLOR_TRANSPARRENT);
    screen->show_cursor(false);

    size_t current_card_idx = 0;

    if (cmdl[{"k", "kindle"}]) {
        model->open_kindle_db();
        auto menu = screen->create<VerticalListMenu>(model->get_kindle_booklist());
        menu->run_modal();
        if (menu->is_cancelled()) {
            throw std::runtime_error("You must select a book first");
        }
        model->load_from_kindle(menu->get_item_string(), current_card_idx);
        model->close_kindle_db();
    }
    else {
        screen->show_cursor(true);
        auto border = screen->create<SimpleBorder>(3, 4);
        auto line = border->create<InputLine>("New word: ");
        line->run_modal();
        if (line->is_cancelled()) {
            throw std::runtime_error("You must add at least one word");
        }
        model->insert_new_card(line->get_text(), current_card_idx);
        border->close();
        screen->show_cursor(false);
    }

    auto layout = screen->create<VerticalLayout>();
    auto border = layout->create<SimpleBorder>(3, 4);
    layout->create<Footer>();
    auto progress = layout->create<ProgressBar>(ColorScheme::Blue);
    border->create<MainWindow>(screen, progress, model, current_card_idx);
    screen->run_modal();
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
