#ifndef CARD_H
#define CARD_H

#include <string>


class Card
{
public:
    Card(const std::string &front, const std::string &back) :
        front(front), back(back)
    {

    }

private:
    std::string front;
    std::string back;
};


#endif // CARD_H
