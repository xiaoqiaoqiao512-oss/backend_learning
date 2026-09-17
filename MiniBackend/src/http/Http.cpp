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

    std::string pathWithQuery;

    ss
    >> request.method
    >> pathWithQuery
    >> request.version;

    const std::size_t queryStart =
        pathWithQuery.find('?');

    if(queryStart != std::string::npos)
    {
        request.path =
            pathWithQuery.substr(0, queryStart);

        std::string querylist =
            pathWithQuery.substr(queryStart + 1);
        std::size_t start = 0;
        while(true)
        {
            const std::size_t end =
                querylist.find('&', start);

            std::string pair;

            if(end == std::string::npos)
            {
                pair = querylist.substr(start);
            }
            else
            {
                pair = querylist.substr(start, end - start);
            }

            const std::size_t equalPos =
                pair.find('=');

            if(equalPos != std::string::npos)
            {
                const std::string key =
                    pair.substr(0, equalPos);
                const std::string value =
                    pair.substr(equalPos + 1);

                request.query[key] = value;
            }

            if(end == std::string::npos)
            {
                break;
            }

            start = end + 1;
        }
    }
    else
    {
        request.path = pathWithQuery;
    }

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
