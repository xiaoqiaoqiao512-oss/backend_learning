#include "repository/UserRepository.h"

#include <sqlite3.h>
#include <stdexcept>

UserRepository::UserRepository()
    : db_(nullptr)
{
    int result =
        sqlite3_open(
            "minibackend.db",
            &db_
        );

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            "failed to open database"
        );
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            age INTEGER NOT NULL
        );
    )";

    char* errorMessage = nullptr;

    result =
        sqlite3_exec(
            db_,
            sql,
            nullptr,
            nullptr,
            &errorMessage
        );

    if (result != SQLITE_OK)
    {
        std::string message =
            errorMessage
                ? errorMessage
                : "failed to create users table";
        sqlite3_free(errorMessage);
        throw std::runtime_error(message);
    }
}

UserRepository::~UserRepository()
{
    if(db_ != nullptr)
    {
        sqlite3_close(db_);
    }
}

User UserRepository::save(
    const User& user
)
{
    const char* sql =
        "INSERT INTO users (name, age) VALUES (?, ?);";

    sqlite3_stmt* statement = nullptr;

    int result =
        sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &statement,
            nullptr
        );

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    sqlite3_bind_text(
        statement,
        1,
        user.name.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_int(
        statement,
        2,
        user.age
    );

    result =
        sqlite3_step(statement);

    if(result != SQLITE_DONE)
    {
        sqlite3_finalize(statement);

        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    sqlite3_finalize(statement);

    User savedUser = user;

    savedUser.id =
        static_cast<int>(
            sqlite3_last_insert_rowid(db_)
        );

    return savedUser;
}

std::optional<User> UserRepository::findById(
    int id
)
{
    const char* sql =
        "SELECT id, name, age FROM users WHERE id = ?;";

    sqlite3_stmt* statement = nullptr;

    int result =
        sqlite3_prepare_v2(
            db_,
            sql,
            -1,
            &statement,
            nullptr
        );

    if(result != SQLITE_OK)
    {
        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    result =
        sqlite3_bind_int(
            statement,
            1,
            id
        );

    if(result != SQLITE_OK)
    {
        sqlite3_finalize(statement);

        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    result =
        sqlite3_step(statement);

    if(result == SQLITE_DONE)
    {
        sqlite3_finalize(statement);
        return std::nullopt;
    }

    if(result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);

        throw std::runtime_error(
            sqlite3_errmsg(db_)
        );
    }

    User user;
    user.id =
        sqlite3_column_int(
            statement,
            0
        );
    user.name =
        reinterpret_cast<const char*>(
            sqlite3_column_text(
                statement,
                1
            )
        );
    user.age =
        sqlite3_column_int(
            statement,
            2
        );

    sqlite3_finalize(statement);
    return user;
}
