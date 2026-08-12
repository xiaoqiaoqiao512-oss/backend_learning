#include "repository/UserRepository.h"

void UserRepository::save(
    const User& user
)
{
    users_.push_back(user);
}

User* UserRepository::findById(
    int id
)
{
    for(auto& user:users_)
    {
        if(user.id == id)
        {
            return &user;
        }
    }

    return nullptr;
}