#pragma once

#include "Middleware.h"

#include <iostream>

class LoggerMiddleware : public Middleware
{
public:
    Response handle(
        Request& request,
        std::function<
            Response(Request&)
        > next
    ) override
    {
        std::cout
            << "[Request] "
            << request.method
            << " "
            << request.path
            << std::endl;

        Response response =
            next(request);

        std::cout
            << "[Response] "
            << response.statusCode
            << std::endl;

        return response;
    }
};