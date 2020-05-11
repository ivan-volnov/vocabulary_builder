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

    size_t size() const;

private:
    std::vector<Card> cards;
    std::shared_ptr<SqliteDatabase> database;
};


#endif // CARDMODEL_H
