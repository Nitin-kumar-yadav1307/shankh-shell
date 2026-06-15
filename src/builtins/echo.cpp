#include "echo.h"

#include <iostream>

void builtinEcho(
    const std::vector<std::string>& tokens
)
{
    for(size_t i = 1; i < tokens.size(); i++)
    {
        if(i > 1)
        {
            std::cout << " ";
        }

        std::cout << tokens[i];
    }

    std::cout << std::endl;
}