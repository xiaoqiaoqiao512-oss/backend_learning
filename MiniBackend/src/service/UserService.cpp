#include "UserService.h"

int UserService::count = 0;

User UserService::createUser(
    const std::string& name,
    int age
)
{
    User user;
    user.name = name;
    user.id = ++count;
    user.age = age;

    repository_.save(user);

    return user;
}

User* UserService::getUserById(
    int id
)
{
    return repository_.findById(id);
}