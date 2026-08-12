#pragma once

#include "http/Request.h"
#include "http/Response.h"
#include "service/UserService.h"

class UserController{
public:
    UserController(
        UserService& service
    );

    Response createUser(
        const Request& Request
    );

    Response getUserById(
        const Request& request
    );

private:
    UserService& service_;
};