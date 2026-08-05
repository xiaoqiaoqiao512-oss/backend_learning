#pragma once

#include "http/Request.h"
#include "http/Response.h"
#include "service/UserService.h"

class UserController{
public:
    static Response getUser(
        const Request& Request
    );

private:
    static UserService service;
};