#include "Http.h"

#include<sstream>

Request Http::parseRequest(
    const std::string& data
)
{
    Request request;

    size_t headerEnd = 
        data.find("\r\n\r\n");

    std::string headerPart;

    if(headerEnd != std::string::npos)
    {
        headerPart =
            data.substr(
                0,
                headerEnd
            );

        request.body =
            data.substr(
                headerEnd + 4
            );
    }
    else
    {
        headerPart = data;
    }

    std::stringstream ss(headerPart);

    ss
    >> request.method
    >> request.path
    >> request.version;

    std::string line;

    std::getline(
        ss,
        line
    );

    while(
        std::getline(ss, line)
    )
    {
        if(line.empty()){
            continue;
        }

        size_t pos = 
            line.find(":");

        if(pos == std::string::npos){
            continue;
        }

        std::string key = 
            line.substr(
                0,
                pos
            );
        
        std::string value = 
            line.substr(
                pos + 1
            );

        while(
            !value.empty()
            &&
            value[0]==' '
        )
        {
            value.erase(
                value.begin()
            );
        }

        request.headers[key]
            = value;
    }

    return request;
}