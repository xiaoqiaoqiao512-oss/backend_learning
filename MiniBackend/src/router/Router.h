#pragma once
#include "http/Request.h"
#include "http/Response.h"

#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

class Router{
public:
    using Handler =
        std::function<Response(const Request&)>;
public:
    void get(
        const std::string& path,
        Handler handler
    );

    void post(
        const std::string& path,
        Handler handler
    );

    Response handle(
        Request& request
    );

private:
    bool match(
        const std::string& routePath,
        const std::string& requestPath,
        Request& request
    );

private:
    struct Route
    {
        std::string method;
        std::string path;
        Handler handler;
    };
    std::vector<Route> routes_;
};