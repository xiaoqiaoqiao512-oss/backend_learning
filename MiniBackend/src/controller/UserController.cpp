#include "UserController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

UserController::UserController(
    UserService& service
)
:
service_(service)
{
}

Response UserController::createUser(
    const Request& request
)
{
    json data =
        json::parse(
            request.body
        );

    if(!data.contains("name"))
    {
        return Response::error(
            400,
            "name is required"
        );
    }

    if(!data.contains("age"))
    {
        return Response::error(
            400,
            "age is required"
        );
    }

    if(!data["name"].is_string())
    {
        return Response::error(
            400,
            "name must be string"
        );
    }

    if(!data["age"].is_number_integer())
    {
        return Response::error(
            400,
            "age must be a integer"
        );
    }

    std::string name =
        data["name"];
    
    int age =
        data["age"]
        .get<int>();

    User user =
        service_.createUser(
            name,
            age
        );
    
    json result;

    result["id"]
        = user.id;
    
    result["name"]
        = user.name;


    result["age"]
        = user.age;
    
    return Response::json(
        result.dump()
    );
}

Response UserController::getUserById(
    const Request& request
)
{
    int id = 
        std::stoi(
            request.params.at("id")
        );

    std::optional<User> user = 
        service_.getUserById(id);

    if(!user)
    {
        return Response::error(
            404,
            "User not found"
        );
    }

    json result;

    result["id"] = id;
    result["name"] = user->name;
    result["age"] = user->age;

    return Response::json(
        result.dump()
    );
}