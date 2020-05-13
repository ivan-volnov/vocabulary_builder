#include "card_model.h"
#include <unordered_set>
#include <regex>
#include <string_essentials/string_essentials.h>
#include "sqlite_database/sqlite_database.h"
#include "utility/anki_client.h"
#include "utility/apple_script.h"
#include "utility/speech_engine.h"
#include "config.h"


CardModel::CardModel() :
    cambridge_dictionary("english-russian")
{
    const auto db_filepath = Config::instance().get_kindle_db_filepath();
    if (!std::filesystem::exists(db_filepath)) {
        throw std::runtime_error("Please connect your Kindle via USB cable first");
    }
    kindle_db = SqliteDatabase::open_read_only(db_filepath);
    vocabulary_profile_db = SqliteDatabase::open_read_only(Config::instance().get_vocabulary_profile_filepath());
    speech = std::make_shared<SpeechEngine>();
    anki = std::make_shared<AnkiClient>();
    if (anki->request("version").get<uint64_t>() < 6) {
        throw std::runtime_error("AnkiConnect plugin is too old. Please update");
    }
}

std::vector<std::string> CardModel::get_kindle_booklist() const
{
    std::vector<std::string> result;
    auto sql = kindle_db->create_query();
    sql << "SELECT DISTINCT title FROM BOOK_INFO";
    while (sql.step()) {
        result.push_back(sql.get_string());
    }
    return result;

}

void CardModel::load_from_kindle(const std::string &book)
{
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
        auto note = anki->request("findNotes", {{"query", "front:\"" + word + "\" -tag:vb_beta"}});
        if (note.empty()) {
            auto pair = get_word_info(word);
            cards.emplace_back(std::move(word), std::move(pair.first), std::move(pair.second));
        }
        ids.insert(note.begin(), note.end());
    }
    if (!ids.empty()) {
        anki->request("addTags", {{"notes", ids}, {"tags", "kindle"}});
    }
    auto notes = anki->request("findNotes", {{"query", "tag:vb_beta"}});
    for (const auto &note : anki->request("notesInfo", {{"notes", notes}})) {
        const auto front = note.at("fields").at("Front").at("value").get<std::string>();
        for (auto &card : cards) {
            if (card.get_front() == front) {
                card.set_note_id(note.at("noteId").get<uint64_t>());
                card.set_back(note.at("fields").at("Back").at("value").get<std::string>());
                break;
            }
        }
    }
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

Card &CardModel::get_card(size_t idx)
{
    return cards.at(idx);
}

const Card &CardModel::get_card(size_t idx) const
{
    return cards.at(idx);
}

size_t CardModel::size() const
{
    return cards.size();
}

void CardModel::look_up_in_safari(const std::string &word)
{
    if (word != last_safari_word) {
        std::ostringstream ss;
        ss << "tell application \"Safari\" to set the URL of the front document to \""
           << "https://dictionary.cambridge.org/search/direct/?datasetsearch=" << cambridge_dictionary << "&q=" << string_essentials::url_encode(word)
           << "\"";
        if (AppleScript::run_apple_script(ss.str())) {
            last_safari_word = word;
        }
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
    if (txt != "or" &&
        txt != "believe it or not" &&
        txt != "or so" &&
        txt != "more or less")
    {
        txt = std::regex_replace(txt, std::regex("\\bor\\b"), ",");
    }
    string_essentials::replace(txt, "(", "");
    string_essentials::replace(txt, ")", "");
    if (txt == "read, read, read") {
        txt = "read, red, red";
    }
    speech->say(txt);
}

void CardModel::anki_add_card(Card &card) const
{
    if (!anki_find_card(card)) {
        auto tags = card.get_levels();
        tags.insert("kindle");
        tags.insert("vb_beta");
        auto note = anki->request("addNotes", {{"notes", {{
            {"deckName", "En::Vocabulary Profile::" + card.get_level()},
            {"modelName", "Main en-GB"},
            {"fields", {
                 {"Front", card.get_front()},
                 {"PoS", card.get_pos_string()} }},
            {"tags", tags},
        }}}});
        card.set_note_id(note.at(0).get<uint64_t>());
    }
    anki_open_browser(card);
}

void CardModel::anki_open_browser(const Card &card) const
{
    anki->request("guiBrowse", {{"query", "front:\"" + card.get_front() + "\""}});
}

void CardModel::anki_reload_card(Card &card) const
{
    do {
        if (!card.get_note_id()) {
            continue;
        }
        auto note = anki->request("notesInfo", {{"notes", {card.get_note_id()}}}).at(0);
        if (note.empty() || card.get_front() != note.at("fields").at("Front").at("value").get<std::string>()) {
            card.set_note_id(0);
            continue;
        }
        card.set_back(note.at("fields").at("Back").at("value").get<std::string>());
        card.set_pos(string_essentials::split<std::set>(note.at("fields").at("PoS").at("value").get<std::string>(), ", "));
        return;
    } while (anki_find_card(card));
}

bool CardModel::anki_find_card(Card &card) const
{
    auto notes = anki->request("findNotes", {{"query", "front:\"" + card.get_front() + "\""}});
    if (notes.empty()) {
        card.set_note_id(0);
        return false;
    }
    card.set_note_id(notes.at(0).get<uint64_t>());
    return true;
}
