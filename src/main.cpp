#include "app.hpp"
#include "card_model.hpp"
#include "config.hpp"
#include <st/logger.hpp>

inline constexpr auto APP_HELP =
    R"(
Usage:
  vocabulary_builder [options]
  vocabulary_builder (-h | --help | -v | --version)

Options:
  -h --help                     Show this help message and exit
  -v --version                  Display version information and exit
  -k --kindle                   Import cards from Kindle
  -l --leech                    Work with leech cards
  -s --sound                    Read aloud current card
  --query <word>                Query vocabulary profile
  --suspended                   Work with suspended cards
  --check-collection            Check whole collection
  --fix-collection              Fix whole collection
  --nvim-export                 Export to ~/.config/nvim/dictionary.json
  --nvim-export <file>          Export to <file>
)";

using namespace st;

auto main(int argc, char *argv[]) -> int
{
    try {
        bool kindle{};
        bool leech{};
        bool sound{};
        const char *query_word{};
        bool suspended{};
        bool check_collection{};
        bool fix_collection{};
        const char *nvim_export_filename{};

        for (auto it = argv + 1, end = argv + argc; it != end; ++it) {
            std::string_view arg{*it};
            if (arg == "-h" || arg == "--help") {
                fmt::print(APP_HELP);
                return 0;
            }
            if (arg == "-v" || arg == "--version") {
                fmt::print("vocabulary_builder v0.1\n");
                return 0;
            }
            if (arg == "-k" || arg == "--kindle") {
                kindle = true;
                continue;
            }
            if (arg == "-l" || arg == "--leech") {
                leech = true;
                continue;
            }
            if (arg == "-s" || arg == "--sound") {
                sound = true;
                continue;
            }
            if (arg == "--query") {
                if (it + 1 == end) {
                    fmt::print("{} requires an argument\n{}", arg, APP_HELP);
                    return 1;
                }
                query_word = *++it;
                continue;
            }
            if (arg == "--suspended") {
                suspended = true;
                continue;
            }
            if (arg == "--check-collection") {
                check_collection = true;
                continue;
            }
            if (arg == "--fix-collection") {
                fix_collection = true;
                continue;
            }
            if (arg == "--nvim-export") {
                if (it + 1 == end) {
                    nvim_export_filename = "~/.config/nvim/dictionary.json";
                }
                else {
                    nvim_export_filename = *++it;
                }
                continue;
            }
            fmt::print("Unexpected argument: {}\n{}", arg, APP_HELP);
            return 1;
        }

        Config::instance().set_sound_enabled(sound);

        auto model = std::make_shared<CardModel>();

        if (check_collection) {
            model->anki_fix_collection(false);
            return 0;
        }
        if (fix_collection) {
            model->anki_fix_collection(true);
            return 0;
        }
        if (nvim_export_filename) {
            model->anki_nvim_export(nvim_export_filename);
            return 0;
        }
        if (query_word) {
            model->query_vocabulary_profile(query_word);
            return 0;
        }

        auto screen = std::make_shared<Screen>();
        screen->init_color(ColorScheme::Window, COLOR_BLACK, COLOR_WHITE);
        screen->init_color(ColorScheme::Error, COLOR_RED, COLOR_TRANSPARRENT);
        screen->init_color(ColorScheme::Gray, 251, COLOR_TRANSPARRENT);
        screen->init_color(ColorScheme::Blue, COLOR_BLUE, COLOR_TRANSPARRENT);
        screen->show_cursor(false);

        size_t current_card_idx = 0;

        if (kindle) {
            size_t item_idx = 0;
            model->open_kindle_db();
            auto booklist = model->get_kindle_booklist();
            if (auto last_book = Config::get_state<std::string>("kindle_book");
                !last_book.empty()) {
                auto it = std::find(booklist.begin(), booklist.end(), last_book);
                if (it != booklist.end()) {
                    item_idx = std::distance(booklist.begin(), it);
                }
            }
            auto menu = screen->create<VerticalListMenu>(std::move(booklist), item_idx);
            menu->run_modal();
            if (menu->is_cancelled()) {
                throw std::runtime_error("You must select a book first");
            }
            model->load_from_kindle(menu->get_item_string(), current_card_idx);
            model->close_kindle_db();
            Config::set_state("kindle_book", menu->get_item_string());
        }
        else if (leech) {
            model->load_leech_cards();
        }
        else if (suspended) {
            model->load_suspended_cards();
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
        auto main_window =
            border->create<MainWindow>(screen, progress, model, current_card_idx);
        screen->run_modal();
        main_window->save_state();
    }
    catch (const std::exception &e) {
        log::error("Error from main: {}", e.what());
    }
}
