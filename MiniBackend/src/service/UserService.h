#pragma once

#include "model/User.h"

#include<string>

class UserService
{
public:
    User createUser(
        const std::string& name,
        int age
    );
private:
   static int count;
};