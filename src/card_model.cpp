#include "card_model.h"
#include "sqlite_database/sqlite_database.h"
#include "utility/anki_client.h"
#include "config.h"


CardModel::CardModel()
{
    const auto db_filepath = Config::instance().get_kindle_db_filepath();
    if (!std::filesystem::exists(db_filepath)) {
        throw std::runtime_error("Please connect your Kindle via USB cable first");
    }
    database = SqliteDatabase::open_read_only(db_filepath);
}

std::vector<std::string> CardModel::get_kindle_booklist() const
{
    std::vector<std::string> result;
    auto sql = database->create_query();
    sql << "SELECT DISTINCT title FROM BOOK_INFO";
    while (sql.step()) {
        result.push_back(sql.get_string());
    }
    return result;

}

void CardModel::load_from_kindle(const std::string &book)
{
    AnkiClient anki;
    auto sql = database->create_query();
    sql << "SELECT DISTINCT w.stem\n"
           "FROM WORDS w\n"
           "JOIN LOOKUPS l ON w.id = l.word_key\n"
           "JOIN BOOK_INFO b ON l.book_key = b.id\n"
           "WHERE b.title = ?\n";
    sql.bind(book);
    std::vector<uint64_t> ids;
    while (sql.step()) {
        auto word = sql.get_string();
        auto note = anki.request("findNotes", {{"query", "front:\"" + word + "\""}});
        if (note.empty()) {
            cards.push_back(std::move(word));
        }
        ids.insert(ids.end(), note.begin(), note.end());
    }
    if (!ids.empty()) {
        anki.request("addTags", {{"notes", ids}, {"tags", "kindle"}});
    }
}

size_t CardModel::size() const
{
    return cards.size();
}
