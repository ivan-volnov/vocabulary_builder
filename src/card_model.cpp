#ifdef __APPLE__
#include "utility/apple_script.h"
#endif
#include "card_model.hpp"
#include "config.hpp"
#include "sqlite_database/sqlite_database.h"
#include "utility/anki_client.hpp"
#include "utility/file.hpp"
#include "utility/speech_engine.hpp"
#include "utility/tools.hpp"
#include <iostream>
#include <regex>
#include <st/formatter.hpp>
#include <st/string_functions.hpp>
#include <unordered_set>

CardModel::CardModel()
{
    vocabulary_profile_db = SqliteDatabase::open_read_only(
        Config::instance().get_vocabulary_profile_filepath());
    if (Config::instance().is_sound_enabled()) {
        speech = std::make_shared<SpeechEngine>("Daniel");
    }
    anki = std::make_shared<AnkiClient>();
    if (anki->request("version").get<uint64_t>() < 6) {
        throw std::runtime_error("AnkiConnect plugin is too old. Please update");
    }
}

void CardModel::open_kindle_db()
{
    const auto db_filepath = Config::instance().get_kindle_db_filepath();
    if (!std::filesystem::exists(db_filepath)) {
        throw std::runtime_error("Please connect your Kindle via USB cable first");
    }
    kindle_db = SqliteDatabase::open_read_only(db_filepath);
}

std::vector<std::string> CardModel::get_kindle_booklist() const
{
    st::assert_or_throw(!!kindle_db, "Kindle database is not open");
    std::vector<std::string> result;
    auto sql = kindle_db->create_query();
    sql << "SELECT DISTINCT title FROM BOOK_INFO";
    while (sql.step()) {
        result.push_back(sql.get_string());
    }
    return result;
}

void CardModel::load_from_kindle(const std::string &book, size_t &current_card_idx)
{
    st::assert_or_throw(!!kindle_db, "Kindle database is not open");
    auto sql = kindle_db->create_query();
    sql << "SELECT DISTINCT w.stem\n"
           "FROM WORDS w\n"
           "JOIN LOOKUPS l ON w.id = l.word_key\n"
           "JOIN BOOK_INFO b ON l.book_key = b.id\n"
           "WHERE b.title = ?\n";
    sql.bind(book);
    std::unordered_set<uint64_t> ids;
    while (sql.step()) {
        auto word = sql.get_string();
        auto note = anki->request(
            "findNotes",
            {
                {"query",
                 "\"deck:" + Config::get<std::string>("deck") + "\" front:\"" + word +
                 "\""}
        });
        if (note.empty()) {
            auto pair = get_word_info(word);
            auto card = std::make_unique<Card>();
            card->set_front(std::move(word));
            card->set_levels(std::move(pair.first));
            card->set_pos(std::move(pair.second));
            card->add_tag("kindle");
            cards.push_back(std::move(card));
        }
        ids.insert(note.begin(), note.end());
    }
    if (!ids.empty()) {
        anki->request(
            "addTags",
            {
                {"notes",      ids},
                { "tags", "kindle"}
        });
    }
    if (cards.empty()) {
        throw std::runtime_error("All cards done! No cards left for adding");
    }
    const auto skipped =
        Config::get_state<std::unordered_set<std::string>>("skipped_list");
    const auto middle =
        std::stable_partition(cards.begin(), cards.end(), [&skipped](const auto &card) {
            return skipped.find(card->get_front()) != skipped.end();
        });
    current_card_idx =
        middle == cards.end() ? cards.size() - 1 : std::distance(cards.begin(), middle);
    std::stable_partition(middle, cards.end(), [](const auto &card) {
        return !card->get_levels().empty();
    });
}

void CardModel::close_kindle_db()
{
    kindle_db.reset();
}

void CardModel::load_suspended_cards()
{
    for (const auto &note_id : anki->request(
             "findNotes",
             {
                 {"query",
                  "\"deck:" + Config::get<std::string>("deck") +
                  "\" is:suspended -tag:leech"}
    })) {
        auto card = std::make_unique<Card>();
        card->set_note_id(note_id.get<uint64_t>());
        anki_reload_card(*card);
        cards.push_back(std::move(card));
    }
}

void CardModel::load_leech_cards()
{
    for (const auto &note_id : anki->request(
             "findNotes",
             {
                 {"query",
                  "\"deck:" + Config::get<std::string>("deck") + "\" tag:leech"}
    })) {
        auto card = std::make_unique<Card>();
        card->set_note_id(note_id.get<uint64_t>());
        anki_reload_card(*card);
        cards.push_back(std::move(card));
    }
}

size_t CardModel::insert_new_card(std::string word, size_t idx)
{
    word = tools::clear_string(word);
    std::transform(word.begin(), word.end(), word.begin(), [](uint8_t c) {
        return std::tolower(c);
    });
    auto it = std::find_if(cards.begin(), cards.end(), [&word](auto &card) {
        return card->get_front() == word;
    });
    if (it == cards.end()) {
        auto pair = get_word_info(word);
        auto card = std::make_unique<Card>();
        card->set_front(std::move(word));
        card->set_levels(std::move(pair.first));
        card->set_pos(std::move(pair.second));
        anki_reload_card(*card);
        it = cards.insert(cards.begin() + idx, std::move(card));
    }
    return it - cards.begin();
}

string_set_pair CardModel::get_word_info(const std::string &word) const
{
    string_set_pair pair;
    auto sql = vocabulary_profile_db->create_query();
    sql << "SELECT level, pos FROM words WHERE base = ?";
    sql.bind(word);
    while (sql.step()) {
        pair.first.insert(sql.get_string());
        pair.second.insert(sql.get_string());
    }
    return pair;
}

void CardModel::query_vocabulary_profile(const std::string &query) const
{
    auto sql = vocabulary_profile_db->create_query();
    sql << "SELECT base, level, pos, gw\n"
           "FROM words\n"
           "WHERE base LIKE ?\n"
           "ORDER BY base, level, pos";
    sql.bind("%" + query + "%");
    while (sql.step()) {
        std::cout << std::left << std::setw(41) << sql.get_string() << std::left
                  << std::setw(4) << sql.get_string() << std::left << std::setw(10)
                  << sql.get_string() << std::left << std::setw(16) << sql.get_string()
                  << std::endl;
    }
}

Card &CardModel::get_card(size_t idx)
{
    return *cards.at(idx);
}

const Card &CardModel::get_card(size_t idx) const
{
    return *cards.at(idx);
}

size_t CardModel::size() const
{
    return cards.size();
}

void CardModel::look_up_in_safari(const std::string &word)
{
    if (word != last_safari_word) {
#ifdef __APPLE__
        std::ostringstream ss;
        ss << "set myURL to "
              "\"https://dictionary.cambridge.org/search/direct/?datasetsearch="
           << Config::get<std::string>("cambridge_dictionary")
           << "&q=" << st::url_encode(word)
           << "\"\n"
              "tell application \"Safari\"\n"
              "    if the URL of the front document starts with "
              "\"https://dictionary.cambridge.org\" then\n"
              "        set the URL of the front document to myURL\n"
              "    else\n"
              "        open location myURL\n"
              "    end if\n"
              "end tell";
        if (AppleScript::run_apple_script(ss.str())) {
            last_safari_word = word;
        }
#endif
    }
}

void CardModel::say(const std::string &word) const
{
    if (!speech) {
        return;
    }
    auto txt = word;
    txt = std::regex_replace(txt, std::regex("\\bsb\\b"), "somebody");
    txt = std::regex_replace(txt, std::regex("\\bsth\\b"), "something");
    txt = std::regex_replace(txt, std::regex("\\bswh\\b"), "somewhere");
    if (txt != "or" && txt != "believe it or not" && txt != "or so" &&
        txt != "more or less") {
        txt = std::regex_replace(txt, std::regex("\\bor\\b"), ",");
    }
    st::erase_all(txt, "(");
    st::erase_all(txt, ")");
    if (txt == "read, read, read") {
        txt = "read, red, red";
    }
    speech->say(txt);
}

void CardModel::anki_add_card(Card &card) const
{
    if (!anki_find_card(card)) {
        auto tags = card.get_tags();
        auto note = anki->request(
            "addNotes",
            {
                {"notes",
                 {{
                 {"deckName", Config::get<std::string>("deck")},
                 {"modelName", Config::get<std::string>("card_model")},
                 {"fields",
                 {{"Front", card.get_front()}, {"PoS", card.get_pos_string()}}},
                 {"tags", tags},
                 }}}
        });
        card.set_note_id(note.at(0).get<uint64_t>());
    }
    anki_open_browser(card);
}

void CardModel::anki_open_browser(const Card &card) const
{
    anki->request(
        "guiBrowse",
        {
            {"query",
             "\"deck:" + Config::get<std::string>("deck") + "\" front:\"" +
             card.get_front() + "\""}
    });
}

void CardModel::anki_reload_card(Card &card) const
{
    do {
        if (!card.get_note_id()) {
            continue;
        }
        auto note = anki->request(
                            "notesInfo",
                            {
                                {"notes", {card.get_note_id()}}
        })
                        .at(0);
        if (note.empty()) {
            card.set_note_id(0);
            continue;
        }
        bool changed = false;
        card.set_front(tools::clear_string(
            note.at("fields").at("Front").at("value").get<std::string>(), changed));
        card.set_back(tools::clear_string(
            note.at("fields").at("Back").at("value").get<std::string>(), changed));
        card.set_pos(tools::split<std::set>(
            tools::clear_string(
                note.at("fields").at("PoS").at("value").get<std::string>(), changed),
            ", "));
        card.set_forms(tools::clear_string(
            note.at("fields").at("Forms").at("value").get<std::string>(), changed));
        if (changed) {
            anki_update_card(card);
        }
        return;
    } while (anki_find_card(card));
}

void CardModel::anki_update_card(const Card &card) const
{
    anki->request(
        "updateNoteFields",
        {
            {"note",
             {
             {"id", card.get_note_id()},
             {"fields",
             {{"Front", card.get_front()},
             {"Back", card.get_back()},
             {"Forms", card.get_forms()},
             {"PoS", card.get_pos_string()}}},
             }}
    });
}

bool CardModel::anki_find_card(Card &card) const
{
    auto notes = anki->request(
        "findNotes",
        {
            {"query",
             "\"deck:" + Config::get<std::string>("deck") + "\" front:\"" +
             card.get_front() + "\""}
    });
    if (notes.empty()) {
        card.set_note_id(0);
        return false;
    }
    card.set_note_id(notes.at(0).get<uint64_t>());
    return true;
}

void CardModel::anki_fix_collection(bool commit) const
{
    for (const auto &note : anki->request(
             "notesInfo",
             {
                 {"notes",
                  anki->request(
                  "findNotes", {{"query",
 "\"deck:" + Config::get<std::string>("deck") + "\""}})}
    })) {
        const auto front_old =
            note.at("fields").at("Front").at("value").get<std::string>();
        const auto back_old = note.at("fields").at("Back").at("value").get<std::string>();
        const auto pos_old = note.at("fields").at("PoS").at("value").get<std::string>();
        const auto note_id = note.at("noteId").get<uint64_t>();
        const auto front = tools::clear_string(front_old);
        if (front != front_old) {
            std::cout << "Fix front: " << front_old << " to: " << front << std::endl;
            if (commit) {
                anki->request(
                    "updateNoteFields",
                    {
                        {"note",
                         {
                         {"id", note_id},
                         {"fields",
                         {
                         {"Front", front},
                         }},
                         }}
                });
            }
        }
        const auto back = tools::clear_string(back_old);
        if (back != back_old) {
            std::cout << "Fix back: " << back_old << " to: " << back << std::endl;
            if (commit) {
                anki->request(
                    "updateNoteFields",
                    {
                        {"note",
                         {
                         {"id", note_id},
                         {"fields",
                         {
                         {"Back", back},
                         }},
                         }}
                });
            }
        }
        const auto s = tools::split<std::set>(tools::clear_string(pos_old), ", ");
        const auto pos = fmt::format("{}", fmt::join(s, ", "));
        if (pos != pos_old) {
            std::cout << "Fix pos: " << pos_old << " to: " << pos << std::endl;
            if (commit) {
                anki->request(
                    "updateNoteFields",
                    {
                        {"note",
                         {
                         {"id", note_id},
                         {"fields", {{"PoS", pos}}},
                         }}
                });
            }
        }
    }
}

void CardModel::anki_nvim_export(const char *filename) const
{
    auto notes = anki->request(
        "findNotes",
        {
            {"query", "\"deck:Vocabulary Profile\" -is:new -is:learn -is:suspended"}
    });
    notes = anki->request(
        "notesInfo",
        {
            {"notes", std::move(notes)}
    });

    std::map<std::string, std::string> map;
    for (const auto &note : notes) {
        const auto &fields = note.at("fields");
        auto phrase = fields.at("Front").at("value").get<std::string>();
        auto translation = fields.at("Back").at("value").get<std::string>();
        st::erase_all(phrase, ", etc.");
        st::erase_all(phrase, ", etc");
        tools::clear_string(phrase);
        tools::clear_string(translation);
        map.emplace(std::move(phrase), std::move(translation));
    }

    File file{filename, "w"};
    fmt::print(file, "{{\n");
    bool first{true};
    for (const auto &w : map) {
        if (first) [[unlikely]] {
            first = false;
        }
        else {
            fmt::print(file, ",\n");
        }
        fmt::print(
            file, R"(    "{}": "{}")", st::formatter::escaped(w.first, "\"\\"),
            st::formatter::escaped(w.second, "\"\\"));
    }
    fmt::print(file, "\n}}\n");
}
