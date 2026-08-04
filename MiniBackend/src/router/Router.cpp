#include "Router.h"

void Router::get(
    const std::string& path,
    Handler handler
){
    getRoutes[path] = handler;
}

Response Router::handle(
    const Request& request
){
    auto it = getRoutes.find(request.path);
    if(it != getRoutes.end())
    {
        return it->second(request);
    }

    Response res;
    res.statusCode = 404;
    res.statusText = "Not Found";
    res.body = "404";

    return res;
}