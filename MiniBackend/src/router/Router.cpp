#include "Router.h"
#include <sstream>

void Router::get(
    const std::string& path,
    Handler handler
){
    routes_.push_back(
        {
            "GET",
            path,
            handler
        }
    );
}

void Router::post(
    const std::string& path,
    Handler handler
)
{
    routes_.push_back(
        {
            "POST",
            path,
            handler
        }
    );
}

Response Router::handle(
    Request& request
){
    for(auto& route : routes_)
    {
        if(
            route.method == request.method &&
            match(
                route.path,
                request.path,
                request
            )
        )
        {
            return route.handler(request);
        }
    }

    Response res;

    res.statusCode = 404;
    res.statusText = "Not Found";
    res.body = "404";

    return res;
}

bool Router::match(
    const std::string& routePath,
    const std::string& requestPath,
    Request& request
)
{
    std::vector<std::string> routeParts;
    std::vector<std::string> requestParts;

    std::stringstream rs(routePath);
    std::stringstream qs(requestPath);

    std::string temp;

    while(
        getline(rs, temp, '/')
    )
    {
        if(!temp.empty())
            routeParts.push_back(temp);
    }

    while(
        getline(qs, temp, '/')
    )
    {
        if(!temp.empty())
            requestParts.push_back(temp);
    }

    if(routeParts.size()
        !=
        requestParts.size())
    {
        return false;
    }

    for(size_t i=0;i<routeParts.size();i++)
    {
        std::string routePart =
            routeParts[i];

        std::string requestPart =
            requestParts[i];

        if(routePart[0] == ':')
        {
            std::string key =
                routePart.substr(1);

            request.params[key]
                = 
                requestPart;
        }
        else
        {
            if(routePart != requestPart)
            {
                return false;
            }
        }
    }

    return true;
}