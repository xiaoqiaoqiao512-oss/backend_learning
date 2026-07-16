#pragma once

#include "Request.h"

#include<string>

class Http
{   
public:
    static Request parseRequest(
        const std::string& data
    );
};