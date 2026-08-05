#include "Response.h"

#include<sstream>

std::string Response::serialize() const{
    std::stringstream ss;

    ss<<"HTTP/1.1 "
    <<statusCode
    <<" "
    <<statusText
    <<"\r\n";

    bool haslength = false;
    bool hasConnection = false;

    for(const auto& header:headers)
    {
        if(header.first == "Content-Length"){
            haslength = true;
        }

        if(header.first == "Connection"){
            hasConnection = true;
        }

        ss<<header.first
        <<": "
        <<header.second
        <<"\r\n";
    }

    if(!hasConnection){
        ss
        << "Connection: close\r\n";
    }

    if(!haslength){
        ss
        << "Content-Length: "
        << body.size()
        << "\r\n";
    }

    ss<<"\r\n";

    ss<<body;

    return ss.str();
}