#include "export.h"

#include <cstdlib>
#include <iostream>

void builtinExport(const std::vector<std::string>& tokens)
{
    if(tokens.size() < 2)
        return;

    std::string arg = tokens[1];

    size_t pos = arg.find('=');

    if(pos == std::string::npos)
    {
        std::cout << "export: invalid format\n";
        return;
    }

    std::string name = arg.substr(0, pos);
    std::string value = arg.substr(pos + 1);

    setenv(
        name.c_str(),
        value.c_str(),
        1
    );
}