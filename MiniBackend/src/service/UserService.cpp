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

    return user;
}