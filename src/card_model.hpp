#ifndef CARDMODEL_HPP
#define CARDMODEL_HPP

#include "card.hpp"
#include <vector>


class SqliteDatabase;
class SpeechEngine;
class AnkiClient;

class CardModel
{
public:
    CardModel();

    void open_kindle_db();
    std::vector<std::string> get_kindle_booklist() const;
    void load_from_kindle(const std::string &book, size_t &current_card_idx);
    void close_kindle_db();

    void load_suspended_cards();
    void load_leech_cards();

    size_t insert_new_card(std::string word, size_t idx);

    string_set_pair get_word_info(const std::string &word) const;
    void query_vocabulary_profile(const std::string &query) const;

    Card &get_card(size_t idx);
    const Card &get_card(size_t idx) const;

    size_t size() const;

    void look_up_in_safari(const std::string &word);
    void say(const std::string &word) const;

    void anki_add_card(Card &card) const;
    void anki_open_browser(const Card &card) const;
    void anki_reload_card(Card &card) const;
    void anki_update_card(const Card &card) const;

    bool anki_find_card(Card &card) const;

    void anki_fix_collection(bool commit) const;
    void anki_nvim_export(const char *filename) const;

private:
    std::vector<std::unique_ptr<Card>> cards;
    std::shared_ptr<SqliteDatabase> kindle_db;
    std::shared_ptr<SqliteDatabase> vocabulary_profile_db;
    std::shared_ptr<SpeechEngine> speech;
    std::shared_ptr<AnkiClient> anki;
    std::string last_safari_word;
};


#endif // CARDMODEL_HPP
