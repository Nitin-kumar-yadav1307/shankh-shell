#include "alias_manager.h"
#include "../parser/parser.h"

#include <vector>
#include <string>
#include <map>

std::map<std::string, std::string> aliases;

void expandAlias(std::vector<std::string>& tokens){

    if(tokens.empty())
        return;

    if(hasAlias(tokens[0]))
    {
        std::string replacement =
            getAlias(tokens[0]);

        std::vector<std::string> aliasTokens =
            splitInput(replacement);

        for(size_t i = 1; i < tokens.size(); i++)
        {
            aliasTokens.push_back(tokens[i]);
        }

        tokens = aliasTokens;
    }
}

void addAlias( const std::string& name,const std::string& value){
    aliases[name] = value;
}

bool hasAlias(const std::string& name){
    return aliases.count(name) > 0;
}

std::string getAlias(const std::string& name){
    return aliases[name];
}