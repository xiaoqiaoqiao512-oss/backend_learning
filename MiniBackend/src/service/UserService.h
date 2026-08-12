#pragma once

#include "model/User.h"
#include "repository/UserRepository.h"

#include<string>

class UserService
{
public:
    User createUser(
        const std::string& name,
        int age
    );

    User* getUserById(
        int id
    );
private:
   static int count;

   UserRepository repository_;
};