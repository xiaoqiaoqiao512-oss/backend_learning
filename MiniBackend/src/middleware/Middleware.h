#pragma once

#include "http/Request.h"
#include "http/Response.h"

#include <iostream>

class Middleware
{
public:
    virtual ~Middleware() = default;

    virtual Response handle(
        Request& request,
        std::function<
            Response(Request&)
        > next
    ) = 0;
};