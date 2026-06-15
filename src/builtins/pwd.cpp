#include "pwd.h"

#include <iostream>
#include <unistd.h>
#include <limits.h>


void builtinPwd(){
    char buffer[PATH_MAX];    
    if(getcwd(buffer, sizeof(buffer)) != nullptr){
        std::cout << buffer << std::endl;    
    }
}        