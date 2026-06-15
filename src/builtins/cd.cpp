#include "cd.h"

#include <iostream>
#include <unistd.h>
#include <cstdlib>

void builtinCd(std::string path)
{
    if(path == "~")
    {
        path = getenv("HOME");
    }

    int result = chdir(path.c_str());

    if(result == -1)
    {
        std::cout << "cd: "
                  << path
                  << ": No such file or directory"
                  << std::endl;
    }
}