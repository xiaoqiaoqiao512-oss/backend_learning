#pragma once

#include "model/User.h"

#include <optional>
#include <sqlite3.h>
#include <vector>


class UserRepository
{
public:
    UserRepository();
    ~UserRepository();

    User save(
        const User& user
    );

    std::optional<User> findById(
        int id
    );

private:
    sqlite3* db_;
    std::vector<User> users_;
};