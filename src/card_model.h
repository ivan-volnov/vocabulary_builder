#ifndef CARDMODEL_H
#define CARDMODEL_H

#include <vector>
#include "card.h"


class SqliteDatabase;
class SpeechEngine;


class CardModel
{
public:
    CardModel();

    std::vector<std::string> get_kindle_booklist() const;
    void load_from_kindle(const std::string &book);

    string_set_pair get_word_info(const std::string &word) const;

    const Card &get_card(size_t idx) const;

    size_t size() const;

    void look_up_in_safari(const std::string &word);
    void say(const std::string &word) const;

private:
    std::vector<Card> cards;
    std::shared_ptr<SqliteDatabase> kindle_db;
    std::shared_ptr<SqliteDatabase> vocabulary_profile_db;
    std::shared_ptr<SpeechEngine> speech;
    std::string last_safari_word;
    std::string cambridge_dictionary;
};


#endif // CARDMODEL_H
