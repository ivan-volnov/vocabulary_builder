#include "card_model.h"
#include <unordered_set>
#include <regex>
#include "sqlite_database/sqlite_database.h"
#include "utility/anki_client.h"
#include "utility/tools.h"
#include "utility/apple_script.h"
#include "utility/speech_engine.h"
#include "config.h"


CardModel::CardModel() :
    speech(std::make_shared<SpeechEngine>()), cambridge_dictionary("english-russian")
{
    const auto db_filepath = Config::instance().get_kindle_db_filepath();
    if (!std::filesystem::exists(db_filepath)) {
        throw std::runtime_error("Please connect your Kindle via USB cable first");
    }
    kindle_db = SqliteDatabase::open_read_only(db_filepath);
    vocabulary_profile_db = SqliteDatabase::open_read_only(Config::instance().get_vocabulary_profile_filepath());
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
    AnkiClient anki;
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
        auto note = anki.request("findNotes", {{"query", "front:\"" + word + "\""}});
        if (note.empty()) {
            auto pair = get_word_info(word);
            cards.emplace_back(std::move(word), std::move(pair.first), std::move(pair.second));
        }
        ids.insert(note.begin(), note.end());
    }
    if (!ids.empty()) {
        anki.request("addTags", {{"notes", ids}, {"tags", "kindle"}});
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
           << "https://dictionary.cambridge.org/search/direct/?datasetsearch=" << cambridge_dictionary << "&q=" << tools::url_encode(word)
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
    tools::string_replace(txt, "(", "");
    tools::string_replace(txt, ")", "");
    if (txt == "read, read, read") {
        txt = "read, red, red";
    }
    speech->say(txt);
}
