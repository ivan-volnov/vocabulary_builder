#ifndef ANKI_CLIENT_HPP
#define ANKI_CLIENT_HPP

#include "curl_request.hpp"
#include <libs/json.hpp>

class AnkiClient
{
public:
    AnkiClient();

    nlohmann::json request(
        const std::string &action, const nlohmann::json &params = nullptr);

private:
    CurlSession session;
};

#endif // ANKI_CLIENT_HPP
