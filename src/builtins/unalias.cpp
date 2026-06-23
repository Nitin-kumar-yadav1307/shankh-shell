#include "unalias.h"
#include "../aliases/alias_manager.h"

void builtinUnalias(const std::vector<std::string>& tokens)
{
    if(tokens.size() < 2)
        return;

    removeAlias(tokens[1]);
}