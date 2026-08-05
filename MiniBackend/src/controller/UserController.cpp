#include "UserController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

UserService UserController::service;

Response UserController::getUser(
    const Request& request
)
{
    json data =
        json::parse(
            request.body
        );

    std::string name =
        data["name"];
    
    int age =
        data["age"]
        .get<int>();

    User user =
        service.createUser(
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
    
    Response response;

    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Type"]
        = "application/json";

    response.body = 
        result.dump();

    return response;
}