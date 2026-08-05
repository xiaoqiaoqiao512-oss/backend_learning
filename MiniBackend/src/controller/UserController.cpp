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
    
    Response response;

    response.statusCode = 200;
    response.statusText = "OK";

    response.headers["Content-Type"]
        = "application/json";
    
    json result;

    result["username"]
        =
        name;
    
    response.body = result.dump();

    return response;
}