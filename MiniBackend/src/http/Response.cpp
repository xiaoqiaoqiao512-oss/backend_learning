#include "Response.h"

#include<sstream>
#include<nlohmann/json.hpp>

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

Response Response::json(
    const std::string& data
)
{
    Response response;

    response.statusCode = 200;
    response.statusText = "OK";

    response.headers[
        "Content-Type"
    ] = "application/json";

    response.body = data;

    return response;
}

Response Response::error(
    int code,
    const std::string& message
)
{
    Response response;

    response.statusCode = code;

    if(code == 400)
    {
        response.statusText = "Bad Request";
    }
    else if(code == 404)
    {
        response.statusText = "Not Found";
    }
    else
    {
        response.statusText = "Error";
    }


    response.headers[
        "Content-Type"
    ] = "application/json";


    nlohmann::json body;

    body["error"] = message;

    response.body = body.dump();

    return response;
}