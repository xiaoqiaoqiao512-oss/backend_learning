#pragma once
#include "model/User.h"
#include <vector>

class UserRepository
{
public:
    void save(
        const User& user
    );

    User* findById(
        int id
    );

private:
    std::vector<User> users_;
};