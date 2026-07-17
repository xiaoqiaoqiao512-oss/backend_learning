#include "Http.h"

#include<sstream>

Request Http::parseRequest(
    const std::string& data
)
{
    Request request;

    std::stringstream ss(data);

    ss
    >> request.method
    >> request.path
    >> request.version;

    return request;
}