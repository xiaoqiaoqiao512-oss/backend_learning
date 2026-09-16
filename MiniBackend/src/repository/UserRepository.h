#pragma once

#include "model/User.h"

#include <optional>
#include <sqlite3.h>


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
};