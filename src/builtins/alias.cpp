#include "alias.h"
#include "../aliases/alias_manager.h"

#include <iostream>

void builtinAlias(const std::vector<std::string>& tokens)
{
    if(tokens.size() < 2)
    {
        return;
    }

    std::string arg = tokens[1];

    size_t pos = arg.find('=');

    if(pos == std::string::npos)
    {
        std::cout << "alias: invalid format\n";
        return;
    }

    std::string name = arg.substr(0, pos);
    std::string value = arg.substr(pos + 1);

    addAlias(name, value);
}