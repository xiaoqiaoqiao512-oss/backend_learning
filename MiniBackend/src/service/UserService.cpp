#include "UserService.h"

std::string UserService::getUserInfo(
    int id
){
    if(id == 1){
        return "Tony";
    }

    return "Unknow User";
}