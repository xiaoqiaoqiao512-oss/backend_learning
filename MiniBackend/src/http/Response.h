#pragma once

#include <string>
#include <unordered_map>

class Response
{
public:
    int statusCode;
    std::string statusText;
    std::unordered_map<
        std::string,
        std::string
    > headers;

    std::string body;

public:
    std::string serialize() const;
};