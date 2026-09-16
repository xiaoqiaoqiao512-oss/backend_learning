#pragma once

#include "model/User.h"
#include "repository/UserRepository.h"

#include <string>
#include <optional>

class UserService
{
public:
    User createUser(
        const std::string& name,
        int age
    );

    std::optional<User> getUserById(
        int id
    );
private:
   UserRepository repository_;
};
