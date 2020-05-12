#ifndef CARDMODEL_H
#define CARDMODEL_H

#include <vector>
#include "card.h"


class SqliteDatabase;


class CardModel
{
public:
    CardModel();

    std::vector<std::string> get_kindle_booklist() const;
    void load_from_kindle(const std::string &book);

    string_set_pair get_word_info(const std::string &word) const;

    size_t size() const;

private:
    std::vector<Card> cards;
    std::shared_ptr<SqliteDatabase> kindle_db;
    std::shared_ptr<SqliteDatabase> vocabulary_profile_db;
};


#endif // CARDMODEL_H
