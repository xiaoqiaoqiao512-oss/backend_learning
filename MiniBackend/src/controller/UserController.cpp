#include "UserController.h"
#include <nlohmann/json.hpp>

#include <stdexcept>

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
    json data;

    try
    {
        data =
            json::parse(
                request.body
            );
    }
    catch(const json::parse_error&)
    {
        return Response::error(
            400,
            "invalid JSON"
        );
    }

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
    const auto parameter =
        request.params.find("id");

    if(parameter == request.params.end())
    {
        return Response::error(
            400,
            "user id is required"
        );
    }

    const std::string& rawId =
        parameter->second;

    int id = 0;
    std::size_t consumed = 0;

    try
    {
        id =
            std::stoi(
                rawId,
                &consumed
            );
    }
    catch(const std::invalid_argument&)
    {
        return Response::error(
            400,
            "invalid user id"
        );
    }
    catch(const std::out_of_range&)
    {
        return Response::error(
            400,
            "invalid user id"
        );
    }

    if(consumed != rawId.size())
    {
        return Response::error(
            400,
            "invalid user id"
        );
    }

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
