#include "variable_manager.h"

std::map<std::string, std::string> shellVars;

bool isValidIdentifier(const std::string& name){
    if(name.empty()) return false;
    
    // first char must be letter or underscore
    if(!isalpha(name[0]) && name[0] != '_') return false;
    
    // rest must be letter, digit, or underscore
    for(size_t i = 1; i < name.size(); i++){
        if(!isalnum(name[i]) && name[i] != '_') return false;
    }
    
    return true;
}

std::string expandVariables(const std::string& token){
    std::string result;
    size_t i = 0;

    while(i < token.size()){

        // ${VAR}
        if(token[i] == '$' &&
           i + 1 < token.size() &&
           token[i + 1] == '{'){

            i += 2;   // skip "${"

            std::string name;

            while(i < token.size() && token[i] != '}'){
                name += token[i];
                i++;
            }

            // skip '}'
            if(i < token.size() && token[i] == '}')
                i++;

            if(shellVars.count(name))
                result += shellVars[name];
        }

        // $VAR
        else if(token[i] == '$'){
            i++;   // skip $

            std::string name;

            while(i < token.size() &&
                  (isalnum(token[i]) || token[i] == '_')){
                name += token[i];
                i++;
            }

            if(shellVars.count(name))
                result += shellVars[name];
        }

        else{
            result += token[i];
            i++;
        }
    }

    return result;
}