#include "Response.h"

#include<sstream>

std::string Response::serialize() const{
    std::stringstream ss;

    ss<<"HTTP/1.1"
    <<statusCode
    <<" "
    <<statusText
    <<"\r\n";

    for(const auto& header:headers)
    {
        ss<<header.first
        <<": "
        <<header.second
        <<"\r\n";
    }

    ss<<"\r\n";

    ss<<body;

    return ss.str();
}