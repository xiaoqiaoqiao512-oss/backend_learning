#pragma once
#include "http/Request.h"
#include "http/Response.h"

#include <functional>
#include <unordered_map>
#include <string>

class Router{
public:
    using Handler =
        std::function<Response(const Request&)>;
public:
    void get(
        const std::string& path,
        Handler handler
    );

    Response handle(
        const Request& request
    );

private:
    std::unordered_map<
        std::string,
        Handler
    > getRoutes;
};