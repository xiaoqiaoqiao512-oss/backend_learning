#include "UserService.h"

User UserService::createUser(
    const std::string& name,
    int age
)
{
    User user{};
    user.name = name;
    user.age = age;

    return repository_.save(user);
}

std::optional<User> UserService::getUserById(
    int id
)
{
    return repository_.findById(id);
}
