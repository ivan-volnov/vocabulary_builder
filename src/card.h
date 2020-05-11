#ifndef CARD_H
#define CARD_H

#include <string>


class Card
{
public:
    Card(std::string &&front) :
        front(std::move(front))
    {

    }

    Card(const std::string &front, const std::string &back) :
        front(front), back(back)
    {

    }

private:
    std::string front;
    std::string back;
};


#endif // CARD_H
