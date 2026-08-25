#pragma once

#include "Middleware.h"

#include <iostream>
#include <chrono>

class TimerMiddleware : public Middleware
{
public:
    Response handle(
        Request& request,
        std::function<
            Response(Request&)
        > next
    )override
    {
        auto start =
            std::chrono::steady_clock::now();

        Response response = 
            next(request);

        auto end = 
            std::chrono::steady_clock::now();

        auto duration =
            std::chrono::duration_cast<
                std::chrono::milliseconds
            >(end-start);

        std::cout
            << "[Time] "
            << duration.count()
            << "ms"
            << std::endl;
            
        return response;
    }
};